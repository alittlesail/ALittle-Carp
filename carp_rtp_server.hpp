#ifndef CARP_RTP_SERVER_INCLUDED
#define CARP_RTP_SERVER_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "carp_udp_server.hpp"
#include "carp_schedule.hpp"

class CarpRtpServer;
typedef std::shared_ptr<CarpRtpServer> CarpRtpServerPtr;
typedef std::weak_ptr<CarpRtpServer> CarpRtpServerWeakPtr;

class CarpRtpServer : public std::enable_shared_from_this<CarpRtpServer>
{
public:
	CarpRtpServer() {}
	~CarpRtpServer() { Close(); }

public:
	// 初始化
	// from_rtp_ip 表示要使用的ip，用于呼叫发起方互发媒体包。
	// to_rtp_ip 表示要使用的ip，用于被呼叫放互发媒体包
	bool Create(const std::string& from_rtp_ip, unsigned int from_rtp_port
		, const std::string& to_rtp_ip, unsigned int to_rtp_port
		, CarpSchedule* schedule)
	{
		// 获取弱引用
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		// 检查ip和端口是否对的上，对不上就关闭，然后重新创建
		if (m_udp_from_rtp && (m_udp_from_rtp->GetIp() != from_rtp_ip || m_udp_from_rtp->GetPort() != from_rtp_port))
		{
			m_udp_from_rtp->Close();
			m_udp_from_rtp = nullptr;
		}
		if (m_udp_to_rtp && (m_udp_to_rtp->GetIp() != to_rtp_ip || m_udp_to_rtp->GetPort() != to_rtp_port))
		{
			m_udp_to_rtp->Close();
			m_udp_to_rtp = nullptr;
		}

		// 创建与呼叫方互发媒体包的rtp
		if (!m_udp_from_rtp)
		{
			auto udp_server = std::make_shared<CarpUdpServer>(schedule->GetIOService());
			CarpUdpServerWeakPtr udp_ptr = udp_server;
			udp_server->RegisterUdpHandle(std::bind(HandleFromRtp, std::placeholders::_1, self_weak_ptr, udp_ptr));
			if (!udp_server->Start(from_rtp_ip, from_rtp_port))
			{
				Close();
				return false;
			}
			m_udp_from_rtp = udp_server;
		}

		// 创建与被呼叫方互发媒体包的rtp
		if (!m_udp_to_rtp)
		{
			auto udp_server = std::make_shared<CarpUdpServer>(schedule->GetIOService());
			CarpUdpServerWeakPtr udp_ptr = udp_server;
			udp_server->RegisterUdpHandle(std::bind(HandleToRtp, std::placeholders::_1, self_weak_ptr, udp_ptr));
			if (!udp_server->Start(to_rtp_ip, to_rtp_port))
			{
				Close();
				return false;
			}
			m_udp_to_rtp = udp_server;
		}

		// 保存相关数据
		m_from_rtp_port = from_rtp_port;
		m_to_rtp_port = to_rtp_port;
		m_schedule = schedule;

		StopRecord();
		return true;
	}

	// 启动一个电话媒体包
	bool Start(const std::string& call_id)
	{
		// 打印正在使用的警告
		if (m_in_using) CARP_ERROR("m_call_id(" << m_call_id << "), m_from_rtp_port(" << m_from_rtp_port << "), m_to_rtp_port(" << m_to_rtp_port << "), already in used!");
		// 如果没有创建直接返回false
		if (!m_udp_from_rtp || !m_udp_to_rtp) return false;
		// 标记为正在使用
		m_in_using = true;

		// 初始化相关数据
		m_has_from_rtp_endpoint = false;
		m_has_to_rtp_endpoint = false;
		m_call_id = call_id;

		m_from_rtp_password = "";
		m_to_rtp_password = "";

		if (m_from_rtp_auth_timer)
		{
			asio::error_code ec;
			m_from_rtp_auth_timer->cancel(ec);
			m_from_rtp_auth_timer = nullptr;
		}
		if (m_to_rtp_auth_timer)
		{
			asio::error_code ec;
			m_to_rtp_auth_timer->cancel(ec);
			m_to_rtp_auth_timer = nullptr;
		}

		StopRecord();
		return true;
	}

	// 停止一个电话媒体包
	void Stop()
	{
		// 标记为不在使用
		m_in_using = false;

		// 更新空闲时间
		m_idle_time = time(nullptr);

		m_from_rtp_password = "";
		m_to_rtp_password = "";

		if (m_from_rtp_auth_timer)
		{
			asio::error_code ec;
			m_from_rtp_auth_timer->cancel(ec);
			m_from_rtp_auth_timer = nullptr;
		}
		if (m_to_rtp_auth_timer)
		{
			asio::error_code ec;
			m_to_rtp_auth_timer->cancel(ec);
			m_to_rtp_auth_timer = nullptr;
		}

		StopRecord();
	}

	// 设置和呼叫方互发RTP包的ip和端口
	void SetFromRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		m_from_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_from_rtp_endpoint = true;
	}

	// 发送和呼叫方的鉴权
	void SetFromAuth(const std::string& password)
	{
		m_from_rtp_password = password;

		// 获取弱引用
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		if (m_from_rtp_auth_timer)
		{
			asio::error_code ec;
			m_from_rtp_auth_timer->cancel(ec);
		}
		m_from_rtp_auth_timer = std::make_shared<CarpAsioTimer>(m_schedule->GetIOService(), std::chrono::seconds(10));
		m_from_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendFromAuth, std::placeholders::_1, self_weak_ptr));
	}

	// 设置和被呼叫方互发RTP包的ip和端口
	void SetToRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		m_to_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_to_rtp_endpoint = true;
	}

	// 发送和被叫方的鉴权
	void SetToAuth(const std::string& password)
	{
		m_to_rtp_password = password;

		// 获取弱引用
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		if (m_to_rtp_auth_timer)
		{
			asio::error_code ec;
			m_to_rtp_auth_timer->cancel(ec);
		}
		m_to_rtp_auth_timer = std::make_shared<CarpAsioTimer>(m_schedule->GetIOService(), std::chrono::seconds(10));
		m_to_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendToAuth, std::placeholders::_1, self_weak_ptr));
	}

	// 开始录音
	bool StartRecord(const std::string& file_path)
	{
		StopRecord();

#ifdef _WIN32
		fopen_s(&m_record_file, file_path.c_str(), "wb");
#else
		m_record_file = fopen(file_path.c_str(), "wb");
#endif
		return m_record_file != nullptr;
	}

	// 停止录音
	void StopRecord()
	{
		if (m_record_file != nullptr)
		{
			fclose(m_record_file);
			m_record_file = nullptr;
		}
	}

	// 关闭rtp
	void Close()
	{
		if (m_udp_from_rtp) m_udp_from_rtp->Close();
		m_udp_from_rtp = nullptr;
		if (m_udp_to_rtp) m_udp_to_rtp->Close();
		m_udp_to_rtp = nullptr;

		if (m_from_rtp_auth_timer)
		{
			asio::error_code ec;
			m_from_rtp_auth_timer->cancel(ec);
			m_from_rtp_auth_timer = nullptr;
		}
		if (m_to_rtp_auth_timer)
		{
			asio::error_code ec;
			m_to_rtp_auth_timer->cancel(ec);
			m_to_rtp_auth_timer = nullptr;
		}

		StopRecord();
	}

public:
	time_t GetIdleTime() const { return m_idle_time; }

private:
	// 空闲时间
	time_t m_idle_time = 0;
	// 是否在使用中
	bool m_in_using = false;

private:
	// 所有用于呼叫发起方互发的udp
	CarpUdpServerPtr m_udp_from_rtp;
    unsigned int m_from_rtp_port = 0;
	// 是否已接入呼叫发起方
	bool m_has_from_rtp_endpoint = false;
	// 呼叫发起方的endpoint
	asio::ip::udp::endpoint m_from_rtp_endpoint;

	// 呼叫方的鉴权密码
	std::string m_from_rtp_password;
	// 鉴权信息发送定时器
	CarpAsioTimerPtr m_from_rtp_auth_timer;

	// 所有用于被呼叫放互发的udp
	CarpUdpServerPtr m_udp_to_rtp;
	unsigned int m_to_rtp_port = 0;
	// 是否已接入被呼叫方
	bool m_has_to_rtp_endpoint = false;
	// 被呼叫方的endpoint
	asio::ip::udp::endpoint m_to_rtp_endpoint;
	// 被叫的鉴权密码
	std::string m_to_rtp_password;
	// 鉴权信息发送定时器
	CarpAsioTimerPtr m_to_rtp_auth_timer;
	
private:
	// 呼叫方录音文件
	FILE* m_record_file = nullptr;

private:
	std::string m_call_id;		// SIP呼叫ID
	CarpSchedule* m_schedule = nullptr;

private:
	// 处理主叫方的媒体包
	static void TimerSendFromAuth(const asio::error_code& ec, CarpRtpServerWeakPtr self)
	{
		if (ec) return;

		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 发送鉴权消息
		if (self_ptr->m_has_from_rtp_endpoint)
		{
			std::string content = "carp_nat_auth:" + self_ptr->m_from_rtp_password;
			// 发送
			void* new_memory = malloc(content.size());
			memcpy(new_memory, content.c_str(), content.size());
			self_ptr->m_udp_from_rtp->Send(new_memory, content.size(), self_ptr->m_from_rtp_endpoint);
		}

		if (!self_ptr->m_from_rtp_auth_timer) return;
		self_ptr->m_from_rtp_auth_timer->expires_after(std::chrono::seconds(10));
		self_ptr->m_from_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendFromAuth, std::placeholders::_1, self));
	}

	// 处理被叫方的媒体包
	static void TimerSendToAuth(const asio::error_code& ec, CarpRtpServerWeakPtr self)
	{
		if (ec) return;

		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 发送鉴权消息
		if (self_ptr->m_has_to_rtp_endpoint)
		{
			std::string content = "carp_nat_auth:" + self_ptr->m_to_rtp_password;
			// 发送
			void* new_memory = malloc(content.size());
			memcpy(new_memory, content.c_str(), content.size());
			self_ptr->m_udp_to_rtp->Send(new_memory, content.size(), self_ptr->m_to_rtp_endpoint);
		}

		if (!self_ptr->m_to_rtp_auth_timer) return;
		self_ptr->m_to_rtp_auth_timer->expires_after(std::chrono::seconds(10));
		self_ptr->m_to_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendToAuth, std::placeholders::_1, self));
	}

	// 处理被呼叫方的媒体包
	static void HandleToRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// 获取Transfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 检查是否正在使用
		if (self_ptr->m_in_using == false) return;

		// 如果没有线路，直接返回
		if (self_ptr->m_has_from_rtp_endpoint == false) return;

		// 写入文件
		if (self_ptr->m_record_file != nullptr)
		{
			fwrite(&info.memory_size, 1, sizeof(info.memory_size), self_ptr->m_record_file);
			fwrite(info.memory, 1, info.memory_size, self_ptr->m_record_file);
		}

		// 发送
		void* new_memory = malloc(info.memory_size);
		memcpy(new_memory, info.memory, info.memory_size);
		self_ptr->m_udp_from_rtp->Send(new_memory, info.memory_size, self_ptr->m_from_rtp_endpoint);
	}

	static void HandleFromRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// 获取Transfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 检查是否正在使用
		if (self_ptr->m_in_using == false) return;

		// 如果没有线路，直接返回
		if (self_ptr->m_has_to_rtp_endpoint == false) return;

		// 写入文件
		if (self_ptr->m_record_file != nullptr)
		{
			fwrite(&info.memory_size, 1, sizeof(info.memory_size), self_ptr->m_record_file);
			fwrite(info.memory, 1, info.memory_size, self_ptr->m_record_file);
		}

		// 发送
		void* new_memory = malloc(info.memory_size);
		memcpy(new_memory, info.memory, info.memory_size);
		self_ptr->m_udp_to_rtp->Send(new_memory, info.memory_size, self_ptr->m_to_rtp_endpoint);
	}
};

#endif