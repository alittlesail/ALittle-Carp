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
		// 释放内存
		if (m_memory) { free(m_memory); m_memory = 0; }
	}

	//连接部分/////////////////////////////////////////////////////////////////////////////////
public:
	/* 异步连接目标服务器
	 * @param route_id: 路由ID，全局唯一
	 * @param ip: 目标服务器IP
	 * @param port: 目标服务器端口
	 */
	void Connect( const std::string& ip, unsigned int port, asio::io_service* io_service
		, std::function<void()> failed_func, std::function<void()> succeed_func, std::function<void()> disconnected_func
		, std::function<void(void*, int)> message_func)
	{
		// 检查是否正在连接
		if (IsConnected() || IsConnecting()) return;

		m_failed_func = failed_func;
		m_succeed_func = succeed_func;
		m_disconnected_func = disconnected_func;
		m_message_func = message_func;

		// 标记为正在连接
		m_is_connecting = true;
		// 创建一个socket对象
		m_socket = CarpSocketPtr(new asio::ip::tcp::socket(*io_service));
		// 创建一个目标服务器的连接点
		asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string(ip), port);

		// 保存并初始化
		m_ip = ip;
		m_port = port;

		// 开始异步连接
		m_socket->async_connect(ep, std::bind(&CarpConnectClient::HandleAsyncConnect, this->shared_from_this(), std::placeholders::_1));
	}

	// 判断是否已经连接
	bool IsConnected() const { return m_is_connected; }

	// 是否正在连接
	bool IsConnecting() const { return m_is_connecting; }

	// 关闭连接
	void Close()
	{
		// 释放带发送的消息包
		auto end = m_pocket_list.end();
		for (auto it = m_pocket_list.begin(); it != end; ++it)
			free(it->memory);
		m_pocket_list.clear();

		// 标记为不是正在发包
		m_excuting = false;
		// 标记为不是正在连接
		m_is_connecting = false;
		// 标记为未连接
		m_is_connected = false;

		// 释放socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
		}

		// 这里不要急着释放m_memory，可能asio正在用
		// 放到析构函数里面释放
		// if (m_memory) { free(m_memory); m_memory = 0; }
	}

private:
	std::string m_ip;			// 目标服务器的IP
	unsigned int m_port = 0;		// 目标服务器端口

public:
	// 获取目标服务器IP和端口
	const std::string& GetIP() const { return m_ip; }
	unsigned int GetPort() const { return m_port; }

private:
	// 异步连接
	void HandleAsyncConnect(const asio::error_code& ec)
	{
		// 标记为不是正在连接
		m_is_connecting = false;
		// 标记为不是正在发包
		m_excuting = false;
		// 标记为已连接
		m_is_connected = false;
		if (ec)
		{
			// 这个日志不打印，因为会出现太多，又不重要
			// ALITTLE_SYSTEM(u8"ConnectClient 连接失败: " << SUTF8(asio::system_error(ec).what()) << ", ip:" << m_ip << ", port:" << m_port);

			// 处理连接失败
			HandleConnectFailed();
			return;
		}
		// 设置 no delay
		m_socket->set_option(asio::ip::tcp::no_delay(true));

		// 标记为已连接
		m_is_connected = true;

		// 开始接受消息包
		NextReadHead();

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
	// 读取协议
	void NextReadHead()
	{
		// 如果已经释放了就直接返回
		if (!m_socket) return;

		// 开始接受协议头
		asio::async_read(*m_socket, asio::buffer(m_message_head, sizeof(m_message_head))
			, std::bind(&CarpConnectClient::HandleReadHead, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadHead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// 释放内存
			if (m_memory) { free(m_memory); m_memory = 0; }
			ExecuteDisconnectCallback();
			return;
		}

		// 读取协议大小
		CARP_MESSAGE_SIZE message_size = *(CARP_MESSAGE_SIZE*)m_message_head;

		// 申请内存
		if (m_memory) { free(m_memory); m_memory = 0; }
		m_memory = malloc(message_size + CARP_PROTOCOL_HEAD_SIZE);
		char* body_memory = (char*)m_memory;

		// 协议头复制到内存
		memcpy(body_memory, m_message_head, sizeof(m_message_head));

		// 如果没有协议体表示读取完成
		if (message_size == 0)
		{
			ReadComplete();
			NextReadHead();
			return;
		}

		// 开始读取协议体
		asio::async_read(*m_socket, asio::buffer((char*)m_memory + CARP_PROTOCOL_HEAD_SIZE, message_size)
			, std::bind(&CarpConnectClient::HandleReadBody, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadBody(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// 释放内存
			if (m_memory) { free(m_memory); m_memory = 0; }
			// 通知断开连接
			ExecuteDisconnectCallback();
			return;
		}

		// 处理协议
		ReadComplete();
		// 继续读取下一个消息包
		NextReadHead();
	}

private:
	// 处理协议
	void ReadComplete()
	{

		// 获取协议大小
		CARP_MESSAGE_SIZE message_size = *(CARP_MESSAGE_SIZE*)m_message_head;
		// 发送给调度系统
		if (m_message_func)
			m_message_func(m_memory, message_size + CARP_PROTOCOL_HEAD_SIZE);
		else
			free(m_memory);
		// 内存已经移交出去，HandleMessage会负责释放
		// 这里置0
		m_memory = nullptr;
	}

public:
	CarpSocketPtr m_socket;					// socket

private:
	// 保存协议头
	char m_message_head[CARP_PROTOCOL_HEAD_SIZE]{};
	// 保存协议体
	void* m_memory = nullptr;

	//发送消息包部分/////////////////////////////////////////////////////////////////////////////////
public:
	// 处理发送，传入的memory由CarpConnectClient接管释放，外部不要释放
	void SendPocket(void* memory, int memory_size)
	{
		// 构建内存结构
		PocketInfo info = {0};
		info.memory_size = memory_size;
		info.memory = memory;

		// 添加到待发送列表
		m_pocket_list.push_back(info);
		// 如果已经正在发送了，那么就直接返回
		if (m_excuting) return;
		// 标记正在发送
		m_excuting = true;
		// 发送一个消息包
		NextSend();
	}
	
	void NextSend()
	{
		// 如果包列表是空的，或者socket已经关闭了，直接返回
		if (m_pocket_list.empty() || !m_socket)
		{
			m_excuting = false;
			return;
		}

		// 获取一个结构体
		PocketInfo info = m_pocket_list.front();
		m_pocket_list.pop_front();

		// 发送
		asio::async_write(*m_socket, asio::buffer(info.memory, info.memory_size)
			, std::bind(&CarpConnectClient::HandleSend, this->shared_from_this()
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
	struct PocketInfo { int memory_size; void* memory; };
	std::list<PocketInfo> m_pocket_list;  // 待发送的数据包列表

	bool m_excuting = false;	// is in sending

private:
	std::function<void()> m_failed_func;
	std::function<void()> m_succeed_func;
	std::function<void()> m_disconnected_func;
	std::function<void(void*, int)> m_message_func;
};

#endif