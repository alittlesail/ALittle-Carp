#ifndef CARP_RUDP_SERVER_INCLUDED
#define CARP_RUDP_SERVER_INCLUDED

#include <asio.hpp>
#include <memory>
#include <unordered_map>
#include <functional>

#include "carp_log.hpp"
#include "carp_message.hpp"
#include "carp_schedule.hpp"
#include "carp_safe_id_creator.hpp"
#include "kcp/ikcp.h"

class CarpRudpReceiver;
typedef std::shared_ptr<CarpRudpReceiver> CarpRudpReceiverPtr;
typedef std::weak_ptr<CarpRudpReceiver> CarpRudpReceiverWeakPtr;

typedef std::shared_ptr<asio::ip::udp::socket> CarpUSocketPtr;
typedef asio::basic_waitable_timer<std::chrono::system_clock> AsioTimer;
typedef std::shared_ptr<AsioTimer> AsioTimerPtr;

class CarpRudpInterface
{
public:
	virtual ~CarpRudpInterface() {}

	virtual void HandleRudpConnect(CarpRudpReceiverPtr sender) { }

	virtual void HandleRudpDisconnect(CarpRudpReceiverPtr sender) { }

	virtual void HandleRudpMessage(CarpRudpReceiverPtr sender, int message_size, int message_id, int message_rpcid, void* memory) {}
};

class CarpRudpServer : public std::enable_shared_from_this<CarpRudpServer>
{
public:
	virtual ~CarpRudpServer() {}

	// �����͹ر�
	virtual bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpRudpInterface* rudp_interface, CarpSchedule* schedule) = 0;
	virtual void Close(bool exit) = 0;

	// ��ȡip�Ͷ˿�
	virtual const std::string& GetYunIp() const = 0;
	virtual const std::string& GetIp() const = 0;
	virtual int GetPort() const = 0;

	virtual CarpUSocketPtr GetSocket() = 0;

	virtual void SendPocket(const asio::ip::udp::endpoint& endpoint, void* memory, int size) = 0;

	// ���������ص�
	virtual void ServerSendHeartbeat(const asio::error_code& ec, int interval) = 0;

	// ����kcp
	virtual void ServerUpdateKcp(const asio::error_code& ec, int interval) = 0;

	// �����ͻص�
	virtual void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory) = 0;

	// �����������
	virtual void HandleRead(const asio::error_code& ec, std::size_t actual_size) = 0;
	
	// ������Ϣ�¼�
	virtual void HandleRudpMessage(CarpRudpReceiverPtr receiver, CARP_MESSAGE_SIZE message_size, CARP_MESSAGE_ID message_id, CARP_MESSAGE_RPCID message_rpcid, void* memory) = 0;
};
typedef std::shared_ptr<CarpRudpServer> CarpRudpServerPtr;
typedef std::weak_ptr<CarpRudpServer> CarpRudpServerWeakPtr;

class CarpRudpReceiver : public std::enable_shared_from_this<CarpRudpReceiver>
{
public:
	CarpRudpReceiver(const asio::ip::udp::endpoint& endpoint, int session, uint32_t conv, CarpRudpServerWeakPtr server, CarpRudpInterface* rudp_interface, CarpSchedule* schedule)
		: m_endpoint(endpoint), m_session(session), m_server(server), m_rudp_interface(rudp_interface), m_schedule(schedule)
	{
		// ��ȡ�ͻ��˵Ĺ���IP
		m_remote_ip = m_endpoint.address().to_string();
		m_remote_port = m_endpoint.port();

		// ����kcp
		m_kcp = ikcp_create(conv, this);
		// ����Ϊ��ģʽ
		m_kcp->stream = 1;
		// ���÷��ͺ���
		m_kcp->output = UdpOutput;
		// ����Ϊ����ģʽ
		ikcp_nodelay(m_kcp, 1, 10, 2, 1);
		// �ڳ��ĸ��ֽ����ڱ���session
		ikcp_setmtu(m_kcp, static_cast<int>(m_kcp->mtu) - static_cast<int>(sizeof(int)));
	}
	
	~CarpRudpReceiver()
	{
		// �ر�socket���ͷ���Դ
		Close(false);
	}

	friend class CarpRudpServerImpl;

public:
	bool CheckSession(int session) const { return m_session == session; }
	bool CheckHeartbeat(time_t cur_time, int interval)
	{
		// ������Ϊ0����ô�ͼ��ɹ�
		if (m_last_heartbeat == 0) return true;
		// ������ʱ�䳬������ô�ͼ��ʧ��
		if (cur_time - m_last_heartbeat >= interval) return false;
		// ���浱ǰʱ�䣬�����´μ��
		m_last_heartbeat = cur_time;
		return true;
	}
	void UpdateKcp(time_t cur_time_ms) const
	{
		if (m_kcp == nullptr) return;
		ikcp_update(m_kcp, static_cast<uint32_t>(cur_time_ms));
	}

	static int UdpOutput(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		auto* self = static_cast<CarpRudpReceiver*>(user);

		auto server = self->m_server.lock();
		if (!server) return 0;
		
		// ����
		char* memory = static_cast<char*>(malloc(len + sizeof(int)));
		// ��䵱ǰsession
		memcpy(memory, &self->m_session, sizeof(int));
		// ��䷢������
		memcpy(memory + sizeof(int), buf, len);

		// ��������
		server->SendPocket(self->m_endpoint, memory, static_cast<int>(len + sizeof(int)));
		return 0;
	}
	
	// �ر����ӣ��ͷ��ڴ�
	void Close(bool exit)
	{
		if (m_is_connected)
		{
			auto server = m_server.lock();
			if (server && m_kcp)
			{
				if (exit && server->GetSocket())
				{
					const auto size = sizeof(int) + sizeof(uint32_t);
					char buffer[size] = { 0 };
					auto n_session = -m_session; // ����ʹ�ø�����session����Ϊ�رղ���
					memcpy(buffer, &n_session, sizeof(int));
					memcpy(buffer + sizeof(int), &m_kcp->conv, sizeof(uint32_t));
					server->GetSocket()->send_to(asio::buffer(buffer, size), m_endpoint);
				}
				else
				{
					const auto size = sizeof(int) + sizeof(uint32_t);
					char* memory = static_cast<char*>(malloc(size));
					auto n_session = -m_session; // ����ʹ�ø�����session����Ϊ�رղ���
					memcpy(memory, &n_session, sizeof(int));
					memcpy(memory + sizeof(int), &m_kcp->conv, sizeof(uint32_t));
					server->SendPocket(m_endpoint, memory, size);
				}
			}
		}
		m_is_connected = false;
		// ����ƫ��
		m_kcp_data_size = 0;

		// �ͷ�kcp
		if (m_kcp)
		{
			ikcp_release(m_kcp);
			m_kcp = nullptr;
		}
	}

	//���ܲ���/////////////////////////////////////////////////////////////////////////////////
private:
	void HandleRead(const asio::ip::udp::endpoint& endpoint, const char* buffer, size_t actual_size)
	{
		// �������µ�endpoint
		m_endpoint = endpoint;
		
		// ������ι��kcp
		int result = ikcp_input(m_kcp, buffer, static_cast<int>(actual_size));
		if (result < 0) CARP_ERROR("ikcp_input:" << result);

		// �������п��Խ��յ�����
		while (true)
		{
			const int peek_size = ikcp_peeksize(m_kcp);
			if (peek_size <= 0) break;

			if (m_kcp_buffer.size() < peek_size + m_kcp_data_size)
				m_kcp_buffer.resize(peek_size + m_kcp_data_size);

			const auto real_len = ikcp_recv(m_kcp, m_kcp_buffer.data() + m_kcp_data_size, static_cast<int>(m_kcp_buffer.size() - m_kcp_data_size));
			if (real_len <= 0)
			{
				CARP_ERROR("ikcp_recv:" << real_len);
				break;
			}

			m_kcp_data_size += real_len;
		}

		// ��ʼ��������
		size_t kcp_data_offset = 0;
		while (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE <= m_kcp_data_size)
		{
			char* memory = m_kcp_buffer.data() + kcp_data_offset;
			// ��ȡЭ���С
			const auto message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(memory);
			CARP_MESSAGE_ID* message_id = reinterpret_cast<CARP_MESSAGE_ID*>(memory + sizeof(CARP_MESSAGE_SIZE));
			CARP_MESSAGE_RPCID* message_rpcid = reinterpret_cast<CARP_MESSAGE_RPCID*>(memory + sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID));

			// ���Э��������ݲ��㣬��ô������
			if (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE + message_size > m_kcp_data_size) break;
			kcp_data_offset += CARP_PROTOCOL_HEAD_SIZE;

			// �����ڴ�
			memory = static_cast<char*>(malloc(message_size));
			memcpy(memory, m_kcp_buffer.data() + kcp_data_offset, message_size);
			// ƫ�������
			kcp_data_offset += message_size;

			// ���������������ô�ͱ��
			if (*message_id == HeartbeatMessage::GetStaticID()) m_last_heartbeat = 0;

			// ͨ����Ϣ����ִ�з��Ͳ���
			m_schedule->Execute(std::bind(&CarpRudpServer::HandleRudpMessage, m_server.lock(), this->shared_from_this()
				, message_size, *message_id, *message_rpcid, memory));
		}

		// �ƶ��ڴ�
		if (kcp_data_offset > 0)
		{
			memmove(m_kcp_buffer.data(), m_kcp_buffer.data() + kcp_data_offset, m_kcp_data_size - kcp_data_offset);
			m_kcp_data_size -= kcp_data_offset;
		}
	}

public:
	// ��ȡ�ͻ��˵Ĺ���IP�Ͷ˿�
	const std::string& GetRemoteIP() const { return m_remote_ip; }
	int GetRemotePort() const { return m_remote_port; }
	bool IsConnected() const { return m_is_connected; }

private:
	asio::ip::udp::endpoint m_endpoint;	// endpoint
	int m_session = 0;
	ikcpcb* m_kcp = nullptr;

private:
	std::string m_remote_ip;			// �ͻ��˵Ĺ���IP
	int m_remote_port = 0;				// �ͻ��˵Ĺ����˿�

private:
	std::vector<char> m_kcp_buffer;					// ���ڽ���kcp���ݵ�
	size_t m_kcp_data_size = 0;
	// �����ķ�����
	CarpRudpServerWeakPtr m_server;
	// ��Ӧ����ģ��
	CarpRudpInterface* m_rudp_interface = nullptr;
	// ����ģ��
	CarpSchedule* m_schedule = nullptr;

public:
	// ����Э��
	void Send(const CarpMessage& message)
	{
		// ����Ѿ��رգ���ô�Ͳ��������ݰ�
		if (m_is_connected == false) return;
		if (m_kcp == nullptr) return;

		// ��ȡ��Ϣ���ܴ�С
		CARP_MESSAGE_SIZE message_size = message.GetTotalSize();
		// ��ȡ��Ϣ��ID
		CARP_MESSAGE_ID message_id = message.GetID();
		// ��ȡRPCID
		CARP_MESSAGE_RPCID message_rpcid = message.GetRpcID();

		// �����ڴ��С
		const int memory_size = CARP_PROTOCOL_HEAD_SIZE + message_size;

		// �����ڴ�
		char* memory = static_cast<char*>(malloc(memory_size));
		if (memory == nullptr)
		{
			CARP_ERROR("memory is null");
			return;
		}
		char* body_memory = memory;

		// д����Ϣ����С��ID
		memcpy(body_memory, &message_size, sizeof(CARP_MESSAGE_SIZE));
		body_memory += sizeof(CARP_MESSAGE_SIZE);
		memcpy(body_memory, &message_id, sizeof(CARP_MESSAGE_ID));
		body_memory += sizeof(CARP_MESSAGE_ID);
		memcpy(body_memory, &message_rpcid, sizeof(CARP_MESSAGE_RPCID));
		body_memory += sizeof(CARP_MESSAGE_RPCID);

		// ϵ�л���Ϣ
		message.Serialize(body_memory);

		// ����ÿ�η��͵�����ֽ���
		const int max_size = static_cast<int>(m_kcp->mss * m_kcp->rcv_wnd);

		// ��һ����������ֽ��������и�
		const char* body = memory;
		int offset = 0;
		while (offset < memory_size)
		{
			int send = memory_size - offset;
			if (send > max_size) send = max_size;
			int result = ikcp_send(m_kcp, body + offset, send);
			if (result < 0) CARP_ERROR("ikcp_send:" << result);
			offset += send;
		}

		free(memory);
	}
	
private:
	bool m_is_connected = true;// �Ƿ�������״̬
	time_t m_last_heartbeat = 0;	// �ϴη���������
};

#define CARP_UDP_SERVER_BUFFER_SIZE 10240 // ����һ�����ֵ���϶��ᳬ��һ��udp

class CarpRudpServerImpl : public CarpRudpServer
{
public:
	CarpRudpServerImpl()
	{
		srand(static_cast<unsigned int>(time(nullptr)));
	}
	
	virtual ~CarpRudpServerImpl() { Close(true); }

	friend CarpRudpReceiver;

public:
	// ����������
	// yun_ip �Ʒ�������ӳ��ip
	// ip	��������IP
	// port �������Ķ˿�
	// heartbeat TCP������������ʱ�䣬��λ��
	bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpRudpInterface* rudp_interface, CarpSchedule* schedule) override
	{
		m_rudp_interface = rudp_interface;
		m_schedule = schedule;

		// ����Ѿ������˾�ֱ�ӷ���
		if (m_socket)
		{
			CARP_ERROR("RudpServer already started(ip: " << m_ip << ", port:" << m_port << ")");
			return false;
		}

		// ����һ����������������
		m_socket = std::make_shared<asio::ip::udp::socket>(schedule->GetIOService());
		asio::ip::udp::endpoint endpoint = asio::ip::udp::endpoint(asio::ip::address_v4::from_string(ip), port);

		// ��socket
		asio::error_code ec;
		m_socket->open(endpoint.protocol(), ec);
		if (ec)
		{
			CARP_ERROR("RudpServer udp socket open error: " << ec.value());
			return false;
		}
		// �󶨶˿�
		m_socket->bind(endpoint, ec);
		if (ec)
		{
			CARP_ERROR("RudpServer udp socket bind error: " << ec.value());
			return false;
		}
		
		// ������ʱ��
		m_heartbeat_timer = std::make_shared<AsioTimer>(schedule->GetIOService(), std::chrono::seconds(heartbeat));
		m_heartbeat_timer->async_wait(std::bind(&CarpRudpServer::ServerSendHeartbeat, this->shared_from_this(), std::placeholders::_1, heartbeat));

		// ������ʱ��
		m_kcp_timer = std::make_shared<AsioTimer>(schedule->GetIOService(), std::chrono::milliseconds(20));
		m_kcp_timer->async_wait(std::bind(&CarpRudpServer::ServerUpdateKcp, this->shared_from_this(), std::placeholders::_1, 20));

		// ��ʼ����
		NextRead();

		// ����ip�˿�
		m_yun_ip = yun_ip;
		m_ip = ip;
		m_port = port;

		CARP_SYSTEM("RudpServer: start succeed at " << m_ip << ":" << m_port);
		return true;
	}

	// �رշ�����
	void Close(bool exit) override
	{
		// �ر����пͻ�������
		for (auto& pair : m_outer_map)
		{
			m_conv_creator.ReleaseID(pair.first);
			pair.second->Close(exit);
		}
		m_outer_map.clear();

		// �ͷŴ����͵���Ϣ��
		for (auto& info : m_pocket_list) free(info.memory);
		m_pocket_list.clear();

		// ���Ϊ�������ڷ���
		m_executing = false;
		// �رղ��ͷŽ�����
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
			m_socket = CarpUSocketPtr();
		}

		// �ͷŶ�ʱ��
		if (m_heartbeat_timer)
		{
			m_heartbeat_timer->cancel();
			m_heartbeat_timer = AsioTimerPtr();
		}

		// �ͷŶ�ʱ��
		if (m_kcp_timer)
		{
			m_kcp_timer->cancel();
			m_kcp_timer = AsioTimerPtr();
		}

		CARP_SYSTEM("RudpServer: stop succeed.");
	}

	CarpUSocketPtr GetSocket() override { return m_socket; }

private:
	// �������ݰ�
	void NextRead()
	{
		// ���������Ƿ�Ϸ�
		if (!m_socket) return;

		// ��ʼ�ȴ�����
		m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_receiver
			, std::bind(&CarpRudpServer::HandleRead, this->shared_from_this()
			, std::placeholders::_1, std::placeholders::_2));
	}

	// �����µ�socket����
	void HandleRead(const asio::error_code& ec, std::size_t actual_size) override
	{
		if (ec)
		{
			CARP_ERROR("RudpServer read failed: " << ec.value());
			NextRead();
			return;
		}

		const auto size = sizeof(int) + sizeof(uint32_t);
		// ��������Ч��
		if (actual_size < size)
		{
			NextRead();
			return;
		}

		// ȡsession
		auto session = *reinterpret_cast<int*>(m_udp_buffer);
		// ȡconv
		auto conv = ikcp_getconv(m_udp_buffer + sizeof(int));

		// �ж��Ƿ�����������
		if (session == 0 && conv == 0)
		{
			// ��������ٴ���conv�ˣ���ô�ͱ�ʾ�Ͽ�
			if (m_conv_creator.IsEmpty())
			{
				void* memory = malloc(size);
				memset(memory, 0, size);
				SendPocket(m_receiver, memory, size);

				NextRead();
				return;
			}

			// ��ȡ�µ�conv
			conv = m_conv_creator.CreateID();
			// ����һ��session
			session = rand();

			// ��������Ӧ��
			{
				char* memory = static_cast<char*>(malloc(size));
				memcpy(memory, &session, sizeof(int));
				memcpy(memory + sizeof(int), &conv, sizeof(uint32_t));
				SendPocket(m_receiver, memory, size);
			}

			// �����µ�����
			HandleOuterConnect(m_receiver, session, conv);

			NextRead();
			return;
		}
		
		// �ж��Ƿ��������ر�����
		if (session < 0 && conv != 0)
		{
			// ������Ӷ����session
			const auto it = m_outer_map.find(conv);
			if (it != m_outer_map.end() && it->second->CheckSession(-session))
				HandleOuterDisconnected(conv);

			NextRead();
			return;
		}
		
		// �ж��Ƿ��Ƿ�������
		if (session > 0 && conv != 0)
		{
			// ������Ӷ����session
			auto it = m_outer_map.find(conv);
			if (it != m_outer_map.end() && it->second->CheckSession(session))
				it->second->HandleRead(m_receiver, m_udp_buffer + sizeof(int), actual_size - sizeof(int));

			NextRead();
			return;
		}
		
		// ʣ�µĶ�����Ч����

		// ������һ������
		NextRead();
	}

public:
	void SendPocket(const asio::ip::udp::endpoint& endpoint, void* memory, int size) override
	{
		// �����ڴ�ṹ
		PocketInfo info;
		info.memory_size = size;
		info.memory = memory;
		info.endpoint = endpoint;

		// ��ӵ��������б�
		m_pocket_list.push_back(info);
		// ����Ѿ����ڷ����ˣ���ô��ֱ�ӷ���
		if (m_executing) return;
		// ������ڷ���
		m_executing = true;
		// ����һ����Ϣ��
		NextSend();
	}

	void NextSend()
	{
		// ������б��ǿյģ�����socket�Ѿ��ر��ˣ�ֱ�ӷ���
		if (m_pocket_list.empty() || !m_socket)
		{
			m_executing = false;
			return;
		}

		// ��ȡһ���ṹ��
		auto info = m_pocket_list.front();
		m_pocket_list.pop_front();

		// ����
		m_socket->async_send_to(asio::buffer(info.memory, info.memory_size), info.endpoint
			, std::bind(&CarpRudpServer::HandleSend, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, info.memory));
	}
	void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory) override
	{
		// �ͷ��ڴ�
		if (memory) free(memory);

		// ������
		if (ec)
		{
			CARP_ERROR("rudp server send failed: " << ec.value());
			return;
		}

		// ������һ����
		NextSend();
	}
	
public:
	// ��ȡ���صķ�����ip�Ͷ˿�
	const std::string& GetYunIp() const override { return m_yun_ip; }
	const std::string& GetIp() const override { return m_ip; }
	int GetPort() const override { return m_port; }

private:
	std::string m_yun_ip;       // �Ʒ����������IP
	std::string m_ip;			// ���ط�������IP
	int m_port = 0;				// ���ط������Ķ˿�

private:
	CarpUSocketPtr m_socket;
	char m_udp_buffer[CARP_UDP_SERVER_BUFFER_SIZE] = {};
	asio::ip::udp::endpoint m_receiver;			// ������ϢԴ

private:
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; asio::ip::udp::endpoint endpoint; };
	std::list<PocketInfo> m_pocket_list;  // �����͵����ݰ��б�
	bool m_executing = false;	// is in sending

///////////////////////////////////////////////////////////////////////////////////////////////

private:
	// ����һ���µ�socket
	void HandleOuterConnect(const asio::ip::udp::endpoint& endpoint, int session, uint32_t conv)
	{
		// ����һ���ͻ�������
		const auto receiver = std::make_shared<CarpRudpReceiver>(endpoint, session, conv, this->shared_from_this(), m_rudp_interface, m_schedule);
		// ��������
		m_outer_map[conv] = receiver;

		// ֪ͨ�ͻ������ӽ�����
		m_rudp_interface->HandleRudpConnect(receiver);
	}

	// ����ĳ�����ӶϿ���
	void HandleOuterDisconnected(uint32_t conv)
	{
		auto it = m_outer_map.find(conv);
		if (it == m_outer_map.end()) return;

		const auto receiver = it->second;
		
		// �رղ��Ƴ��ͻ�������
		it->second->Close(false);
		m_outer_map.erase(it);

		// ����
		m_conv_creator.ReleaseID(conv);

		// ֪ͨ�Ͽ�����
		m_rudp_interface->HandleRudpDisconnect(receiver);
	}

public:
	// ������Ϣ�¼�
	void HandleRudpMessage(CarpRudpReceiverPtr receiver, CARP_MESSAGE_SIZE message_size, CARP_MESSAGE_ID message_id, CARP_MESSAGE_RPCID message_rpcid, void* memory) override
	{
		// ֪ͨ������Ϣ��
		m_rudp_interface->HandleRudpMessage(receiver, message_size, message_id, message_rpcid, memory);
		// �ͷ��ڴ�
		if (memory) free(memory);
	}

private:
	// ����ͻ������Ӷ���
	std::unordered_map<uint32_t, CarpRudpReceiverPtr> m_outer_map;	// container outer
	CarpSafeIDCreator<uint32_t> m_conv_creator; // conv������
	CarpRudpInterface* m_rudp_interface = nullptr;
	CarpSchedule* m_schedule = nullptr;

private:
	// ���������һ��ʱ����ͻ��˷���������
	void ServerSendHeartbeat(const asio::error_code& ec, int interval) override
	{
		const auto cur_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
		
		// ���������
		std::set<uint32_t> remove_map;
		for (auto& pair : m_outer_map)
		{
			if (pair.second->CheckHeartbeat(cur_time, interval)) continue;
			remove_map.insert(pair.first);
		}

		// ����������ʱ
		for (const auto& conv : remove_map)
			HandleOuterDisconnected(conv);
		
		// �����пͻ��˷���������
		const HeartbeatMessage msg;
		const auto end = m_outer_map.end();
		for (auto it = m_outer_map.begin(); it != end; ++it)
			it->second->Send(msg);

		if (!m_heartbeat_timer) return;
		m_heartbeat_timer->expires_after(std::chrono::seconds(interval));
		m_heartbeat_timer->async_wait(std::bind(&CarpRudpServer::ServerSendHeartbeat, this->shared_from_this(),
		                                        std::placeholders::_1, interval));
	}
	// ��������ʱ��
	AsioTimerPtr m_heartbeat_timer;

	// ���������һ��ʱ����ͻ��˷���������
	void ServerUpdateKcp(const asio::error_code& ec, int interval) override
	{
		const auto cur_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;

		for (auto& pair : m_outer_map)
			pair.second->UpdateKcp(cur_time_ms);

		if (!m_kcp_timer) return;
		m_kcp_timer->expires_after(std::chrono::milliseconds(interval));
		m_kcp_timer->async_wait(std::bind(&CarpRudpServer::ServerUpdateKcp, this->shared_from_this(),
			std::placeholders::_1, interval));
	}
	// ��������ʱ��
	AsioTimerPtr m_kcp_timer;
};

#endif