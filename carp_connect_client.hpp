#ifndef CARP_CONNECT_CLIENT_INCLUDED
#define CARP_CONNECT_CLIENT_INCLUDED (1)

#include <memory>
#include <asio.hpp>

class CarpConnectClient;
typedef std::shared_ptr<CarpConnectClient> CarpConnectClientPtr;
typedef std::shared_ptr<asio::ip::tcp::socket> CarpSocketPtr;

typedef unsigned int CARP_MESSAGE_SIZE;
typedef int CARP_MESSAGE_ID;
typedef int CARP_MESSAGE_RPCID;

#define CARP_PROTOCOL_HEAD_SIZE (sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID) + sizeof(CARP_MESSAGE_RPCID))

class CarpConnectClient : public std::enable_shared_from_this<CarpConnectClient>
{
public:
	CarpConnectClient() { }
	~CarpConnectClient()
	{
		Close();
		// �ͷ��ڴ�
		if (m_memory) { free(m_memory); m_memory = 0; }
	}

	//���Ӳ���/////////////////////////////////////////////////////////////////////////////////
public:
	/* �첽����Ŀ�������
	 * @param route_id: ·��ID��ȫ��Ψһ
	 * @param ip: Ŀ�������IP
	 * @param port: Ŀ��������˿�
	 */
	void Connect( const std::string& ip, unsigned int port, asio::io_service* io_service
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
		m_socket = CarpSocketPtr(new asio::ip::tcp::socket(*io_service));
		// ����һ��Ŀ������������ӵ�
		asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string(ip), port);

		// ���沢��ʼ��
		m_ip = ip;
		m_port = port;

		// ��ʼ�첽����
		m_socket->async_connect(ep, std::bind(&CarpConnectClient::HandleAsyncConnect, this->shared_from_this(), std::placeholders::_1));
	}

	// �ж��Ƿ��Ѿ�����
	bool IsConnected() const { return m_is_connected; }

	// �Ƿ���������
	bool IsConnecting() const { return m_is_connecting; }

	// �ر�����
	void Close()
	{
		// �ͷŴ����͵���Ϣ��
		auto end = m_pocket_list.end();
		for (auto it = m_pocket_list.begin(); it != end; ++it)
			free(it->memory);
		m_pocket_list.clear();

		// ���Ϊ�������ڷ���
		m_excuting = false;
		// ���Ϊ������������
		m_is_connecting = false;
		// ���Ϊδ����
		m_is_connected = false;

		// �ͷ�socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
		}

		// ���ﲻҪ�����ͷ�m_memory������asio������
		// �ŵ��������������ͷ�
		// if (m_memory) { free(m_memory); m_memory = 0; }
	}

private:
	std::string m_ip;			// Ŀ���������IP
	unsigned int m_port = 0;		// Ŀ��������˿�

public:
	// ��ȡĿ�������IP�Ͷ˿�
	const std::string& GetIP() const { return m_ip; }
	unsigned int GetPort() const { return m_port; }

private:
	// �첽����
	void HandleAsyncConnect(const asio::error_code& ec)
	{
		// ���Ϊ������������
		m_is_connecting = false;
		// ���Ϊ�������ڷ���
		m_excuting = false;
		// ���Ϊ������
		m_is_connected = false;
		if (ec)
		{
			// �����־����ӡ����Ϊ�����̫�࣬�ֲ���Ҫ
			// ALITTLE_SYSTEM(u8"ConnectClient ����ʧ��: " << SUTF8(asio::system_error(ec).what()) << ", ip:" << m_ip << ", port:" << m_port);

			// ��������ʧ��
			HandleConnectFailed();
			return;
		}
		// ���� no delay
		m_socket->set_option(asio::ip::tcp::no_delay(true));

		// ���Ϊ������
		m_is_connected = true;

		// ��ʼ������Ϣ��
		NextReadHead();

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
	// ��ȡЭ��
	void NextReadHead()
	{
		// ����Ѿ��ͷ��˾�ֱ�ӷ���
		if (!m_socket) return;

		// ��ʼ����Э��ͷ
		asio::async_read(*m_socket, asio::buffer(m_message_head, sizeof(m_message_head))
			, std::bind(&CarpConnectClient::HandleReadHead, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadHead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// �ͷ��ڴ�
			if (m_memory) { free(m_memory); m_memory = 0; }
			ExecuteDisconnectCallback();
			return;
		}

		// ��ȡЭ���С
		CARP_MESSAGE_SIZE message_size = *(CARP_MESSAGE_SIZE*)m_message_head;

		// �����ڴ�
		if (m_memory) { free(m_memory); m_memory = 0; }
		m_memory = malloc(message_size + CARP_PROTOCOL_HEAD_SIZE);
		char* body_memory = (char*)m_memory;

		// Э��ͷ���Ƶ��ڴ�
		memcpy(body_memory, m_message_head, sizeof(m_message_head));

		// ���û��Э�����ʾ��ȡ���
		if (message_size == 0)
		{
			ReadComplete();
			NextReadHead();
			return;
		}

		// ��ʼ��ȡЭ����
		asio::async_read(*m_socket, asio::buffer((char*)m_memory + CARP_PROTOCOL_HEAD_SIZE, message_size)
			, std::bind(&CarpConnectClient::HandleReadBody, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadBody(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// �ͷ��ڴ�
			if (m_memory) { free(m_memory); m_memory = 0; }
			// ֪ͨ�Ͽ�����
			ExecuteDisconnectCallback();
			return;
		}

		// ����Э��
		ReadComplete();
		// ������ȡ��һ����Ϣ��
		NextReadHead();
	}

private:
	// ����Э��
	void ReadComplete()
	{

		// ��ȡЭ���С
		CARP_MESSAGE_SIZE message_size = *(CARP_MESSAGE_SIZE*)m_message_head;
		// ���͸�����ϵͳ
		if (m_message_func)
			m_message_func(m_memory, message_size + CARP_PROTOCOL_HEAD_SIZE);
		else
			free(m_memory);
		// �ڴ��Ѿ��ƽ���ȥ��HandleMessage�Ḻ���ͷ�
		// ������0
		m_memory = nullptr;
	}

public:
	CarpSocketPtr m_socket;					// socket

private:
	// ����Э��ͷ
	char m_message_head[CARP_PROTOCOL_HEAD_SIZE]{};
	// ����Э����
	void* m_memory = nullptr;

	//������Ϣ������/////////////////////////////////////////////////////////////////////////////////
public:
	// �����ͣ������memory��CarpConnectClient�ӹ��ͷţ��ⲿ��Ҫ�ͷ�
	void SendPocket(void* memory, int memory_size)
	{
		// �����ڴ�ṹ
		PocketInfo info = {0};
		info.memory_size = memory_size;
		info.memory = memory;

		// ��ӵ��������б�
		m_pocket_list.push_back(info);
		// ����Ѿ����ڷ����ˣ���ô��ֱ�ӷ���
		if (m_excuting) return;
		// ������ڷ���
		m_excuting = true;
		// ����һ����Ϣ��
		NextSend();
	}
	
	void NextSend()
	{
		// ������б��ǿյģ�����socket�Ѿ��ر��ˣ�ֱ�ӷ���
		if (m_pocket_list.empty() || !m_socket)
		{
			m_excuting = false;
			return;
		}

		// ��ȡһ���ṹ��
		PocketInfo info = m_pocket_list.front();
		m_pocket_list.pop_front();

		// ����
		asio::async_write(*m_socket, asio::buffer(info.memory, info.memory_size)
			, std::bind(&CarpConnectClient::HandleSend, this->shared_from_this()
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
	struct PocketInfo { int memory_size; void* memory; };
	std::list<PocketInfo> m_pocket_list;  // �����͵����ݰ��б�

	bool m_excuting = false;	// is in sending

private:
	std::function<void()> m_failed_func;
	std::function<void()> m_succeed_func;
	std::function<void()> m_disconnected_func;
	std::function<void(void*, int)> m_message_func;
};

#endif