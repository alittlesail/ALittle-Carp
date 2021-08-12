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
	// ����
	// nat_ip ��ʾnat�������Լ���ip�Լ��˿�
	// target_ip ��ʾת��Ŀ��ip�Ͷ˿�
	bool Create(const std::string& nat_ip, unsigned int nat_port, CarpSchedule* schedule)
	{
		// ��ȡ������
		CarpNatServerWeakPtr self_weak_ptr = this->shared_from_this();

		// ���ip�Ͷ˿��Ƿ�Ե��ϣ��Բ��Ͼ͹رգ�Ȼ�����´���
		if (m_nat_udp && (m_nat_udp->GetIp() != nat_ip || m_nat_udp->GetPort() != nat_port))
		{
			m_nat_udp->Close();
			m_nat_udp = nullptr;
		}

		// �������ص�nat��udp
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

	// ����һ��Natת��
	bool Start()
	{
		// ��ӡ����ʹ�õľ���
		if (m_in_using) CARP_ERROR("m_nat_port(" << m_nat_port << ") already in used!");
		// ���û�д���ֱ�ӷ���false
		if (!m_nat_udp) return false;
		// ���Ϊ����ʹ��
		m_in_using = true;

		// ��ʼ���������
		m_has_client_endpoint = false;
		m_nat_password = "";
		return true;
	}

	// ֹͣһ��Natת��
	void Stop()
	{
		// ���Ϊ����ʹ��
		m_in_using = false;

		// ���¿���ʱ��
		m_idle_time = time(nullptr);
	}

	// ����Nat��Ȩ����
	void SetAuth(const std::string& password)
	{
		m_nat_password = password;

		// �����ǰ�ͻ��˺����õĲ�һ������ô������ͻ���
		if (m_client_password != password)
			m_has_client_endpoint = false;
	}

	// ����Ŀ�������
	void SetTarget(const std::string& ip, unsigned int port)
	{
		m_target_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(ip), port);
		m_has_target_endpoint = true;
	}

	// �ر�rtp
	void Close()
	{
		if (m_nat_udp) m_nat_udp->Close();
		m_nat_udp = nullptr;
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
	CarpUdpServerPtr m_nat_udp;
    unsigned int m_nat_port = 0;

	// ע��nat��Ȩ����
	std::string m_nat_password;

	// �Ƿ��ѽ�����з���
	bool m_has_client_endpoint = false;
	// ���з��𷽵�endpoint
	asio::ip::udp::endpoint m_client_endpoint;
	// ����from������
	std::string m_client_password;

	// ת����Ŀ��IP
	bool m_has_target_endpoint = false;
	asio::ip::udp::endpoint m_target_endpoint;

private:
	static void HandleNatData(CarpUdpServer::HandleInfo& info, CarpNatServerWeakPtr self, CarpUdpServerWeakPtr real_udp)
	{
		// ��ȡTransfer
		CarpNatServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		// ����Ƿ�����ʹ��
		if (self_ptr->m_in_using == false) return;

		// �����ж��Ƿ��Ǽ�Ȩudp
		if (!self_ptr->m_nat_password.empty() && info.memory_size > 0 && info.memory[0] == 'c')
		{
			// ����ж��Ƿ��Ǽ�Ȩ
			std::string content(info.memory, info.memory_size);
			if (content == "carp_nat_auth:" + self_ptr->m_nat_password)
			{
				self_ptr->m_has_client_endpoint = true;
				self_ptr->m_client_endpoint = info.end_point;
				return;
			}
		}

		// �����յ��ͻ��˵���Ϣ��ת����Ŀ�������
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

		// �����յ�Ŀ��ip����Ϣ��ת�����ͻ���
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