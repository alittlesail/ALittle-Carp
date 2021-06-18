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

	// 启动和关闭
	virtual bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpRudpInterface* rudp_interface, CarpSchedule* schedule) = 0;
	virtual void Close(bool exit) = 0;

	// 获取ip和端口
	virtual const std::string& GetYunIp() const = 0;
	virtual const std::string& GetIp() const = 0;
	virtual int GetPort() const = 0;

	virtual CarpUSocketPtr GetSocket() = 0;

	virtual void SendPocket(const asio::ip::udp::endpoint& endpoint, void* memory, int size) = 0;

	// 发送心跳回调
	virtual void ServerSendHeartbeat(const asio::error_code& ec, int interval) = 0;

	// 更新kcp
	virtual void ServerUpdateKcp(const asio::error_code& ec, int interval) = 0;

	// 处理发送回调
	virtual void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory) = 0;

	// 处理接收数据
	virtual void HandleRead(const asio::error_code& ec, std::size_t actual_size) = 0;
	
	// 处理消息事件
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
		// 获取客户端的公网IP
		m_remote_ip = m_endpoint.address().to_string();
		m_remote_port = m_endpoint.port();

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
	}
	
	~CarpRudpReceiver()
	{
		// 关闭socket，释放资源
		Close(false);
	}

	friend class CarpRudpServerImpl;

public:
	bool CheckSession(int session) const { return m_session == session; }
	bool CheckHeartbeat(time_t cur_time, int interval)
	{
		// 如果标记为0，那么就检查成功
		if (m_last_heartbeat == 0) return true;
		// 如果间隔时间超过，那么就检查失败
		if (cur_time - m_last_heartbeat >= interval) return false;
		// 保存当前时间，用于下次检查
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
		
		// 发送
		char* memory = static_cast<char*>(malloc(len + sizeof(int)));
		// 填充当前session
		memcpy(memory, &self->m_session, sizeof(int));
		// 填充发送数据
		memcpy(memory + sizeof(int), buf, len);

		// 发送数据
		server->SendPocket(self->m_endpoint, memory, static_cast<int>(len + sizeof(int)));
		return 0;
	}
	
	// 关闭连接，释放内存
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
					auto n_session = -m_session; // 这里使用负数的session来作为关闭操作
					memcpy(buffer, &n_session, sizeof(int));
					memcpy(buffer + sizeof(int), &m_kcp->conv, sizeof(uint32_t));
					server->GetSocket()->send_to(asio::buffer(buffer, size), m_endpoint);
				}
				else
				{
					const auto size = sizeof(int) + sizeof(uint32_t);
					char* memory = static_cast<char*>(malloc(size));
					auto n_session = -m_session; // 这里使用负数的session来作为关闭操作
					memcpy(memory, &n_session, sizeof(int));
					memcpy(memory + sizeof(int), &m_kcp->conv, sizeof(uint32_t));
					server->SendPocket(m_endpoint, memory, size);
				}
			}
		}
		m_is_connected = false;
		// 重置偏移
		m_kcp_data_size = 0;

		// 释放kcp
		if (m_kcp)
		{
			ikcp_release(m_kcp);
			m_kcp = nullptr;
		}
	}

	//接受部分/////////////////////////////////////////////////////////////////////////////////
private:
	void HandleRead(const asio::ip::udp::endpoint& endpoint, const char* buffer, size_t actual_size)
	{
		// 保存最新的endpoint
		m_endpoint = endpoint;
		
		// 把数据喂给kcp
		int result = ikcp_input(m_kcp, buffer, static_cast<int>(actual_size));
		if (result < 0) CARP_ERROR("ikcp_input:" << result);

		// 接收所有可以接收的数据
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

		// 开始遍历处理
		size_t kcp_data_offset = 0;
		while (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE <= m_kcp_data_size)
		{
			char* memory = m_kcp_buffer.data() + kcp_data_offset;
			// 读取协议大小
			const auto message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(memory);
			CARP_MESSAGE_ID* message_id = reinterpret_cast<CARP_MESSAGE_ID*>(memory + sizeof(CARP_MESSAGE_SIZE));
			CARP_MESSAGE_RPCID* message_rpcid = reinterpret_cast<CARP_MESSAGE_RPCID*>(memory + sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID));

			// 如果协议体的数据不足，那么就跳出
			if (kcp_data_offset + CARP_PROTOCOL_HEAD_SIZE + message_size > m_kcp_data_size) break;
			kcp_data_offset += CARP_PROTOCOL_HEAD_SIZE;

			// 申请内存
			memory = static_cast<char*>(malloc(message_size));
			memcpy(memory, m_kcp_buffer.data() + kcp_data_offset, message_size);
			// 偏移向后走
			kcp_data_offset += message_size;

			// 如果是心跳包，那么就标记
			if (*message_id == HeartbeatMessage::GetStaticID()) m_last_heartbeat = 0;

			// 通过消息队列执行发送操作
			m_schedule->Execute(std::bind(&CarpRudpServer::HandleRudpMessage, m_server.lock(), this->shared_from_this()
				, message_size, *message_id, *message_rpcid, memory));
		}

		// 移动内存
		if (kcp_data_offset > 0)
		{
			memmove(m_kcp_buffer.data(), m_kcp_buffer.data() + kcp_data_offset, m_kcp_data_size - kcp_data_offset);
			m_kcp_data_size -= kcp_data_offset;
		}
	}

public:
	// 读取客户端的公网IP和端口
	const std::string& GetRemoteIP() const { return m_remote_ip; }
	int GetRemotePort() const { return m_remote_port; }
	bool IsConnected() const { return m_is_connected; }

private:
	asio::ip::udp::endpoint m_endpoint;	// endpoint
	int m_session = 0;
	ikcpcb* m_kcp = nullptr;

private:
	std::string m_remote_ip;			// 客户端的公网IP
	int m_remote_port = 0;				// 客户端的公网端口

private:
	std::vector<char> m_kcp_buffer;					// 用于接收kcp数据的
	size_t m_kcp_data_size = 0;
	// 归属的服务器
	CarpRudpServerWeakPtr m_server;
	// 对应调用模块
	CarpRudpInterface* m_rudp_interface = nullptr;
	// 调度模块
	CarpSchedule* m_schedule = nullptr;

public:
	// 发送协议
	void Send(const CarpMessage& message)
	{
		// 如果已经关闭，那么就不发送数据包
		if (m_is_connected == false) return;
		if (m_kcp == nullptr) return;

		// 获取消息包总大小
		CARP_MESSAGE_SIZE message_size = message.GetTotalSize();
		// 获取消息包ID
		CARP_MESSAGE_ID message_id = message.GetID();
		// 获取RPCID
		CARP_MESSAGE_RPCID message_rpcid = message.GetRpcID();

		// 计算内存大小
		const int memory_size = CARP_PROTOCOL_HEAD_SIZE + message_size;

		// 申请内存
		char* memory = static_cast<char*>(malloc(memory_size));
		if (memory == nullptr)
		{
			CARP_ERROR("memory is null");
			return;
		}
		char* body_memory = memory;

		// 写入消息包大小和ID
		memcpy(body_memory, &message_size, sizeof(CARP_MESSAGE_SIZE));
		body_memory += sizeof(CARP_MESSAGE_SIZE);
		memcpy(body_memory, &message_id, sizeof(CARP_MESSAGE_ID));
		body_memory += sizeof(CARP_MESSAGE_ID);
		memcpy(body_memory, &message_rpcid, sizeof(CARP_MESSAGE_RPCID));
		body_memory += sizeof(CARP_MESSAGE_RPCID);

		// 系列化消息
		message.Serialize(body_memory);

		// 计算每次发送的最大字节数
		const int max_size = static_cast<int>(m_kcp->mss * m_kcp->rcv_wnd);

		// 将一个包按最大字节数进行切割
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
	bool m_is_connected = true;// 是否处于连接状态
	time_t m_last_heartbeat = 0;	// 上次发送心跳包
};

#define CARP_UDP_SERVER_BUFFER_SIZE 10240 // 设置一个大的值，肯定会超过一个udp

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
	// 启动服务器
	// yun_ip 云服务器的映射ip
	// ip	服务器的IP
	// port 服务器的端口
	// heartbeat TCP心跳包保活间隔时间，单位秒
	bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpRudpInterface* rudp_interface, CarpSchedule* schedule) override
	{
		m_rudp_interface = rudp_interface;
		m_schedule = schedule;

		// 如果已经开启了就直接返回
		if (m_socket)
		{
			CARP_ERROR("RudpServer already started(ip: " << m_ip << ", port:" << m_port << ")");
			return false;
		}

		// 创建一个接收器，并启动
		m_socket = std::make_shared<asio::ip::udp::socket>(schedule->GetIOService());
		asio::ip::udp::endpoint endpoint = asio::ip::udp::endpoint(asio::ip::address_v4::from_string(ip), port);

		// 打开socket
		asio::error_code ec;
		m_socket->open(endpoint.protocol(), ec);
		if (ec)
		{
			CARP_ERROR("RudpServer udp socket open error: " << ec.value());
			return false;
		}
		// 绑定端口
		m_socket->bind(endpoint, ec);
		if (ec)
		{
			CARP_ERROR("RudpServer udp socket bind error: " << ec.value());
			return false;
		}
		
		// 创建定时器
		m_heartbeat_timer = std::make_shared<AsioTimer>(schedule->GetIOService(), std::chrono::seconds(heartbeat));
		m_heartbeat_timer->async_wait(std::bind(&CarpRudpServer::ServerSendHeartbeat, this->shared_from_this(), std::placeholders::_1, heartbeat));

		// 创建定时器
		m_kcp_timer = std::make_shared<AsioTimer>(schedule->GetIOService(), std::chrono::milliseconds(20));
		m_kcp_timer->async_wait(std::bind(&CarpRudpServer::ServerUpdateKcp, this->shared_from_this(), std::placeholders::_1, 20));

		// 开始接收
		NextRead();

		// 保存ip端口
		m_yun_ip = yun_ip;
		m_ip = ip;
		m_port = port;

		CARP_SYSTEM("RudpServer: start succeed at " << m_ip << ":" << m_port);
		return true;
	}

	// 关闭服务器
	void Close(bool exit) override
	{
		// 关闭所有客户端连接
		for (auto& pair : m_outer_map)
		{
			m_conv_creator.ReleaseID(pair.first);
			pair.second->Close(exit);
		}
		m_outer_map.clear();

		// 释放带发送的消息包
		for (auto& info : m_pocket_list) free(info.memory);
		m_pocket_list.clear();

		// 标记为不是正在发包
		m_executing = false;
		// 关闭并释放接收器
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
			m_socket = CarpUSocketPtr();
		}

		// 释放定时器
		if (m_heartbeat_timer)
		{
			m_heartbeat_timer->cancel();
			m_heartbeat_timer = AsioTimerPtr();
		}

		// 释放定时器
		if (m_kcp_timer)
		{
			m_kcp_timer->cancel();
			m_kcp_timer = AsioTimerPtr();
		}

		CARP_SYSTEM("RudpServer: stop succeed.");
	}

	CarpUSocketPtr GetSocket() override { return m_socket; }

private:
	// 接收数据包
	void NextRead()
	{
		// 检查接收器是否合法
		if (!m_socket) return;

		// 开始等待接收
		m_socket->async_receive_from(asio::buffer(m_udp_buffer, sizeof(m_udp_buffer)), m_receiver
			, std::bind(&CarpRudpServer::HandleRead, this->shared_from_this()
			, std::placeholders::_1, std::placeholders::_2));
	}

	// 处理新的socket接入
	void HandleRead(const asio::error_code& ec, std::size_t actual_size) override
	{
		if (ec)
		{
			CARP_ERROR("RudpServer read failed: " << ec.value());
			NextRead();
			return;
		}

		const auto size = sizeof(int) + sizeof(uint32_t);
		// 丢弃掉无效包
		if (actual_size < size)
		{
			NextRead();
			return;
		}

		// 取session
		auto session = *reinterpret_cast<int*>(m_udp_buffer);
		// 取conv
		auto conv = ikcp_getconv(m_udp_buffer + sizeof(int));

		// 判断是否是请求连接
		if (session == 0 && conv == 0)
		{
			// 如果不能再创建conv了，那么就表示断开
			if (m_conv_creator.IsEmpty())
			{
				void* memory = malloc(size);
				memset(memory, 0, size);
				SendPocket(m_receiver, memory, size);

				NextRead();
				return;
			}

			// 获取新的conv
			conv = m_conv_creator.CreateID();
			// 生成一个session
			session = rand();

			// 发送连接应答
			{
				char* memory = static_cast<char*>(malloc(size));
				memcpy(memory, &session, sizeof(int));
				memcpy(memory + sizeof(int), &conv, sizeof(uint32_t));
				SendPocket(m_receiver, memory, size);
			}

			// 处理新的连接
			HandleOuterConnect(m_receiver, session, conv);

			NextRead();
			return;
		}
		
		// 判断是否是主动关闭连接
		if (session < 0 && conv != 0)
		{
			// 检查连接对象和session
			const auto it = m_outer_map.find(conv);
			if (it != m_outer_map.end() && it->second->CheckSession(-session))
				HandleOuterDisconnected(conv);

			NextRead();
			return;
		}
		
		// 判断是否是发送数据
		if (session > 0 && conv != 0)
		{
			// 检查连接对象和session
			auto it = m_outer_map.find(conv);
			if (it != m_outer_map.end() && it->second->CheckSession(session))
				it->second->HandleRead(m_receiver, m_udp_buffer + sizeof(int), actual_size - sizeof(int));

			NextRead();
			return;
		}
		
		// 剩下的都是无效数据

		// 接收下一个连接
		NextRead();
	}

public:
	void SendPocket(const asio::ip::udp::endpoint& endpoint, void* memory, int size) override
	{
		// 构建内存结构
		PocketInfo info;
		info.memory_size = size;
		info.memory = memory;
		info.endpoint = endpoint;

		// 添加到待发送列表
		m_pocket_list.push_back(info);
		// 如果已经正在发送了，那么就直接返回
		if (m_executing) return;
		// 标记正在发送
		m_executing = true;
		// 发送一个消息包
		NextSend();
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
		m_socket->async_send_to(asio::buffer(info.memory, info.memory_size), info.endpoint
			, std::bind(&CarpRudpServer::HandleSend, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, info.memory));
	}
	void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory) override
	{
		// 释放内存
		if (memory) free(memory);

		// 检查错误
		if (ec)
		{
			CARP_ERROR("rudp server send failed: " << ec.value());
			return;
		}

		// 发送下一个包
		NextSend();
	}
	
public:
	// 获取本地的服务器ip和端口
	const std::string& GetYunIp() const override { return m_yun_ip; }
	const std::string& GetIp() const override { return m_ip; }
	int GetPort() const override { return m_port; }

private:
	std::string m_yun_ip;       // 云服务器对外的IP
	std::string m_ip;			// 本地服务器的IP
	int m_port = 0;				// 本地服务器的端口

private:
	CarpUSocketPtr m_socket;
	char m_udp_buffer[CARP_UDP_SERVER_BUFFER_SIZE] = {};
	asio::ip::udp::endpoint m_receiver;			// 接收消息源

private:
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; asio::ip::udp::endpoint endpoint; };
	std::list<PocketInfo> m_pocket_list;  // 待发送的数据包列表
	bool m_executing = false;	// is in sending

///////////////////////////////////////////////////////////////////////////////////////////////

private:
	// 处理一个新的socket
	void HandleOuterConnect(const asio::ip::udp::endpoint& endpoint, int session, uint32_t conv)
	{
		// 创建一个客户端连接
		const auto receiver = std::make_shared<CarpRudpReceiver>(endpoint, session, conv, this->shared_from_this(), m_rudp_interface, m_schedule);
		// 保存起来
		m_outer_map[conv] = receiver;

		// 通知客户端连接进来了
		m_rudp_interface->HandleRudpConnect(receiver);
	}

	// 处理某个连接断开了
	void HandleOuterDisconnected(uint32_t conv)
	{
		auto it = m_outer_map.find(conv);
		if (it == m_outer_map.end()) return;

		const auto receiver = it->second;
		
		// 关闭并移除客户端连接
		it->second->Close(false);
		m_outer_map.erase(it);

		// 回收
		m_conv_creator.ReleaseID(conv);

		// 通知断开连接
		m_rudp_interface->HandleRudpDisconnect(receiver);
	}

public:
	// 处理消息事件
	void HandleRudpMessage(CarpRudpReceiverPtr receiver, CARP_MESSAGE_SIZE message_size, CARP_MESSAGE_ID message_id, CARP_MESSAGE_RPCID message_rpcid, void* memory) override
	{
		// 通知处理消息包
		m_rudp_interface->HandleRudpMessage(receiver, message_size, message_id, message_rpcid, memory);
		// 释放内存
		if (memory) free(memory);
	}

private:
	// 保存客户端连接对象
	std::unordered_map<uint32_t, CarpRudpReceiverPtr> m_outer_map;	// container outer
	CarpSafeIDCreator<uint32_t> m_conv_creator; // conv生成器
	CarpRudpInterface* m_rudp_interface = nullptr;
	CarpSchedule* m_schedule = nullptr;

private:
	// 服务器间隔一定时间向客户端发送心跳包
	void ServerSendHeartbeat(const asio::error_code& ec, int interval) override
	{
		const auto cur_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
		
		// 检查心跳包
		std::set<uint32_t> remove_map;
		for (auto& pair : m_outer_map)
		{
			if (pair.second->CheckHeartbeat(cur_time, interval)) continue;
			remove_map.insert(pair.first);
		}

		// 处理心跳超时
		for (const auto& conv : remove_map)
			HandleOuterDisconnected(conv);
		
		// 向所有客户端发送心跳包
		const HeartbeatMessage msg;
		const auto end = m_outer_map.end();
		for (auto it = m_outer_map.begin(); it != end; ++it)
			it->second->Send(msg);

		if (!m_heartbeat_timer) return;
		m_heartbeat_timer->expires_after(std::chrono::seconds(interval));
		m_heartbeat_timer->async_wait(std::bind(&CarpRudpServer::ServerSendHeartbeat, this->shared_from_this(),
		                                        std::placeholders::_1, interval));
	}
	// 心跳包定时器
	AsioTimerPtr m_heartbeat_timer;

	// 服务器间隔一定时间向客户端发送心跳包
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
	// 心跳包定时器
	AsioTimerPtr m_kcp_timer;
};

#endif