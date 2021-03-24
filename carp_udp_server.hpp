#ifndef CARP_UDP_SERVER_INCLUDED
#define CARP_UDP_SERVER_INCLUDED

#include <memory>
#include <string>
#include <asio.hpp>

#include "carp_log.hpp"

typedef std::shared_ptr<asio::ip::udp::socket> CarpUSocketPtr;
class CarpUdpServer;
typedef std::shared_ptr<CarpUdpServer> CarpUdpServerPtr;
typedef std::weak_ptr<CarpUdpServer> CarpUdpServerWeakPtr;

class CarpUdpServer : public std::enable_shared_from_this<CarpUdpServer>
{
public:
	CarpUdpServer(asio::io_service& io_service) : m_io_service(io_service) {}
	~CarpUdpServer() { Close(); }

public:
	/* start server
	 * @param local_ip: local ip dress
	 * @param local_port: port
	 * @param buffer_size: size of receive buffer
	 */
	bool Start(const std::string& local_ip, unsigned int local_port, unsigned int buffer_size = 10240)
	{
		// check is already started
		if (m_socket)
		{
			CARP_ERROR("udp system binded(ip: " << m_ip << ", port:" << m_port << ")");
			return false;
		}

		m_socket = std::make_shared<asio::ip::udp::socket>(m_io_service);
		asio::ip::udp::endpoint local_add = asio::ip::udp::endpoint(asio::ip::address_v4::from_string(local_ip), local_port);

		asio::error_code ec;
		m_socket->open(local_add.protocol(), ec);
		if (ec)
		{
			CARP_ERROR("udp server socket open failed: " << ec.value());
			return false;
		}
		m_socket->bind(local_add, ec);
		if (ec)
		{
			CARP_ERROR("udp server socket bind failed: " << ec.value());
			return false;
		}
		m_ip = local_ip;
		m_port = local_port;
		m_buffer.resize(buffer_size);

		CARP_INFO("udp server start succeed:" << m_ip << ", " << m_port);
		NextRead();
		return true;
	}

	/* close server
	 */
	void Close()
	{
		if (!m_socket) return;

		// close accept
		asio::error_code ec;
		m_socket->close(ec);

		// release acceptor
		m_socket = CarpUSocketPtr();

		CARP_INFO("udp System stop succeed:" << m_ip << ", " << m_port);
	}

	//register callback///////////////////////////////////////////////////////////////////////////
public:
	/* handler function define
	 */
	struct HandleInfo
	{
		CarpUdpServerPtr sender;
		asio::ip::udp::endpoint end_point;
		const char* memory;
		size_t memory_size;
	};
	// release memory ,if set false then UDPSystem does not release info's memory, if true then release
	typedef std::function<void(HandleInfo& info)> UdpHandle;

	/* register get handler
	 * @param func: handle callback
	 */
	void RegisterUdpHandle(UdpHandle func) { m_udp_handle = func; }

private:
	/* handle
	 */
	void HandleUdpHandle(const asio::ip::udp::endpoint& end_point, const char* memory, size_t memory_size)
	{
		if (m_udp_handle)
		{
			HandleInfo info;
			info.sender = this->shared_from_this();
			info.end_point = end_point;
			info.memory = memory;
			info.memory_size = memory_size;
			m_udp_handle(info);
		}

		free((void*)memory);
	}

	UdpHandle m_udp_handle; // udp handle

public:
	/* send data
	 */
	void Send(void* memory, size_t size, const asio::ip::udp::endpoint& end_point)
	{
		m_socket->async_send_to(asio::buffer(memory, size), end_point
			, std::bind(&CarpUdpServer::HandleSend, this->shared_from_this(), memory, std::placeholders::_1, std::placeholders::_2));
	}
	void Send(const std::string& content, const asio::ip::udp::endpoint& end_point)
	{
		void* memory = malloc(content.size());
		memcpy(memory, content.c_str(), content.size());
		Send(memory, content.size(), end_point);
	}
	size_t GetBufferSize() const { return m_buffer.size(); }

private:
	/* handle send
	 */
	void HandleSend(void* memory, const asio::error_code& ec, std::size_t actual_size)
	{
		free(memory);
		if (ec) CARP_ERROR("HandleSend error:" << ec.value());
	}

	/* next read
	 */
	void NextRead()
	{
		if (!m_socket) return;
		m_socket->async_receive_from(asio::buffer(&(m_buffer[0]), m_buffer.size()), m_receiver
			, std::bind(&CarpUdpServer::HandleNextRead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	/* handle next read
	 */
	void HandleNextRead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// 不打印日志
			// if (m_usocket)
			//	ALITTLE_ERROR("UDPSystem::HandleNextRead failed: " << STRING_2_UTF8(asio::system_error(ec).what()));
			NextRead();
			return;
		}

		if (actual_size >= m_buffer.size())
		{
			CARP_ERROR("udp read size(" << actual_size << ") >= buffer size(" << m_buffer.size() << ")");
			return;
		}

		// create memory from pool
		char* memory = (char*)malloc(actual_size + 1);
		// copy message head to memory
		memcpy(memory, &(m_buffer[0]), actual_size);
		// set end for string
		memory[actual_size] = 0;

		// handle message
		std::function<void()> func = std::bind(&CarpUdpServer::HandleUdpHandle, this->shared_from_this(), m_receiver, memory, actual_size);

		m_io_service.post(func);

		// next read
		NextRead();
	}

private:
	CarpUSocketPtr m_socket;
	asio::io_service& m_io_service;

private:
	std::string m_ip;
	unsigned int m_port = 0;

private:
	asio::ip::udp::endpoint m_receiver;
	std::vector<char> m_buffer;
};

#endif