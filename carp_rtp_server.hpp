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
	// ��ʼ��
	// from_rtp_ip ��ʾҪʹ�õ�ip�����ں��з��𷽻���ý�����
	// to_rtp_ip ��ʾҪʹ�õ�ip�����ڱ����зŻ���ý���
	bool Create(const std::string& from_rtp_ip, unsigned int from_rtp_port
		, const std::string& to_rtp_ip, unsigned int to_rtp_port
		, CarpSchedule* schedule)
	{
		// ��ȡ������
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		// ���ip�Ͷ˿��Ƿ�Ե��ϣ��Բ��Ͼ͹رգ�Ȼ�����´���
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

		// ��������з�����ý�����rtp
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

		// �����뱻���з�����ý�����rtp
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

		// �����������
		m_from_rtp_port = from_rtp_port;
		m_to_rtp_port = to_rtp_port;
		m_schedule = schedule;

		StopRecord();
		return true;
	}

	// ����һ���绰ý���
	bool Start(const std::string& call_id)
	{
		// ��ӡ����ʹ�õľ���
		if (m_in_using) CARP_ERROR("m_call_id(" << m_call_id << "), m_from_rtp_port(" << m_from_rtp_port << "), m_to_rtp_port(" << m_to_rtp_port << "), already in used!");
		// ���û�д���ֱ�ӷ���false
		if (!m_udp_from_rtp || !m_udp_to_rtp) return false;
		// ���Ϊ����ʹ��
		m_in_using = true;

		// ��ʼ���������
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

	// ֹͣһ���绰ý���
	void Stop()
	{
		// ���Ϊ����ʹ��
		m_in_using = false;

		// ���¿���ʱ��
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

	// ���úͺ��з�����RTP����ip�Ͷ˿�
	void SetFromRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		m_from_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_from_rtp_endpoint = true;
	}

	// ���ͺͺ��з��ļ�Ȩ
	void SetFromAuth(const std::string& password)
	{
		m_from_rtp_password = password;

		// ��ȡ������
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		if (m_from_rtp_auth_timer)
		{
			asio::error_code ec;
			m_from_rtp_auth_timer->cancel(ec);
		}
		m_from_rtp_auth_timer = std::make_shared<CarpAsioTimer>(m_schedule->GetIOService(), std::chrono::seconds(10));
		m_from_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendFromAuth, std::placeholders::_1, self_weak_ptr));
	}

	// ���úͱ����з�����RTP����ip�Ͷ˿�
	void SetToRtp(const std::string& rtp_ip, unsigned int rtp_port)
	{
		m_to_rtp_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(rtp_ip), rtp_port);
		m_has_to_rtp_endpoint = true;
	}

	// ���ͺͱ��з��ļ�Ȩ
	void SetToAuth(const std::string& password)
	{
		m_to_rtp_password = password;

		// ��ȡ������
		CarpRtpServerWeakPtr self_weak_ptr = this->shared_from_this();

		if (m_to_rtp_auth_timer)
		{
			asio::error_code ec;
			m_to_rtp_auth_timer->cancel(ec);
		}
		m_to_rtp_auth_timer = std::make_shared<CarpAsioTimer>(m_schedule->GetIOService(), std::chrono::seconds(10));
		m_to_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendToAuth, std::placeholders::_1, self_weak_ptr));
	}

	// ��ʼ¼��
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

	// ֹͣ¼��
	void StopRecord()
	{
		if (m_record_file != nullptr)
		{
			fclose(m_record_file);
			m_record_file = nullptr;
		}
	}

	// �ر�rtp
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
	// ����ʱ��
	time_t m_idle_time = 0;
	// �Ƿ���ʹ����
	bool m_in_using = false;

private:
	// �������ں��з��𷽻�����udp
	CarpUdpServerPtr m_udp_from_rtp;
    unsigned int m_from_rtp_port = 0;
	// �Ƿ��ѽ�����з���
	bool m_has_from_rtp_endpoint = false;
	// ���з��𷽵�endpoint
	asio::ip::udp::endpoint m_from_rtp_endpoint;

	// ���з��ļ�Ȩ����
	std::string m_from_rtp_password;
	// ��Ȩ��Ϣ���Ͷ�ʱ��
	CarpAsioTimerPtr m_from_rtp_auth_timer;

	// �������ڱ����зŻ�����udp
	CarpUdpServerPtr m_udp_to_rtp;
	unsigned int m_to_rtp_port = 0;
	// �Ƿ��ѽ��뱻���з�
	bool m_has_to_rtp_endpoint = false;
	// �����з���endpoint
	asio::ip::udp::endpoint m_to_rtp_endpoint;
	// ���еļ�Ȩ����
	std::string m_to_rtp_password;
	// ��Ȩ��Ϣ���Ͷ�ʱ��
	CarpAsioTimerPtr m_to_rtp_auth_timer;
	
private:
	// ���з�¼���ļ�
	FILE* m_record_file = nullptr;

private:
	std::string m_call_id;		// SIP����ID
	CarpSchedule* m_schedule = nullptr;

private:
	// �������з���ý���
	static void TimerSendFromAuth(const asio::error_code& ec, CarpRtpServerWeakPtr self)
	{
		if (ec) return;

		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// ���ͼ�Ȩ��Ϣ
		if (self_ptr->m_has_from_rtp_endpoint)
		{
			std::string content = "carp_nat_auth:" + self_ptr->m_from_rtp_password;
			// ����
			void* new_memory = malloc(content.size());
			memcpy(new_memory, content.c_str(), content.size());
			self_ptr->m_udp_from_rtp->Send(new_memory, content.size(), self_ptr->m_from_rtp_endpoint);
		}

		if (!self_ptr->m_from_rtp_auth_timer) return;
		self_ptr->m_from_rtp_auth_timer->expires_after(std::chrono::seconds(10));
		self_ptr->m_from_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendFromAuth, std::placeholders::_1, self));
	}

	// �����з���ý���
	static void TimerSendToAuth(const asio::error_code& ec, CarpRtpServerWeakPtr self)
	{
		if (ec) return;

		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// ���ͼ�Ȩ��Ϣ
		if (self_ptr->m_has_to_rtp_endpoint)
		{
			std::string content = "carp_nat_auth:" + self_ptr->m_to_rtp_password;
			// ����
			void* new_memory = malloc(content.size());
			memcpy(new_memory, content.c_str(), content.size());
			self_ptr->m_udp_to_rtp->Send(new_memory, content.size(), self_ptr->m_to_rtp_endpoint);
		}

		if (!self_ptr->m_to_rtp_auth_timer) return;
		self_ptr->m_to_rtp_auth_timer->expires_after(std::chrono::seconds(10));
		self_ptr->m_to_rtp_auth_timer->async_wait(std::bind(&CarpRtpServer::TimerSendToAuth, std::placeholders::_1, self));
	}

	// �������з���ý���
	static void HandleToRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// ��ȡTransfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// ����Ƿ�����ʹ��
		if (self_ptr->m_in_using == false) return;

		// ���û����·��ֱ�ӷ���
		if (self_ptr->m_has_from_rtp_endpoint == false) return;

		// д���ļ�
		if (self_ptr->m_record_file != nullptr)
		{
			fwrite(&info.memory_size, 1, sizeof(info.memory_size), self_ptr->m_record_file);
			fwrite(info.memory, 1, info.memory_size, self_ptr->m_record_file);
		}

		// ����
		void* new_memory = malloc(info.memory_size);
		memcpy(new_memory, info.memory, info.memory_size);
		self_ptr->m_udp_from_rtp->Send(new_memory, info.memory_size, self_ptr->m_from_rtp_endpoint);
	}

	static void HandleFromRtp(CarpUdpServer::HandleInfo& info, CarpRtpServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// ��ȡTransfer
		CarpRtpServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// ����Ƿ�����ʹ��
		if (self_ptr->m_in_using == false) return;

		// ���û����·��ֱ�ӷ���
		if (self_ptr->m_has_to_rtp_endpoint == false) return;

		// д���ļ�
		if (self_ptr->m_record_file != nullptr)
		{
			fwrite(&info.memory_size, 1, sizeof(info.memory_size), self_ptr->m_record_file);
			fwrite(info.memory, 1, info.memory_size, self_ptr->m_record_file);
		}

		// ����
		void* new_memory = malloc(info.memory_size);
		memcpy(new_memory, info.memory, info.memory_size);
		self_ptr->m_udp_to_rtp->Send(new_memory, info.memory_size, self_ptr->m_to_rtp_endpoint);
	}
};

#endif