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
#define CARP_UDP_CLIENT_BUFFER_SIZE 10240 // 设置一个大的值，肯定会超过一个udp

class CarpRudpClient : public std::enable_shared_from_this<CarpRudpClient>
{
public:
	~CarpRudpClient()
	{
		Close();
	}

	//连接部分/////////////////////////////////////////////////////////////////////////////////
public:
	/* 异步连接目标服务器
	 * @param ip: 目标服务器IP
	 * @param port: 目标服务器端口
	 */
	void Connect(const std::string& ip, unsigned int port, asio::io_service* io_service
		, std::function<void()> failed_func, std::function<void()> succeed_func, std::function<void()> disconnected_func
		, std::function<void(void*, int)> message_func)
	{
		// 检查是否正在连接
		if (IsConnected() || IsConnecting()) return;

		if (ip.empty())
		{
			io_service->post(std::bind(&CarpRudpClient::HandleAsyncConnect, this->shared_from_this(), false));
			return;
		}

		m_failed_func = failed_func;
		m_succeed_func = succeed_func;
		m_disconnected_func = disconnected_func;
		m_message_func = message_func;

		// 标记为正在连接
		m_is_connecting = true;
		// 创建一个socket对象
		m_socket = std::make_shared<asio::ip::udp::socket>(*io_service);
		// 创建一个目标服务器的连接点
		m_endpoint = asio::ip::udp::endpoint(asio::ip::address_v4::from_string(ip), port);
		
		// 保存并初始化
		m_ip = ip;
		m_port = port;

		// 打开socket
		asio::error_code ec;
		m_socket->open(asio::ip::udp::v4(), ec);
		if (ec)
		{
			io_service->post(std::bind(&CarpRudpClient::HandleAsyncConnect, this->shared_from_this(), false));
			return;
		}

		// 发起连接
		SendConnect(io_service);
	}

	// 判断是否已经连接
	bool IsConnected() const { return m_is_connected; }

	// 是否正在连接
	bool IsConnecting() const { return m_is_connecting; }

	// 关闭连接
	void Close()
	{
		// 如果已连接成功，那么就发送关闭连接的协议
		if (m_is_connected && m_socket && m_kcp)
		{
			const auto size = sizeof(int) + sizeof(uint32_t);
			char buffer[size] = { 0 };
			auto n_session = -m_session; // 这里使用负数的session来作为关闭操作
			memcpy(buffer, &n_session, sizeof(int));
			memcpy(buffer + sizeof(int), &m_kcp->conv, sizeof(uint32_t));
			m_socket->send_to(asio::buffer(buffer, size), m_endpoint);
		}
		
		// 释放带发送的消息包
		for (auto& info : m_pocket_list) free(info.memory);
		m_pocket_list.clear();

		// 标记为不是正在发包
		m_executing = false;
		// 标记为不是正在连接
		m_is_connecting = false;
		// 标记为未连接
		m_is_connected = false;
		// 重置偏移
		m_kcp_data_size = 0;

		// 释放socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
			// 这里不要把m_socket的置为空指针，否则会宕机。
		}

		// 释放定时器
		if (m_kcp_timer)
		{
			m_kcp_timer->cancel();
			m_kcp_timer = AsioTimerPtr();
		}
		if (m_connect_timer)
		{
			m_connect_timer->cancel();
			m_connect_timer = AsioTimerPtr();
		}

		// 释放kcp
		if (m_kcp)
		{
			ikcp_release(m_kcp);
			m_kcp = nullptr;
		}
	}

private:
	std::string m_ip;			// 目标服务器的IP
	unsigned int m_port = 0;	// 目标服务器端口
	int m_session = 0;
	// 心跳包定时器
	AsioTimerPtr m_connect_timer;	// 连接定时器

public:
	// 获取目标服务器IP和端口
	const std::string& GetIP() const { return m_ip; }
	unsigned int GetPort() const { return m_port; }

private:
	// 发起连接请求
	void SendConnect(asio::io_service* io_service)
	{
		CARP_SYSTEM("asd111");
		const auto size = sizeof(int) + sizeof(uint32_t);
		void* memory = malloc(size);
		// 这里讲session设置为0作为请求连接的操作
		memset(memory, 0, size);
		
		m_socket->async_send_to(asio::buffer(memory, size), m_endpoint
			, std::bind(&CarpRudpClient::HandleSendConnect, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, memory));

		// 等待接收连接应答包
		m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_endpoint
			, std::bind(&CarpRudpClient::HandleReceiveConnect, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, io_service));

		// 创建定时器
		m_connect_timer = std::make_shared<AsioTimer>(*io_service, std::chrono::seconds(5));
		m_connect_timer->async_wait(std::bind(&CarpRudpClient::HandleAsyncConnect, this->shared_from_this(), false));
	}
	void HandleSendConnect(const asio::error_code& ec, std::size_t actual_size, void* memory)
	{
		free(memory);
	}
	void HandleReceiveConnect(const asio::error_code& ec, std::size_t actual_size, asio::io_service* io_service)
	{
		CARP_SYSTEM("asd222");
		if (ec)
		{
			HandleAsyncConnect(false);
			return;
		}

		// 如果不是指定大小，那么就直接丢弃掉
		const auto size = sizeof(int) + sizeof(uint32_t);
		if (actual_size != size)
		{
			// 继续收包
			m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_endpoint
				, std::bind(&CarpRudpClient::HandleReceiveConnect, this->shared_from_this()
					, std::placeholders::_1, std::placeholders::_2, io_service));
			return;
		}

		// 取消定时器
		if (m_connect_timer)
		{
			asio::error_code cancel_ec;
			m_connect_timer->cancel(cancel_ec);
			m_connect_timer = AsioTimerPtr();
		}

		// 取session
		m_session = *reinterpret_cast<int*>(m_udp_buffer);
		// 取conv
		const auto conv = *reinterpret_cast<uint32_t*>(m_udp_buffer + sizeof(int));
		// 检查是否连接成功
		if (m_session <= 0 || conv == 0)
		{
			HandleAsyncConnect(false);
			return;
		}

		// 创建kcp
		m_kcp = ikcp_create(conv, this);
		// 设置为流模式
		m_kcp->stream = 1;
		// 设置发送函数
		m_kcp->output = UdpOutput;
		// 设置为极速模式
		ikcp_nodelay(m_kcp, 1, 10, 2, 1);
		// 腾出四个字节用于保存session
		ikcp_setmtu(m_kcp, static_cast<int>(m_kcp->mtu) - static_cast<int>(sizeof(int)));

		// 创建定时器
		const int heartbeat = 20;
		m_kcp_timer = std::make_shared<AsioTimer>(*io_service, std::chrono::milliseconds(heartbeat));
		m_kcp_timer->async_wait(std::bind(&CarpRudpClient::UpdateKcp, this->shared_from_this(), std::placeholders::_1, heartbeat));

		HandleAsyncConnect(true);
	}
	// 处理连接超时
	void HandleConnectTimeout(const asio::error_code& ec)
	{
		CARP_SYSTEM("asd444:" << ec.value());
		HandleAsyncConnect(false);
	}
	
	// 异步连接
	void HandleAsyncConnect(bool succeed)
	{
		CARP_SYSTEM("asd333:" << (int)succeed);
		// 重置偏移
		m_kcp_data_size = 0;
		// 标记为不是正在连接
		m_is_connecting = false;
		// 标记为不是正在发包
		m_executing = false;
		// 标记为已连接
		m_is_connected = false;
		if (!succeed)
		{
			// 处理连接失败
			HandleConnectFailed();
			return;
		}

		// 标记为已连接
		m_is_connected = true;

		// 开始接受消息包
		NextRead();

		// 处理连接成功
		HandleConnectSucceed();
	}

	bool m_is_connecting = false;
	bool m_is_connected = false;

private:
	// 处理断开连接
	void ExecuteDisconnectCallback()
	{
		// 如果不是正在连接，并且未连接成功，那么肯定是主动调用Close引起的
		// 这个时候不属于断开连接，所以不要调用HandleDisconnected()
		const bool close_by_self = m_is_connecting == false && m_is_connected == false;

		// 关闭，内部会把m_socket设置为空指针
		// 所以即使同时因为接收失败或者发送失败而触发的ExecuteDisconnectCallback也不会多次调用HandleDisconnected
		Close();

		// 如果不是自己关闭的，那么就调用回调
		if (close_by_self == false) HandleDisconnected();
	}
	// 处理连接失败
	void HandleConnectFailed() const
	{
		if (m_failed_func) m_failed_func();
	}
	// 处理连接成功
	void HandleConnectSucceed() const
	{
		if (m_succeed_func) m_succeed_func();
	}
	// 处理断开连接
	void HandleDisconnected() const
	{
		if (m_disconnected_func) m_disconnected_func();
	}

	//读取消息包部分/////////////////////////////////////////////////////////////////////////////////
public:
	// 读取udp包
	void NextRead()
	{
		// 如果已经释放了就直接返回
		if (!m_socket) return;

		// 开始接受协议头
		m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_receiver
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

		// 如果字节数不对，那么就直接丢弃
		if (actual_size <= sizeof(int) + sizeof(uint32_t))
		{
			NextRead();
			return;
		}

		// 取conv
		const auto conv = ikcp_getconv(m_udp_buffer + sizeof(int));
		// 如果conv和当前对不上，直接丢弃
		if (conv != m_kcp->conv)
		{
			NextRead();
			return;
		}
		
		// 更新session
		m_session = *reinterpret_cast<int*>(m_udp_buffer);
		// 如果session小于0，那么说明是服务器主动断开连接
		if (m_session <= 0)
		{
			ExecuteDisconnectCallback();
			return;
		}

		// 把数据喂给kcp
		ikcp_input(m_kcp, m_udp_buffer + sizeof(int), static_cast<int>(actual_size - sizeof(int)));
		// 继续接收下一个数据包
		NextRead();

		// 接收所有可以接收的数据
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

		// 开始遍历处理
		size_t kcp_data_offset = 0;
		while (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE <= m_kcp_data_size)
		{
			// 读取协议大小
			const auto message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(m_kcp_buffer.data() + kcp_data_offset);

			// 如果协议体的数据不足，那么就跳出
			if (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE + message_size > m_kcp_data_size) break;
			
			// 申请内存
			void* memory = malloc(message_size + CARP_PROTOCOL_HEAD_SIZE);
			memcpy(memory, m_kcp_buffer.data() + kcp_data_offset, message_size + CARP_PROTOCOL_HEAD_SIZE);

			// 读取完毕
			// 发送给调度系统
			if (m_message_func)
				m_message_func(memory, message_size + CARP_PROTOCOL_HEAD_SIZE);
			else
				free(memory);
			
			// 偏移向后走
			kcp_data_offset += CARP_PROTOCOL_HEAD_SIZE + message_size;
		}

		// 移动内存
		if (kcp_data_offset > 0)
		{
			memmove(m_kcp_buffer.data(), m_kcp_buffer.data() + kcp_data_offset, m_kcp_data_size - kcp_data_offset);
			m_kcp_data_size -= kcp_data_offset;
		}
	}
	
private:
	ikcpcb* m_kcp = nullptr;
	// 心跳包定时器
	AsioTimerPtr m_kcp_timer;
	
	// 服务器间隔一定时间向客户端发送心跳包
	void UpdateKcp(const asio::error_code& ec, int interval)
	{
		const auto cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		ikcp_update(m_kcp, static_cast<uint32_t>(cur_time));
		
		if (!m_kcp_timer) return;
		m_kcp_timer->expires_after(std::chrono::milliseconds(interval));
		m_kcp_timer->async_wait(std::bind(&CarpRudpClient::UpdateKcp, this->shared_from_this(),
			std::placeholders::_1, interval));
	}
	
public:
	CarpUdpSocketPtr m_socket;					// socket
	asio::ip::udp::endpoint m_endpoint;			// 发送目标
	asio::ip::udp::endpoint m_receiver;			// 接收消息源

	char m_udp_buffer[CARP_UDP_CLIENT_BUFFER_SIZE] = {};	// 用于接收asio的udp数据缓冲区
	std::vector<char> m_kcp_buffer;					// 用于接收kcp数据的
	size_t m_kcp_data_size = 0;

	//发送消息包部分/////////////////////////////////////////////////////////////////////////////////
public:
	// 处理发送，传入的memory由CarpRudpClient接管释放，外部不要释放
	void SendPocket(void* memory, const int memory_size)
	{
		// 计算每次发送的最大字节数
		const int max_size = static_cast<int>(m_kcp->mss * m_kcp->rcv_wnd);

		// 将一个包按最大字节数进行切割
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
		// 如果包列表是空的，或者socket已经关闭了，直接返回
		if (m_pocket_list.empty() || !m_socket)
		{
			m_executing = false;
			return;
		}

		// 获取一个结构体
		auto info = m_pocket_list.front();
		m_pocket_list.pop_front();

		// 发送
		m_socket->async_send_to(asio::buffer(info.memory, info.memory_size), m_endpoint
			, std::bind(&CarpRudpClient::HandleSend, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, info.memory));
	}
	void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory)
	{
		// 释放内存
		if (memory) free(memory);

		// 检查错误
		if (ec)
		{
			// 这里不通知断开连接，等待接受那部分通知断开
			ExecuteDisconnectCallback();
			return;
		}

		// 发送下一个包
		NextSend();
	}

private:
	static int UdpOutput(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		auto* self = static_cast<CarpRudpClient*>(user);
		// 发送
		char* memory = static_cast<char*>(malloc(len + sizeof(int)));
		// 填充当前session
		memcpy(memory, &self->m_session, sizeof(int));
		// 填充发送数据
		memcpy(memory + sizeof(int), buf, len);

		// 构建内存结构
		PocketInfo info;
		info.memory_size = static_cast<int>(len + sizeof(int));
		info.memory = memory;

		// 添加到待发送列表
		self->m_pocket_list.push_back(info);
		// 如果已经正在发送了，那么就直接返回
		if (self->m_executing) return 0;
		// 标记正在发送
		self->m_executing = true;
		// 发送一个消息包
		self->NextSend();
		return 0;
	}

private:
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; };
	std::list<PocketInfo> m_pocket_list;  // 待发送的数据包列表
	bool m_executing = false;	// is in sending

private:
	std::function<void()> m_failed_func;
	std::function<void()> m_succeed_func;
	std::function<void()> m_disconnected_func;
	std::function<void(void*, int)> m_message_func;
};

#endif