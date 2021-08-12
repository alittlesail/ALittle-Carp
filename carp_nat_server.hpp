#ifndef CARP_NAT_SERVER_INCLUDED
#define CARP_NAT_SERVER_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "carp_udp_server.hpp"
#include "carp_schedule.hpp"

class CarpNatServer;
typedef std::shared_ptr<CarpNatServer> CarpNatServerPtr;
typedef std::weak_ptr<CarpNatServer> CarpNatServerWeakPtr;

class CarpNatServer : public std::enable_shared_from_this<CarpNatServer>
{
public:
	CarpNatServer() {}
	~CarpNatServer() { Close(); }

public:
	// 创建
	// nat_ip 表示nat服务器自己的ip以及端口
	// target_ip 表示转发目标ip和端口
	bool Create(const std::string& nat_ip, unsigned int nat_port, CarpSchedule* schedule)
	{
		// 获取弱引用
		CarpNatServerWeakPtr self_weak_ptr = this->shared_from_this();

		// 检查ip和端口是否对的上，对不上就关闭，然后重新创建
		if (m_nat_udp && (m_nat_udp->GetIp() != nat_ip || m_nat_udp->GetPort() != nat_port))
		{
			m_nat_udp->Close();
			m_nat_udp = nullptr;
		}

		// 创建本地的nat的udp
		if (!m_nat_udp)
		{
			auto udp_server = std::make_shared<CarpUdpServer>(schedule->GetIOService());
			CarpUdpServerWeakPtr udp_ptr = udp_server;
			udp_server->RegisterUdpHandle(std::bind(HandleNatData, std::placeholders::_1, self_weak_ptr, udp_ptr));
			if (!udp_server->Start(nat_ip, nat_port))
			{
				Close();
				return false;
			}
			m_nat_udp = udp_server;
		}

		return true;
	}

	// 启动一个Nat转发
	bool Start()
	{
		// 打印正在使用的警告
		if (m_in_using) CARP_ERROR("m_nat_port(" << m_nat_port << ") already in used!");
		// 如果没有创建直接返回false
		if (!m_nat_udp) return false;
		// 标记为正在使用
		m_in_using = true;

		// 初始化相关数据
		m_has_client_endpoint = false;
		m_nat_password = "";
		return true;
	}

	// 停止一个Nat转发
	void Stop()
	{
		// 标记为不在使用
		m_in_using = false;

		// 更新空闲时间
		m_idle_time = time(nullptr);
	}

	// 设置Nat鉴权密码
	void SetAuth(const std::string& password)
	{
		m_nat_password = password;

		// 如果当前客户端和设置的不一样，那么就清理客户端
		if (m_client_password != password)
			m_has_client_endpoint = false;
	}

	// 设置目标服务器
	void SetTarget(const std::string& ip, unsigned int port)
	{
		m_target_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(ip), port);
		m_has_target_endpoint = true;
	}

	// 关闭rtp
	void Close()
	{
		if (m_nat_udp) m_nat_udp->Close();
		m_nat_udp = nullptr;
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
	CarpUdpServerPtr m_nat_udp;
    unsigned int m_nat_port = 0;

	// 注册nat鉴权密码
	std::string m_nat_password;

	// 是否已接入呼叫发起方
	bool m_has_client_endpoint = false;
	// 呼叫发起方的endpoint
	asio::ip::udp::endpoint m_client_endpoint;
	// 保存from的密码
	std::string m_client_password;

	// 转发的目标IP
	bool m_has_target_endpoint = false;
	asio::ip::udp::endpoint m_target_endpoint;

private:
	static void HandleNatData(CarpUdpServer::HandleInfo& info, CarpNatServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// 获取Transfer
		CarpNatServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// 检查是否正在使用
		if (self_ptr->m_in_using == false) return;

		// 初步判断是否是鉴权udp
		if (!self_ptr->m_nat_password.empty() && info.memory_size > 0 && info.memory[0] == 'c')
		{
			// 深度判断是否是鉴权
			std::string content(info.memory, info.memory_size);
			if (content == "carp_nat_auth:" + self_ptr->m_nat_password)
			{
				self_ptr->m_has_client_endpoint = true;
				self_ptr->m_client_endpoint = info.end_point;
				return;
			}
		}

		// 处理收到客户端的消息，转发给目标服务器
		if (self_ptr->m_has_client_endpoint && self_ptr->m_client_endpoint == info.end_point)
		{
			if (self_ptr->m_has_target_endpoint)
			{
				void* new_memory = malloc(info.memory_size);
				memcpy(new_memory, info.memory, info.memory_size);
				self_ptr->m_nat_udp->Send(new_memory, info.memory_size, self_ptr->m_target_endpoint);
			}

			return;
		}

		// 处理收到目标ip的消息，转发给客户端
		if (self_ptr->m_has_target_endpoint && self_ptr->m_target_endpoint == info.end_point)
		{
			if (self_ptr->m_has_client_endpoint)
			{
				void* new_memory = malloc(info.memory_size);
				memcpy(new_memory, info.memory, info.memory_size);
				self_ptr->m_nat_udp->Send(new_memory, info.memory_size, self_ptr->m_client_endpoint);
			}

			return;
		}
	}
};

#endif