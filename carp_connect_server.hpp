#ifndef CARP_MESSAGE_SERVER_INCLUDED
#define CARP_MESSAGE_SERVER_INCLUDED

#include <asio.hpp>
#include <memory>
#include <functional>

#include "carp_crypto.hpp"
#include "carp_log.hpp"
#include "carp_message.hpp"
#include "carp_string.hpp"
#include "carp_schedule.hpp"

class CarpConnectReceiver;
typedef std::shared_ptr<CarpConnectReceiver> CarpConnectReceiverPtr;
typedef std::weak_ptr<CarpConnectReceiver> CarpConnectReceiverWeakPtr;

typedef std::shared_ptr<asio::ip::tcp::socket> SocketPtr;
typedef std::shared_ptr<asio::ip::tcp::acceptor> AcceptorPtr;
typedef asio::basic_waitable_timer<std::chrono::system_clock> AsioTimer;
typedef std::shared_ptr<AsioTimer> AsioTimerPtr;

class CarpConnectSchedule : public CarpSchedule
{
public:
	virtual ~CarpConnectSchedule() {}

	virtual void HandleClientConnect(CarpConnectReceiverPtr sender) { }

	virtual void HandleClientDisconnect(CarpConnectReceiverPtr sender) { }

	virtual void HandleClientMessage(CarpConnectReceiverPtr sender, int message_size, int message_id, int message_rpcid, void* memory) {}
};

class CarpConnectServer : public std::enable_shared_from_this<CarpConnectServer>
{
public:
	virtual ~CarpConnectServer() {}

	// 启动和关闭
	virtual bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpConnectSchedule* schedule) = 0;
	virtual void Close() = 0;

	// 获取ip和端口
	virtual const std::string& GetYunIp() const = 0;
	virtual const std::string& GetIp() const = 0;
	virtual int GetPort() const = 0;

	// 发送心跳回调
	virtual void ServerSendHeartbeat(const asio::error_code& ec, int interval) = 0;

	// 接收连接
	virtual void HandleAccept(const asio::error_code& ec, SocketPtr socket, int error_count) = 0;

	// 处理一个新的socket
	virtual void HandleOuterConnect(SocketPtr socket) = 0;

	// 处理某个连接断开了
	virtual void HandleOuterDisconnected(CarpConnectReceiverPtr receiver) = 0;

	// 处理消息事件
	virtual void HandleClientMessage(CarpConnectReceiverPtr receiver, CARP_MESSAGE_SIZE message_size, CARP_MESSAGE_ID message_id, CARP_MESSAGE_RPCID message_rpcid, void* memory) = 0;
};
typedef std::shared_ptr<CarpConnectServer> CarpConnectServerPtr;
typedef std::weak_ptr<CarpConnectServer> CarpConnectServerWeakPtr;

class CarpConnectReceiver : public std::enable_shared_from_this<CarpConnectReceiver>
{
public:
	CarpConnectReceiver(SocketPtr socket, CarpConnectServerWeakPtr server, CarpConnectSchedule* schedule)
		: m_socket(socket), m_server(server), m_schedule(schedule)
	{
		// 获取客户端的公网IP
		m_remote_ip = socket->remote_endpoint().address().to_string();
		m_remote_port = socket->remote_endpoint().port();
	}
	
	~CarpConnectReceiver()
	{
		// 关闭socket，释放资源
		Close();
		// 释放内存
		if (m_memory) { free(m_memory); m_memory = nullptr; }
		if (m_websocket_buffer) { free(m_websocket_buffer); m_websocket_buffer = nullptr; }
	}

	friend class CarpConnectServerImpl;

public:
	// 关闭连接，释放内存
	void Close()
	{
		// 释放正在发送的数据包
		const auto end = m_pocket_list.end();
		for (auto it = m_pocket_list.begin(); it != end; ++it)
			free(it->memory);
		m_pocket_list.clear();

		// 标记不是正在执行
		m_executing = false;

		// 关闭socket
		if (m_socket)
		{
			asio::error_code ec;
			m_socket->close(ec);
		}
		m_socket = SocketPtr();

		// 这里不要急着释放m_memory，可能asio正在用
		// 放到析构函数里面释放
		// if (m_memory) { free(m_memory); m_memory = 0; }
	}

	//接受部分/////////////////////////////////////////////////////////////////////////////////

private:
	void NextReadHeadFirst()
	{
		asio::async_read(*m_socket, asio::buffer(m_message_head, sizeof(m_message_head))
			, std::bind(&CarpConnectReceiver::HandleReadHeadFirst, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadHeadFirst(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed:" << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// check first char, if is 'G' then is websocket, otherwise binary
		if (m_message_head[0] != 'G')
			HandleReadHeadBinary(ec, actual_size);
		else
		{
			m_is_websocket = true;
			HandleWebSocketHandShakeReceive(ec, actual_size);
		}
	}

private:
	// 读取协议
	void NextReadHeadBinary()
	{
		// 如果socket已经被释放，就直接返回
		if (!m_socket) return;

		// 异步读取数据
		asio::async_read(*m_socket, asio::buffer(m_message_head, sizeof(m_message_head))
			, std::bind(&CarpConnectReceiver::HandleReadHeadBinary, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadHeadBinary(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// 释放内存
			if (m_memory) { free(m_memory); m_memory = 0; }
			// 读取失败了说明是连接断开了，通知给server
			CARP_SYSTEM("CarpConnectReceiver::HandleReadHead receive failed:" << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// 从协议头读取数据包大小
		CARP_MESSAGE_SIZE message_size = *(CARP_MESSAGE_SIZE*)m_message_head;

		// 如果数据包大小为0，那么就直接执行完成
		if (message_size == 0)
		{
			// 读取完成操作
			ReadComplete();
			// 进行下一步读取
			NextReadHeadBinary();
			return;
		}

		// 有协议体，那么就申请内存
		if (m_memory) { free(m_memory); m_memory = 0; }
		m_memory = malloc(message_size);

		// 读取协议体
		asio::async_read(*m_socket, asio::buffer(m_memory, message_size)
			, std::bind(&CarpConnectReceiver::HandleReadBodyBinary, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadBodyBinary(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// 释放内存
			if (m_memory) { free(m_memory); m_memory = 0; }
			// 读取失败了说明是断开了，通知给server
			CARP_SYSTEM("CarpConnectReceiver::HandleReadBody receive failed:" << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server) server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// 读取完成操作
		ReadComplete();
		// 进行下一步读取
		NextReadHeadBinary();
	}

private:
	void NextReadWebSocketHandShake()
	{
		m_socket->async_read_some(asio::buffer(m_message_head, sizeof(m_message_head))
			, std::bind(&CarpConnectReceiver::HandleWebSocketHandShakeReceive, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleWebSocketHandShakeReceive(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed:" << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// store current size
		int current_size = (int)m_websocket_handshake.size();
		// add to buffer
		m_websocket_handshake.append(m_message_head, actual_size);

		// stop receive if lager than max size
		if (m_websocket_handshake.size() > WEBSOCKET_HEAD_BUFFER_SIZE_MAX)
		{
			CARP_SYSTEM("websocket hand shake is large than " << WEBSOCKET_HEAD_BUFFER_SIZE_MAX);
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// set start point to find
		int find_start_pos = current_size - (int)strlen("\r\n\r\n");
		if (find_start_pos < 0) find_start_pos = 0;

		// find \r\n\r\n
		std::string::size_type find_pos = m_websocket_handshake.find("\r\n\r\n", find_start_pos);
		if (find_pos != std::string::npos)
		{
			// the end position of \r\n\r\n in m_http_head
			int head_size = (int)find_pos + (int)strlen("\r\n\r\n");
			// resize http size, delete other data
			m_websocket_handshake.resize(head_size);

			// get the key
			std::string::size_type pos = m_websocket_handshake.find("Sec-WebSocket-Key:");
			if (pos == std::string::npos)
			{
				CARP_SYSTEM("can't find Sec-WebSocket-Key in web socket head");
				CarpConnectServerPtr server = m_server.lock();
				if (server)	server->HandleOuterDisconnected(this->shared_from_this());
				return;
			}

			pos += strlen("Sec-WebSocket-Key:");

			std::string::size_type end_pos = m_websocket_handshake.find("\r\n", pos);
			if (pos == std::string::npos)
			{
				CARP_SYSTEM("can't find end of Sec-WebSocket-Key in web socket head");
				CarpConnectServerPtr server = m_server.lock();
				if (server)	server->HandleOuterDisconnected(this->shared_from_this());
				return;
			}

			// encode the key
			std::string key = m_websocket_handshake.substr(pos, end_pos - pos);
			CarpString::TrimLeft(key);
			CarpString::TrimRight(key);
			key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

			unsigned int digest[5];
			CarpCrypto::Sha1(key, digest);

			char* tmp = reinterpret_cast<char*>(digest);
			char new_digest[20] = { 0 };
			for (int i = 0; i < 5; ++i)
			{
				new_digest[i * 4] = tmp[i * 4 + 3];
				new_digest[i * 4 + 1] = tmp[i * 4 + 2];
				new_digest[i * 4 + 2] = tmp[i * 4 + 1];
				new_digest[i * 4 + 3] = tmp[i * 4];
			}

			key = CarpCrypto::Base64Encode(new_digest, 20);

			m_websocket_handshake = "";
			m_websocket_handshake.append("HTTP/1.1 101 Switching Protocols\r\n");
			m_websocket_handshake.append("Sec-WebSocket-Version: 13\r\n");
			m_websocket_handshake.append("Upgrade: websocket\r\n");
			m_websocket_handshake.append("Connection: Upgrade\r\n");
			m_websocket_handshake.append("Server: ALittle\r\n");
			m_websocket_handshake.append("Sec-WebSocket-Accept: ").append(key).append("\r\n\r\n");

			// send hand shake
			asio::async_write(*m_socket, asio::buffer(m_websocket_handshake.c_str(), m_websocket_handshake.size())
				, std::bind(&CarpConnectReceiver::HandleWebSocketHandShakeSend, this->shared_from_this()
					, std::placeholders::_1, std::placeholders::_2));

			return;
		}

		NextReadWebSocketHandShake();
	}
	void HandleWebSocketHandShakeSend(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("websocket hand shake send failed!, " << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		NextWebSocketRead1();
	}

private:
	void NextWebSocketRead1()
	{
		asio::async_read(*m_socket, asio::buffer(m_frame_buffer, 2)
			, std::bind(&CarpConnectReceiver::HandleNextWebSocketRead1, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleNextWebSocketRead1(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed!, " << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		bool fin = (m_frame_buffer[0] & 0x80) > 0;
		m_op_code = m_frame_buffer[0] & 0x0F;
		m_current_is_message = false;

		switch (m_op_code)
		{
		case 0: // lian xu message
		case 1: // text message
		case 2: m_current_is_message = true; break;	// binary message
		case 8:
		{
			CARP_INFO("websocket client send to server close");
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}
		default: break;
		}

		m_has_mark = (m_frame_buffer[1] & 0x80) > 0;
		int payload_len = m_frame_buffer[1] & 0x7F;
		if (payload_len < 0)
		{
			CARP_SYSTEM("payload len < 0");
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		NextWebSocketRead2(payload_len);
	}
	void NextWebSocketRead2(int payload_len)
	{
		if (payload_len == 126)
			asio::async_read(*m_socket, asio::buffer(m_frame_buffer, 2)
				, std::bind(&CarpConnectReceiver::HandleNextWebSocketRead2, this->shared_from_this()
					, std::placeholders::_1, std::placeholders::_2));
		else if (payload_len == 127)
			asio::async_read(*m_socket, asio::buffer(m_frame_buffer, 8)
				, std::bind(&CarpConnectReceiver::HandleNextWebSocketRead2, this->shared_from_this()
					, std::placeholders::_1, std::placeholders::_2));
		else
		{
			m_data_length = payload_len;
			if (m_has_mark)
				NextWebSocketReadMark();
			else
				NextWebSocketReadData();
		}
	}
	void HandleNextWebSocketRead2(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed!, " << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		if (actual_size == 2)
		{
			short data_length = 0;
			unsigned char* char_data_length = reinterpret_cast<unsigned char*>(&data_length);
			char_data_length[0] = m_frame_buffer[1];
			char_data_length[1] = m_frame_buffer[0];
			m_data_length = data_length;
		}
		else
		{
			m_data_length = 0;
			unsigned char* char_data_length = reinterpret_cast<unsigned char*>(&m_data_length);
			char_data_length[0] = m_frame_buffer[7];
			char_data_length[1] = m_frame_buffer[6];
			char_data_length[2] = m_frame_buffer[5];
			char_data_length[3] = m_frame_buffer[4];
		}

		if (m_data_length < 0)
		{
			CARP_SYSTEM("m_data_length < 0");
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		if (m_has_mark)
			NextWebSocketReadMark();
		else
			NextWebSocketReadData();
	}
	void NextWebSocketReadMark()
	{
		asio::async_read(*m_socket, asio::buffer(m_frame_buffer, 4)
			, std::bind(&CarpConnectReceiver::HandleNextWebSocketReadMark, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleNextWebSocketReadMark(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed!, " << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		memcpy(m_mark, m_frame_buffer, 4);

		NextWebSocketReadData();
	}
	void NextWebSocketReadData()
	{
		if (m_websocket_buffer) free(m_websocket_buffer);
		m_websocket_buffer = malloc(m_data_length);

		asio::async_read(*m_socket, asio::buffer((char*)m_websocket_buffer, m_data_length)
			, std::bind(&CarpConnectReceiver::HandleNextWebSocketReadData, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2));
	}
	void HandleNextWebSocketReadData(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			CARP_SYSTEM("receive failed!, " << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server)	server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		if (m_current_is_message == false)
		{
			// release buffer
			ReleaseWebSocketBuffer();
			// read next
			NextWebSocketRead1();
			return;
		}

		// handle mark
		char* body = static_cast<char*>(m_websocket_buffer);
		if (m_has_mark)
		{
			for (unsigned int i = 0; i < actual_size; ++i)
				body[i] = (body[i] ^ m_mark[i % 4]);
		}

		HandleWebSocketReadData(static_cast<int>(actual_size), 0);
	}
	void HandleWebSocketReadData(int total_size, int offset)
	{// check offset
		if (offset >= total_size)
		{
			ReleaseWebSocketBuffer();
			NextWebSocketRead1();
			return;
		}
		// calc remain size
		int remain_size = total_size - offset;

		char* body = static_cast<char*>(m_websocket_buffer) + offset;

		// check is read message head
		if (m_head_size < sizeof(m_message_head))
		{
			// calc need head size
			int need_size = sizeof(m_message_head) - m_head_size;
			int copy_size = remain_size < need_size ? remain_size : need_size;

			// copy to message head
			memcpy(m_message_head + m_head_size, body, copy_size);
			m_head_size += copy_size;
			offset += copy_size;
			body += copy_size;

			// if head size is enough
			if (m_head_size == sizeof(m_message_head))
			{
				// get message size from head buffer
				const CARP_MESSAGE_SIZE message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(m_message_head);
				const CARP_MESSAGE_ID message_id = *reinterpret_cast<CARP_MESSAGE_ID*>(m_message_head + sizeof(CARP_MESSAGE_SIZE));

				// check message_id is 0 and message_size is 0 then is heartbeat message
				if (message_id == 0 && message_size == 0)
				{
					ReadComplete();
					HandleWebSocketReadData(total_size, offset);
					return;
				}

				if (message_size > MESSAGE_BUFFER_SIZE)
				{
					CARP_ERROR("message_size(" << message_size << ") is large then " << MESSAGE_BUFFER_SIZE);
					CarpConnectServerPtr server = m_server.lock();
					if (server)	server->HandleOuterDisconnected(this->shared_from_this());
					return;
				}

				// if message size is 0,then read completed
				if (message_size == 0)
				{
					ReadComplete();
					HandleWebSocketReadData(total_size, offset);
					return;
				}

				// create memory from pool
				m_memory = malloc(message_size);

				// copy remain data
				remain_size = total_size - offset;
				if (remain_size >= (int)message_size)
				{
					// copy message
					memcpy(m_memory, body, message_size);
					ReadComplete();

					offset += message_size;

					// handle next
					HandleWebSocketReadData(total_size, offset);
					return;
				}

				// copy remain
				memcpy(m_memory, body, remain_size);
				offset += remain_size;
				m_body_size = remain_size;
				// handle next
				HandleWebSocketReadData(total_size, offset);
				return;
			}

			HandleWebSocketReadData(total_size, offset);
			return;
		}

		// get message size from head buffer
		const CARP_MESSAGE_SIZE message_size = *reinterpret_cast<CARP_MESSAGE_SIZE*>(m_message_head);

		// offset body memory
		char* body_memory = (char*)m_memory;
		body_memory += m_body_size;

		// calc need size
		int need_size = static_cast<int>(message_size) - m_body_size;
		if (remain_size >= need_size)
		{
			// copy message
			memcpy(body_memory, body, need_size);
			ReadComplete();

			offset += need_size;

			// handle next
			HandleWebSocketReadData(total_size, offset);
			return;
		}

		// copy message
		memcpy(body_memory, body, remain_size);
		offset += remain_size;
		m_body_size += remain_size;
		// handle next
		HandleWebSocketReadData(total_size, offset);
	}
	void ReleaseWebSocketBuffer()
	{
		if (m_websocket_buffer)
		{
			free(m_websocket_buffer);
			m_websocket_buffer = 0;
		}
	}

private:
	bool m_is_websocket = false;
	void* m_websocket_buffer = nullptr;		// web socket buffer
	std::string m_websocket_handshake;	// websocket hand shake string
	char m_frame_buffer[8]={};				// frame buffer

	bool m_has_mark = false;				// has mark
	char m_mark[4]={};					// mark
	int m_op_code = 0;					// opcode
	int m_data_length = 0;				// data length

	int m_head_size = 0;				// head size received
	int m_body_size = 0;				// body size received
	bool m_current_is_message = false;		// current is message
//======================================================================================

private:
	// 处理协议
	void ReadComplete()
	{
		// 设置大小为0
		m_head_size = 0;
		m_body_size = 0;

		// 检查服务器是否还存在
		CarpConnectServerPtr server = m_server.lock();
		if (!server)
		{
			// 释放内存
			if (m_memory) { free(m_memory); m_memory = nullptr; }
			return;
		}

		// 读取协议头的信息
		CARP_MESSAGE_SIZE* message_size = reinterpret_cast<CARP_MESSAGE_SIZE*>(m_message_head);
		CARP_MESSAGE_ID* message_id = reinterpret_cast<CARP_MESSAGE_ID*>(m_message_head + sizeof(CARP_MESSAGE_SIZE));
		CARP_MESSAGE_RPCID* message_rpcid = reinterpret_cast<CARP_MESSAGE_RPCID*>(m_message_head + sizeof(CARP_MESSAGE_SIZE) + sizeof(CARP_MESSAGE_ID));

		// 通过消息队列执行发送操作
		m_schedule->Execute(std::bind(&CarpConnectServer::HandleClientMessage, server, this->shared_from_this()
			, *message_size, *message_id, *message_rpcid, m_memory));
		// 内存已经移交出去了，HandleClientMessage这个函数内部会负责释放
		// 这里置零
		m_memory = nullptr;
	}

public:
	// 读取客户端的公网IP和端口
	const std::string& GetRemoteIP() const { return m_remote_ip; }
	int GetRemotePort() const { return m_remote_port; }
	bool IsConnected() const { return m_is_connected; }

private:
	SocketPtr m_socket;				// Socket
	std::string m_remote_ip;		// 客户端的公网IP
	int m_remote_port = 0;				// 客户端的公网端口

private:
	// 协议头
	char m_message_head[CARP_PROTOCOL_HEAD_SIZE]={};
	// 协议体
	void* m_memory = nullptr;
	// 归属的服务器
	CarpConnectServerWeakPtr m_server;
	// 对应调用模块
	CarpConnectSchedule* m_schedule = nullptr;

	//发送部分/////////////////////////////////////////////////////////////////////////////////

public:
	// 发送协议
	void Send(const CarpMessage& message)
	{
		if (m_is_websocket)
			SendWebSocket(message);
		else
			SendBinary(message);
	}

private:
	void SendBinary(const CarpMessage& message)
	{
		// 如果已经关闭，那么就不发送数据包
		if (m_is_connected == false) return;

		// 获取消息包总大小
		CARP_MESSAGE_SIZE message_size = message.GetTotalSize();
		// 获取消息包ID
		CARP_MESSAGE_ID message_id = message.GetID();
		// 获取RPCID
		CARP_MESSAGE_RPCID message_rpcid = message.GetRpcID();

		// 计算内存大小
		const int memory_size = CARP_PROTOCOL_HEAD_SIZE + message_size;

		// 申请内存
		void* memory = malloc(memory_size);
		if (memory == nullptr)
		{
			CARP_ERROR("memory is null");
			return;
		}
		char* body_memory = static_cast<char*>(memory);

		// 写入消息包大小和ID
		memcpy(body_memory, &message_size, sizeof(CARP_MESSAGE_SIZE));
		body_memory += sizeof(CARP_MESSAGE_SIZE);
		memcpy(body_memory, &message_id, sizeof(CARP_MESSAGE_ID));
		body_memory += sizeof(CARP_MESSAGE_ID);
		memcpy(body_memory, &message_rpcid, sizeof(CARP_MESSAGE_RPCID));
		body_memory += sizeof(CARP_MESSAGE_RPCID);

		// 系列化消息
		message.Serialize(body_memory);

		// 发送数据包
		SendPocket(memory, memory_size);
	}
	void SendWebSocket(const CarpMessage& message)
	{// get the size of message body and save in head
		CARP_MESSAGE_SIZE message_size = message.GetTotalSize();
		// get the id of message and save in head
		CARP_MESSAGE_ID message_id = message.GetID();
		// 获取RPCID
		CARP_MESSAGE_RPCID message_rpcid = message.GetRpcID();

		// memory size = body size + head size
		unsigned int extend_size = message_size + CARP_PROTOCOL_HEAD_SIZE;
		unsigned int memory_size = extend_size;
		void* memory = nullptr;
		unsigned char* body_memory = nullptr;

		if (extend_size < 126)
		{
			memory_size += 2;

			// create memory from pool
			memory = malloc(memory_size);
			body_memory = static_cast<unsigned char*>(memory);
			// clear bit
			body_memory[0] = body_memory[1] = 0;
			// put FIN
			body_memory[0] = body_memory[0] | 0x80;
			// put op code
			body_memory[0] = body_memory[0] | 0x02;
			// put payload
			body_memory[1] = static_cast<unsigned char>(extend_size);
			body_memory += 2;
		}
		else if (extend_size <= 65535)
		{
			memory_size += 4;

			// create memory from pool
			memory = malloc(memory_size);
			body_memory = (unsigned char*)memory;
			// clear bit
			body_memory[0] = body_memory[1] = 0;
			// put FIN
			body_memory[0] = body_memory[0] | 0x80;
			// put opcode
			body_memory[0] = body_memory[0] | 0x02;
			// put payload
			body_memory[1] = 126;
			body_memory += 2;
			// put length
			unsigned short value = (unsigned short)extend_size;
			unsigned char* char_extend_size = (unsigned char*)(&value);
			body_memory[0] = char_extend_size[1];
			body_memory[1] = char_extend_size[0];
			body_memory += 2;
		}
		else
		{
			memory_size += 10;

			// create memory from pool
			memory = malloc(memory_size);
			body_memory = (unsigned char*)memory;
			// clear bit
			body_memory[0] = body_memory[1] = 0;
			// put FIN
			body_memory[0] = body_memory[0] | 0x80;
			// put opcode
			body_memory[0] = body_memory[0] | 0x02;
			// put payload
			body_memory[1] = 127;
			body_memory += 2;
			// put length
			unsigned char* char_extend_size = (unsigned char*)&extend_size;
			body_memory[0] = 0;
			body_memory[1] = 0;
			body_memory[2] = 0;
			body_memory[3] = 0;
			body_memory[4] = char_extend_size[3];
			body_memory[5] = char_extend_size[2];
			body_memory[6] = char_extend_size[1];
			body_memory[7] = char_extend_size[0];
			body_memory += 8;
		}

		// set head info
		memcpy(body_memory, &message_size, sizeof(CARP_MESSAGE_SIZE));
		body_memory += sizeof(CARP_MESSAGE_SIZE);
		memcpy(body_memory, &message_id, sizeof(CARP_MESSAGE_ID));
		body_memory += sizeof(CARP_MESSAGE_ID);
		memcpy(body_memory, &message_rpcid, sizeof(CARP_MESSAGE_RPCID));
		body_memory += sizeof(CARP_MESSAGE_RPCID);
		// transfer message to memory
		message.Serialize(body_memory);

		SendPocket(memory, memory_size);
	}

private:
	// 处理发送
	void SendPocket(void* memory, int memory_size)
	{
		// 保存信息
		PocketInfo info;
		info.memory_size = memory_size;
		info.memory = memory;

		// 添加一个数据包
		m_pocket_list.push_back(info);
		// 如果现在正在发送，那么就返回
		if (m_executing) return;
		// 标记为正在发送
		m_executing = true;
		// 发出一个包
		NextSend();
	}
	void NextSend()
	{
		// 如果没有数据包了，或者socket已经被关闭了，就返回
		if (m_pocket_list.empty() || !m_socket)
		{
			m_executing = false;
			return;
		}

		// 获取内存数据
		PocketInfo info = m_pocket_list.front();
		m_pocket_list.pop_front();

		// 异步发送
		asio::async_write(*m_socket, asio::buffer(info.memory, info.memory_size)
			, std::bind(&CarpConnectReceiver::HandleSend, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2, info.memory));
	}
	void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred, void* memory)
	{
		// 释放内存
		if (memory) free(memory);

		if (ec)
		{
			// 如果发送失败了，说明已经断开连接了
			CARP_SYSTEM("send failed:" << ec.value());
			CarpConnectServerPtr server = m_server.lock();
			if (server) server->HandleOuterDisconnected(this->shared_from_this());
			return;
		}

		// 发送下一个数据包
		NextSend();
	}

private:
	struct PocketInfo { int memory_size = 0; void* memory = nullptr; };
	std::list<PocketInfo> m_pocket_list; // 待发送的数据包列表

	bool m_executing = false;	// 是否正在发送
	bool m_is_connected = true;// 是否处于连接状态

private:
	static const int WEBSOCKET_HEAD_BUFFER_SIZE_MAX = 1024;
	static const int MESSAGE_BUFFER_SIZE = 102400000;
};

class CarpConnectServerImpl : public CarpConnectServer
{
public:
	virtual ~CarpConnectServerImpl() { Close(); }

	friend CarpConnectReceiver;

public:
	// 启动服务器
	// yun_ip 云服务器的映射ip
	// ip	服务器的IP
	// port 服务器的端口
	// heartbeat TCP心跳包保活间隔时间，单位秒
	bool Start(const std::string& yun_ip, const std::string& ip, int port, int heartbeat, CarpConnectSchedule* schedule) override
	{
		m_schedule = schedule;

		// 如果已经开启了就直接返回
		if (m_acceptor)
		{
			CARP_ERROR("Client server already started(ip: " << m_ip << ", port:" << m_port << ")");
			return false;
		}

		// 创建一个接收器，并启动
		try
		{
			if (!ip.empty())
				m_acceptor = std::make_shared<asio::ip::tcp::acceptor>(schedule->GetIOService()
				                                                       , asio::ip::tcp::endpoint(
					                                                       asio::ip::address_v4::from_string(ip), port),
				                                                       false);
			else
				m_acceptor = std::make_shared<asio::ip::tcp::acceptor>(schedule->GetIOService()
				                                                       , asio::ip::tcp::endpoint(
					                                                       asio::ip::tcp::v4(), port), false);
		}
		catch (asio::error_code& ec)
		{
			m_acceptor = AcceptorPtr();
			CARP_ERROR("ClientServer: " << ip << " start failed at port: " << port << " error: " << ec.value());
			return false;
		}

		// 创建定时器i
		m_heartbeat_timer = std::make_shared<AsioTimer>(schedule->GetIOService(), std::chrono::seconds(heartbeat));
		m_heartbeat_timer->async_wait(std::bind(&CarpConnectServer::ServerSendHeartbeat, this->shared_from_this(), std::placeholders::_1, heartbeat));

		// 开始接受
		NextAccept(0);

		m_yun_ip = yun_ip;
		m_ip = ip;
		m_port = port;

		CARP_SYSTEM("ClientServer: start succeed at " << m_ip << ":" << m_port);
		return true;
	}

	// 关闭服务器
	void Close() override
	{
		// 关闭并释放接收器
		if (m_acceptor)
		{
			m_acceptor->close();
			m_acceptor = AcceptorPtr();
		}

		// 释放定时器
		if (m_heartbeat_timer)
		{
			m_heartbeat_timer->cancel();
			m_heartbeat_timer = AsioTimerPtr();
		}

		// 关闭所有客户端连接
		auto end = m_outer_set.end();
		for (auto it = m_outer_set.begin(); it != end; ++it)
			(*it)->Close();
		m_outer_set.clear();

		CARP_SYSTEM("ClientServer: stop succeed.");
	}

private:
	// 等待下一个socket接入
	void NextAccept(int error_count)
	{
		// 检查接收器是否合法
		if (!m_acceptor) return;

		// 创建一个Socket对象
		SocketPtr socket = std::make_shared<asio::ip::tcp::socket>(m_schedule->GetIOService());
		// 开始等待接收
		m_acceptor->async_accept(*socket, std::bind(&CarpConnectServer::HandleAccept, this->shared_from_this()
			, std::placeholders::_1, socket, error_count));
	}

	// 处理新的socket接入
	void HandleAccept(const asio::error_code& ec, SocketPtr socket, int error_count) override
	{
		if (ec)
		{
			CARP_ERROR("ClientServer accept failed: " << ec.value());
			if (error_count > 100)
				Close();
			else
				NextAccept(error_count + 1);
			return;
		}
		// 设置为no delay
		socket->set_option(asio::ip::tcp::no_delay(true));

		// 处理新的socket
		HandleOuterConnect(socket);

		// 接收下一个连接
		NextAccept(0);
	}

public:
	// 获取本地的服务器ip和端口
	const std::string& GetYunIp() const override { return m_yun_ip; }
	const std::string& GetIp() const override { return m_ip; }
	int GetPort() const override { return m_port; }

private:
	AcceptorPtr m_acceptor;		// 接收器，用于接收新的socket
	std::string m_yun_ip;       // 云服务器对外的IP
	std::string m_ip;			// 本地服务器的IP
	int m_port = 0;					// 本地服务器的端口

///////////////////////////////////////////////////////////////////////////////////////////////

private:
	// 处理一个新的socket
	void HandleOuterConnect(SocketPtr socket) override
	{
		// 创建一个客户端连接
		CarpConnectReceiverPtr receiver = std::make_shared<CarpConnectReceiver>(socket, this->shared_from_this(), m_schedule);
		// 保存起来
		m_outer_set.insert(receiver);

		// 客户端连接马上开始接受数据包
		receiver->NextReadHeadFirst();

		// 通知客户端连接进来了
		m_schedule->HandleClientConnect(receiver);
	}

	// 处理某个连接断开了
	void HandleOuterDisconnected(CarpConnectReceiverPtr receiver) override
	{
		// 关闭并移除客户端连接
		receiver->Close();
		m_outer_set.erase(receiver);

		// 通知断开连接
		m_schedule->HandleClientDisconnect(receiver);
	}

public:
	// 处理消息事件
	void HandleClientMessage(CarpConnectReceiverPtr receiver, CARP_MESSAGE_SIZE message_size, CARP_MESSAGE_ID message_id, CARP_MESSAGE_RPCID message_rpcid, void* memory) override
	{
		// 通知处理消息包
		m_schedule->HandleClientMessage(receiver, message_size, message_id, message_rpcid, memory);
		// 释放内存
		if (memory) free(memory);
	}

private:
	// 保存客户端连接对象
	std::set<CarpConnectReceiverPtr> m_outer_set;	// container outer
	CarpConnectSchedule* m_schedule = nullptr;

private:
	// 服务器间隔一定时间向客户端发送心跳包
	void ServerSendHeartbeat(const asio::error_code& ec, int interval) override
	{
		// 向所有客户端发送心跳包
		HeartbeatMessage msg;
		const auto end = m_outer_set.end();
		for (auto it = m_outer_set.begin(); it != end; ++it)
			(*it)->Send(msg);

		if (!m_heartbeat_timer) return;
		m_heartbeat_timer->expires_at(std::chrono::system_clock::now() + std::chrono::seconds(interval));
		m_heartbeat_timer->async_wait(std::bind(&CarpConnectServer::ServerSendHeartbeat, this->shared_from_this(),
		                                        std::placeholders::_1, interval));
	}
	// 心跳包定时器
	AsioTimerPtr m_heartbeat_timer;
};

#endif