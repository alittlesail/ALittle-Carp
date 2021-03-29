#ifndef CARP_SIP_SERVER_INCLUDED
#define CARP_SIP_SERVER_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "carp_udp_server.hpp"
#include "carp_schedule.hpp"

#include "osipparser2/osip_parser.h"
#include "osip2/osip.h"

class CarpSipServer;
typedef std::shared_ptr<CarpSipServer> CarpSipServerPtr;
typedef std::weak_ptr<CarpSipServer> CarpSipServerWeakPtr;

class CarpSipServer : public std::enable_shared_from_this<CarpSipServer>
{
public:
	CarpSipServer() {}
	~CarpSipServer() { Close(); }

public:
	bool Start(const char* self_sip_ip, int self_sip_port
		, const char* remote_sip_ip, int remote_sip_port
		, CarpSchedule* schedule)
	{
		if (self_sip_ip == nullptr || remote_sip_ip == nullptr) return false;

		// 获取弱引用
		CarpSipServerWeakPtr self_weak_ptr = this->shared_from_this();
		// 创建线路sip
		m_udp_self_sip = std::make_shared<CarpUdpServer>(schedule->GetIOService());
		m_udp_self_sip->RegisterUdpHandle(std::bind(HandleRemoteSip, std::placeholders::_1, self_weak_ptr));
		if (!m_udp_self_sip->Start(self_sip_ip, self_sip_port))
		{
			Close();
			return false;
		}

		m_self_sip_ip = self_sip_ip;
		m_self_sip_port = self_sip_port;
		m_remote_sip_ip = remote_sip_ip;
		m_remote_sip_port = remote_sip_port;
		m_remote_sip_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(m_remote_sip_ip), m_remote_sip_port);

		// 构建osip
		if (osip_init(&m_osip) != 0)
		{
			Close();
			return false;
		}
		osip_set_application_context(m_osip, this);
		osip_set_cb_send_message(m_osip, &HandleUdpSendMessage);

		return true;
	}

	void Close()
	{
		if (m_udp_self_sip) m_udp_self_sip->Close();
		m_udp_self_sip = CarpUdpServerPtr();

		if (m_osip != nullptr) osip_release(m_osip);
		m_osip = nullptr;
	}

	void Run()
	{
		// 复制一份
		for (const auto& transaction : m_transaction_map)
		{
			while (true)
			{
				auto* event = static_cast<osip_event_t*>(osip_fifo_get(transaction->transactionff));
				if (event == nullptr) break;

				auto result = osip_transaction_execute(transaction, event);
			}
		}

		// 开始移除
		for (const auto& transaction : m_remove_map)
			m_transaction_map.erase(transaction);
		m_remove_map.clear();
	}

public:
	void RegisterAccount(const char* account, const char* password)
	{
		if (account == nullptr || password == nullptr) return;

		osip_message_t* msg = nullptr;
		osip_message_init(&msg);

		osip_message_set_method(msg, CreateString("REGISTER"));
	}

private:
	static char* CreateString(const char* content)
	{
		const auto size = strlen(content) + 1;
		char* string = static_cast<char*>(osip_malloc(size));
		if (string == nullptr) return nullptr;
		memcpy(string, content, size);
		return string;
	}

private:
	// 与线路方互发sip的udp
	CarpUdpServerPtr m_udp_self_sip;
	std::string m_self_sip_ip;
	int m_self_sip_port = 0;

	std::string m_remote_sip_ip;
	int m_remote_sip_port = 0;
	asio::ip::udp::endpoint m_remote_sip_endpoint;

private:
	// osip
	osip_t* m_osip = nullptr;
	// 所有sip集合
	std::set<osip_transaction_t*> m_transaction_map;
	std::set<osip_transaction_t*> m_remove_map;

private:
	// sip数据包处理
	static void HandleRemoteSip(CarpUdpServer::HandleInfo& info, CarpSipServerWeakPtr self)
	{
		// 获取Transfer
		const CarpSipServerPtr self_ptr = self.lock();
		if (!self_ptr) return;

		auto* event = osip_parse(info.memory, info.memory_size);
		osip_find_transaction_and_add_event(self_ptr->m_osip, event);
	}

	// 处理发送
	static int HandleUdpSendMessage(osip_transaction_t* ict, osip_message_t* sip, char* destination, int port, int out_socket)
	{
        auto* self = static_cast<CarpSipServer*>(ict->your_instance);
		if (self == nullptr) return -1;
		if (!self->m_udp_self_sip) return -1;

		char* dest = nullptr;
		size_t size = 0;
		osip_message_to_str(sip, &dest, &size);
		if (self->m_remote_sip_ip == destination && self->m_remote_sip_port == port)
		    self->m_udp_self_sip->Send(dest, self->m_remote_sip_endpoint);
		else
			self->m_udp_self_sip->Send(dest, asio::ip::udp::endpoint(asio::ip::address::from_string(destination), port));
		osip_free(dest);
		return 0;
	}
};


#endif