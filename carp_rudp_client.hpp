#ifndef CARP_RUDP_CLIENT_INCLUDED
#define CARP_RUDP_CLIENT_INCLUDED

#include <memory>
#include <asio.hpp>
#include "kcp/ikcp.h"

class CarpRudpClient;
typedef std::shared_ptr<CarpRudpClient> CarpRudpClientPtr;
typedef std::shared_ptr<asio::ip::udp::socket> CarpUdpSocketPtr;
typedef asio::basic_waitable_timer<std::chrono::system_clock> AsioTimer;
typedef std::shared_ptr<AsioTimer> AsioTimerPtr;


typedef unsigned int CARP_MESSAGE_SIZE;
typedef int CARP_MESSAGE_ID;
typedef int CARP_MESSAGE_RPCID;

#define CARP_PROTOCOL_HEAD_SIZE (sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID) + sizeof(CARP_MESSAGE_RPCID))
#define CARP_UDP_BUFFER_SIZE 10240 // ����һ�����ֵ���϶��ᳬ��һ��udp

class CarpRudpClient : public std::enable_shared_from_this<CarpRudpClient>
{
public:
	~CarpRudpClient()
	{
		Close();
	}

	//���Ӳ���/////////////////////////////////////////////////////////////////////////////////
public:
	/* �첽����Ŀ�������
	 * @param conv: ���ڱ�ʶͨѶ˫����ȷ���ͷ���������һ��
	 * @param ip: Ŀ�������IP
	 * @param port: Ŀ��������˿�
	 */
	void Connect(uint32_t conv, const std::string& ip, unsigned int port, asio::io_service* io_service
		, std::function<void()> failed_func, std::function<void()> succeed_func, std::function<void()> disconnected_func
		, std::function<void(void*, int)> message_func)
	{
		// ����Ƿ���������
		if (IsConnected() || IsConnecting()) return;

		m_failed_func = failed_func;
		m_succeed_func = succeed_func;
		m_disconnected_func = disconnected_func;
		m_message_func = message_func;

		// ���Ϊ��������
		m_is_connecting = true;
		// ����һ��socket����
		m_socket = std::make_shared<asio::ip::udp::socket>(*io_service);
		// ����һ��Ŀ������������ӵ�
		m_endpoint = asio::ip::udp::endpoint(asio::ip::address_v4::from_string(ip), port);
		// ����kcp
		m_kcp = ikcp_create(conv, this);
		// ����Ϊ��ģʽ
		m_kcp->stream = 1;
		// ���÷��ͺ���
		m_kcp->output = UdpOutput;
		// ����Ϊ����ģʽ
		ikcp_nodelay(m_kcp, 1, 10, 2, 1);

		// ������ʱ��
		const int heartbeat = 20;
		m_kcp_timer = std::make_shared<AsioTimer>(*io_service, std::chrono::milliseconds(heartbeat));
		m_kcp_timer->async_wait(std::bind(&CarpRudpClient::UpdateKcp, this->shared_from_this(), std::placeholders::_1, heartbeat));

		// ���沢��ʼ��
		m_ip = ip;
		m_port = port;

		// ��socket
		asio::error_code ec;
		m_socket->open(asio::ip::udp::v4(), ec);
		HandleAsyncConnect(ec);
	}

	// �ж��Ƿ��Ѿ�����
	bool IsConnected() const { return m_is_connected; }

	// �Ƿ���������
	bool IsConnecting() const { return m_is_connecting; }

	// �ر�����
	void Close()
	{
		// �ͷŴ����͵���Ϣ��
		for (auto& info : m_pocket_list) free(info.memory);
		m_pocket_list.clear();

		// ���Ϊ�������ڷ���
		m_executing = false;
		// ���Ϊ������������
		m_is_connecting = false;
		// ���Ϊδ����
		m_is_connected = false;
		// ����ƫ��
		m_kcp_data_size = 0;

		// �ͷ�socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
			// ���ﲻҪ��m_socket����Ϊ��ָ�룬�����崻���
		}

		// �ͷŶ�ʱ��
		if (m_kcp_timer)
		{
			m_kcp_timer->cancel();
			m_kcp_timer = AsioTimerPtr();
		}

		// �ͷ�kcp
		if (m_kcp)
		{
			ikcp_release(m_kcp);
			m_kcp = nullptr;
		}

		// ���ﲻҪ�����ͷ�m_memory������asio������
		// �ŵ��������������ͷ�
		// if (m_memory) { free(m_memory); m_memory = 0; }
	}

private:
	std::string m_ip;			// Ŀ���������IP
	unsigned int m_port = 0;	// Ŀ��������˿�

public:
	// ��ȡĿ�������IP�Ͷ˿�
	const std::string& GetIP() const { return m_ip; }
	unsigned int GetPort() const { return m_port; }

private:
	// �첽����
	void HandleAsyncConnect(const asio::error_code& ec)
	{
		// ����ƫ��
		m_kcp_data_size = 0;
		// ���Ϊ������������
		m_is_connecting = false;
		// ���Ϊ�������ڷ���
		m_executing = false;
		// ���Ϊ������
		m_is_connected = false;
		if (ec)
		{
			// ��������ʧ��
			HandleConnectFailed();
			return;
		}

		// ���Ϊ������
		m_is_connected = true;

		// ��ʼ������Ϣ��
		NextRead();

		// �������ӳɹ�
		HandleConnectSucceed();
	}

	bool m_is_connecting = false;
	bool m_is_connected = false;

private:
	// ����Ͽ�����
	void ExecuteDisconnectCallback()
	{
		// ��������������ӣ�����δ���ӳɹ�����ô�϶�����������Close�����
		// ���ʱ�����ڶϿ����ӣ����Բ�Ҫ����HandleDisconnected()
		const bool close_by_self = m_is_connecting == false && m_is_connected == false;

		// �رգ��ڲ����m_socket����Ϊ��ָ��
		// ���Լ�ʹͬʱ��Ϊ����ʧ�ܻ��߷���ʧ�ܶ�������ExecuteDisconnectCallbackҲ�����ε���HandleDisconnected
		Close();

		// ��������Լ��رյģ���ô�͵��ûص�
		if (close_by_self == false) HandleDisconnected();
	}
	// ��������ʧ��
	void HandleConnectFailed() const
	{
		if (m_failed_func) m_failed_func();
	}
	// �������ӳɹ�
	void HandleConnectSucceed() const
	{
		if (m_succeed_func) m_succeed_func();
	}
	// ����Ͽ�����
	void HandleDisconnected() const
	{
		if (m_disconnected_func) m_disconnected_func();
	}

	//��ȡ��Ϣ������/////////////////////////////////////////////////////////////////////////////////
public:
	// ��ȡudp��
	void NextRead()
	{
		// ����Ѿ��ͷ��˾�ֱ�ӷ���
		if (!m_socket) return;

		// ��ʼ����Э��ͷ
		m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_endpoint
			, std::bind(&CarpRudpClient::HandleRead, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleRead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			ExecuteDisconnectCallback();
			return;
		}

		ikcp_input(m_kcp, m_udp_buffer, actual_size);
		NextRead();

		// �������п��Խ��յ�����
		while (true)
		{
			const int peek_size = ikcp_peeksize(m_kcp);
			if (peek_size <= 0) break;

			if (static_cast<int>(m_kcp_buffer.size() - m_kcp_data_size) < peek_size)
				m_kcp_buffer.resize(peek_size + m_kcp_data_size);

			const auto real_len = ikcp_recv(m_kcp, m_kcp_buffer.data() + m_kcp_data_size, static_cast<int>(m_kcp_buffer.size() - m_kcp_data_size));
			if (real_len == 0) break;

			m_kcp_data_size += real_len;	
		}

		// ��ʼ��������
		size_t kcp_data_offset = 0;
		while (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE <= m_kcp_data_size)
		{
			// ��ȡЭ���С
			const auto message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(m_kcp_buffer.data() + kcp_data_offset);

			// ���Э��������ݲ��㣬��ô������
			if (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE + message_size > m_kcp_data_size) break;
			
			// �����ڴ�
			void* memory = malloc(message_size + CARP_PROTOCOL_HEAD_SIZE);
			memcpy(memory, m_kcp_buffer.data() + kcp_data_offset, message_size + CARP_PROTOCOL_HEAD_SIZE);

			// ��ȡ���
			// ���͸�����ϵͳ
			if (m_message_func)
				m_message_func(memory, message_size + CARP_PROTOCOL_HEAD_SIZE);
			else
				free(memory);
			
			// ƫ�������
			kcp_data_offset += CARP_PROTOCOL_HEAD_SIZE + message_size;
		}

		// �ƶ��ڴ�
		if (kcp_data_offset > 0)
		{
			memmove(m_kcp_buffer.data(), m_kcp_buffer.data() + kcp_data_offset, m_kcp_data_size - kcp_data_offset);
			m_kcp_data_size -= kcp_data_offset;
		}
	}
	
private:
	ikcpcb* m_kcp = nullptr;
	// ��������ʱ��
	AsioTimerPtr m_kcp_timer;
	
	// ���������һ��ʱ����ͻ��˷���������
	void UpdateKcp(const asio::error_code& ec, int interval)
	{
		const auto cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		ikcp_update(m_kcp, cur_time);
		
		if (!m_kcp_timer) return;
		m_kcp_timer->expires_after(std::chrono::milliseconds(interval));
		m_kcp_timer->async_wait(std::bind(&CarpRudpClient::UpdateKcp, this->shared_from_this(),
			std::placeholders::_1, interval));
	}
	
public:
	CarpUdpSocketPtr m_socket;					// socket
	asio::ip::udp::endpoint m_endpoint;			// ����Ŀ��

	char m_udp_buffer[CARP_UDP_BUFFER_SIZE] = {};	// ���ڽ���asio��udp���ݻ�����
	std::vector<char> m_kcp_buffer;					// ���ڽ���kcp���ݵ�
	size_t m_kcp_data_size = 0;

	//������Ϣ������/////////////////////////////////////////////////////////////////////////////////
public:
	// �����ͣ������memory��CarpRudpClient�ӹ��ͷţ��ⲿ��Ҫ�ͷ�
	void SendPocket(void* memory, const int memory_size)
	{
		// ����ÿ�η��͵�����ֽ���
		const int max_size = m_kcp->mss * m_kcp->rcv_wnd;

		// ��һ����������ֽ��������и�
		const char* body = static_cast<const char*>(memory);
		int offset = 0;
		while (offset < memory_size)
		{
			int send = memory_size - offset;
			if (send > max_size) send = max_size;
			ikcp_send(m_kcp, body + offset, send);
			offset += send;
		}

		free(memory);
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
		m_socket->async_send_to(asio::buffer(info.memory, info.memory_size), m_endpoint
			, std::bind(&CarpRudpClient::HandleSend, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, info.memory));
	}
	void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory)
	{
		// �ͷ��ڴ�
		if (memory) free(memory);

		// ������
		if (ec)
		{
			// ���ﲻ֪ͨ�Ͽ����ӣ��ȴ������ǲ���֪ͨ�Ͽ�
			ExecuteDisconnectCallback();
			return;
		}

		// ������һ����
		NextSend();
	}

private:
	static int UdpOutput(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		auto* self = static_cast<CarpRudpClient*>(user);
		// ����
		void* memory = malloc(len);
		memcpy(memory, buf, len);

		// �����ڴ�ṹ
		PocketInfo info;
		info.memory_size = len;
		info.memory = memory;

		// ��ӵ��������б�
		self->m_pocket_list.push_back(info);
		// ����Ѿ����ڷ����ˣ���ô��ֱ�ӷ���
		if (self->m_executing) return 0;
		// ������ڷ���
		self->m_executing = true;
		// ����һ����Ϣ��
		self->NextSend();
		return 0;
	}

private:
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; };
	std::list<PocketInfo> m_pocket_list;  // �����͵����ݰ��б�

	bool m_executing = false;	// is in sending

private:
	std::function<void()> m_failed_func;
	std::function<void()> m_succeed_func;
	std::function<void()> m_disconnected_func;
	std::function<void(void*, int)> m_message_func;
};

#endif