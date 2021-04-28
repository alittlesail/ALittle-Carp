#ifndef CARP_CONNECT_CLIENT_INCLUDED
#define CARP_CONNECT_CLIENT_INCLUDED

#include <memory>
#include <vector>
#include <asio.hpp>

typedef std::shared_ptr<asio::ip::tcp::socket> CarpSocketPtr;

// MESSAGE_HEAD_SIZE ��ʾ������Ϣͷ�Ĵ�С
// MESSAGE_BODY_SIZE ��ʾ��Ϣ�����Ϣ���С
template <size_t MESSAGE_HEAD_SIZE, typename MESSAGE_BODY_SIZE>
struct CarpMessageHeadTemplate
{
	static size_t GetHeadSize() { return MESSAGE_HEAD_SIZE; }
	static MESSAGE_BODY_SIZE GetBodySize(std::vector<char>& memory) { return *reinterpret_cast<MESSAGE_BODY_SIZE*>(memory.data()); }
};

template <typename H>
class CarpConnectClientTemplate : public std::enable_shared_from_this<CarpConnectClientTemplate<H>>
{
public:
	CarpConnectClientTemplate()
	{
		m_message_head.resize(H::GetHeadSize());
	}
	~CarpConnectClientTemplate()
	{
		Close();
		// �ͷ��ڴ�
		if (m_memory) { free(m_memory); m_memory = nullptr; }
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
		m_socket = std::make_shared<asio::ip::tcp::socket>(*io_service);
		// ����һ��Ŀ������������ӵ�
		asio::error_code ec;
		const asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string(ip, ec), port);
		if (ec)
		{
			io_service->post(std::bind(&CarpConnectClientTemplate<H>::HandleAsyncConnect, this->shared_from_this(), ec));
			return;
		}

		// ���沢��ʼ��
		m_ip = ip;
		m_port = port;

		// ��ʼ�첽����
		m_socket->async_connect(ep, std::bind(&CarpConnectClientTemplate<H>::HandleAsyncConnect, this->shared_from_this(), std::placeholders::_1));
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

		// �ͷ�socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
			// ���ﲻҪ��m_socket����Ϊ��ָ�룬�����崻���
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
		asio::async_read(*m_socket, asio::buffer(m_message_head.data(), m_message_head.size())
			, std::bind(&CarpConnectClientTemplate<H>::HandleReadHead, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadHead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// �ͷ��ڴ�
			if (m_memory) { free(m_memory); m_memory = nullptr; }
			ExecuteDisconnectCallback();
			return;
		}

		// ��ȡЭ���С
		const auto message_size = H::GetBodySize(m_message_head);

		// �����ڴ�
		if (m_memory) { free(m_memory); m_memory = nullptr; }
		m_memory = malloc(message_size + m_message_head.size());
		auto* const body_memory = static_cast<char*>(m_memory);

		// Э��ͷ���Ƶ��ڴ�
		memcpy(body_memory, m_message_head.data(), m_message_head.size());

		// ���û��Э�����ʾ��ȡ���
		if (message_size == 0)
		{
			ReadComplete();
			NextReadHead();
			return;
		}

		// ��ʼ��ȡЭ����
		asio::async_read(*m_socket, asio::buffer(static_cast<char*>(m_memory) + m_message_head.size(), message_size)
			, std::bind(&CarpConnectClientTemplate<H>::HandleReadBody, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadBody(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// �ͷ��ڴ�
			if (m_memory) { free(m_memory); m_memory = nullptr; }
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
		const auto message_size = H::GetBodySize(m_message_head);
		// ���͸�����ϵͳ
		if (m_message_func)
			m_message_func(m_memory, static_cast<int>(message_size + m_message_head.size()));
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
	std::vector<char> m_message_head{};
	// ����Э����
	void* m_memory = nullptr;

	//������Ϣ������/////////////////////////////////////////////////////////////////////////////////
public:
	// �����ͣ������memory��CarpConnectClient�ӹ��ͷţ��ⲿ��Ҫ�ͷ�
	void SendPocket(void* memory, const int memory_size)
	{
		// �����ڴ�ṹ
		PocketInfo info;
		info.memory_size = memory_size;
		info.memory = memory;

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
		asio::async_write(*m_socket, asio::buffer(info.memory, info.memory_size)
			, std::bind(&CarpConnectClientTemplate<H>::HandleSend, this->shared_from_this()
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
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; };
	std::list<PocketInfo> m_pocket_list;  // �����͵����ݰ��б�

	bool m_executing = false;	// is in sending

private:
	std::function<void()> m_failed_func;
	std::function<void()> m_succeed_func;
	std::function<void()> m_disconnected_func;
	std::function<void(void*, int)> m_message_func;
};

typedef unsigned int CARP_MESSAGE_SIZE;
typedef int CARP_MESSAGE_ID;
typedef int CARP_MESSAGE_RPCID;

using CarpMessageHead = CarpMessageHeadTemplate<(sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID) + sizeof(CARP_MESSAGE_RPCID)), CARP_MESSAGE_SIZE>;
using CarpConnectClient = CarpConnectClientTemplate<CarpMessageHead>;

typedef std::shared_ptr<CarpConnectClient> CarpConnectClientPtr;


#endif