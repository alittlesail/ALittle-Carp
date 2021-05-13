#ifndef CARP_HTTP_SERVER_INCLUDED
#define CARP_HTTP_SERVER_INCLUDED

#include <memory>
#include <map>

#include "carp_file.hpp"
#include "carp_http.hpp"
#include "carp_log.hpp"

class CarpHttpReceiver;
typedef std::shared_ptr<CarpHttpReceiver> CarpHttpReceiverPtr;
typedef std::weak_ptr<CarpHttpReceiver> CarpHttpReceiverWeakPtr;

class CarpHttpSender;
typedef std::shared_ptr<CarpHttpSender> CarpHttpSenderPtr;
typedef std::weak_ptr<CarpHttpSender> CarpHttpSenderWeakPtr;

class CarpHttpServerInterface
{
public:
	virtual ~CarpHttpServerInterface() {};

public:
	virtual void HandleHttpMessage(CarpHttpSenderPtr sender, const std::string& msg) = 0;
	virtual bool HandleHttpFileMessage(CarpHttpSenderPtr sender, const std::string& msg) = 0;
	virtual void HandleHttpFileCompletedMessage(CarpHttpSenderPtr sender, const std::string& msg, const std::string& file_path, const std::string* reason) = 0;
	virtual void ExecuteRemoveCallBack(CarpHttpSocketPtr socket) = 0;
	virtual void SenderSendString(CarpHttpSenderPtr sender, const std::string& content) = 0;
};
typedef std::shared_ptr<CarpHttpServerInterface> CarpHttpServerInterfacePtr;
typedef std::weak_ptr<CarpHttpServerInterface> CarpHttpServerInterfaceWeakPtr;

typedef std::shared_ptr<asio::ip::tcp::acceptor> AcceptorPtr;

typedef asio::basic_waitable_timer<std::chrono::system_clock> AsioTimer;
typedef std::shared_ptr<AsioTimer> AsioTimerPtr;

class CarpHttpReceiver : public std::enable_shared_from_this<CarpHttpReceiver>
{
public:
	friend class CarpHttpServer;

public:
	CarpHttpReceiver(CarpHttpSocketPtr socket, CarpHttpServerInterfacePtr server)
	    : m_socket(socket), m_server_system(server)
	{
		memset(m_http_buffer, 0, sizeof(m_http_buffer));
	}
	virtual ~CarpHttpReceiver()
	{
		Clear();
	}

private:
	void JustWait()
	{
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpReceiver::HandleJustWait, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleJustWait(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			Close();
			return;
		}

		JustWait();
	}

private:
	void NextRead()
	{
		// update to current time
		m_receive_time = CarpTime::GetCurTime();

		// read next bytes
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpReceiver::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleRead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			Close();
			return;
		}

		// update to current time
		m_receive_time = CarpTime::GetCurTime();

		// store current size
		int current_size = static_cast<int>(m_http_head.size());
		// add to buffer
		m_http_head.append(m_http_buffer, actual_size);

		// stop receive if lager than max size
		if (m_http_head.size() > HTTP_HEAD_BUFFER_SIZE_MAX)
		{
			CARP_ERROR("HTTP head is large than " << HTTP_HEAD_BUFFER_SIZE_MAX);
			Close();
			return;
		}

		// set start point to find
		int find_start_pos = current_size - static_cast<int>(strlen("\r\n\r\n"));
		if (find_start_pos < 0) find_start_pos = 0;

		// find \r\n\r\n
		std::string::size_type find_pos = m_http_head.find("\r\n\r\n", find_start_pos);
		if (find_pos != std::string::npos)
		{
			// the end position of \r\n\r\n in m_http_head
			int head_size = static_cast<int>(find_pos) + static_cast<int>(strlen("\r\n\r\n"));
			// resize http size, delete other data
			m_http_head.resize(head_size);

			// pos content is then count of bytes after current size
			int receive_size = head_size - current_size;
			// calc content size
			int last_size = static_cast<int>(actual_size) - receive_size;

			// handle GET
			if (m_http_head.substr(0, 3) == "GET")
			{
				// receive completed and do not affect by heart beat
				m_receive_time = 0;

				// handle GET message
				CarpHttpServerInterfacePtr server = m_server_system.lock();
				if (server)	server->HandleHttpMessage(m_sender, m_http_head);

				// read next to check sender is disconnected
				JustWait();
				return;
			}

			// handle POST
			if (m_http_head.substr(0, 4) == "POST")
			{
				// get content size from head
				CarpHttp::ResponseType response_type = CarpHttp::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH;
				if (!CarpHttp::CalcFileSizeFromHttp(m_http_head, m_receive_size, response_type)
					|| response_type != CarpHttp::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH)
				{
					CARP_ERROR("can't find Content-Length: in http head:" << m_http_head);
					Close();
					return;
				}

				// get content type from head
				std::string content_type;
				if (!CarpHttp::CalcContentTypeFromHttp(m_http_head, content_type))
				{
					CARP_ERROR("can't find Content-Type: in http head:" << m_http_head);
					Close();
					return;
				}

				// check content type, only support below
				if (content_type.find("application/x-www-form-urlencoded") != std::string::npos
					|| content_type.find("text/xml") != std::string::npos
					|| content_type.find("text/plain") != std::string::npos
					|| content_type.find("application/json") != std::string::npos)
				{
					// check is completed
					if (last_size > 0)
					{
						// adjust buffer
						for (int i = 0; i < last_size; ++i)
							m_http_buffer[i] = m_http_buffer[receive_size + i];

						// handle buffer
						asio::error_code ec;
						HandleReadPost(ec, last_size);
						return;
					}

					// read next
					NextReadPost();
					return;
				}

				if (content_type.find("multipart/form-data") != std::string::npos)
				{
					// get boundary
					size_t boundary_pos = content_type.find("boundary=");
					if (boundary_pos == std::string::npos)
					{
						CARP_ERROR("can't find boundary in Content-Type:" << m_http_head);
						Close();
						return;
					}
					boundary_pos += strlen("boundary=");
					// dec boundary size at last line, \r\n--boundary--\r\n
					m_receive_size -= static_cast<int>(content_type.size()) - static_cast<int>(boundary_pos) + 8;

					// ready to read boundary
					m_boundary_or_file = true;

					// get file path to save
					CarpHttpServerInterfacePtr server = m_server_system.lock();
					if (!server)
					{
						Close();
						return;
					}

					// save last size
					m_last_size_of_http_buffer = last_size;

					// check is completed
					if (last_size > 0)
					{
						// adjust buffer
						for (int i = 0; i < last_size; ++i)
							m_http_buffer[i] = m_http_buffer[receive_size + i];
					}

					bool result = server->HandleHttpFileMessage(m_sender, m_http_head);
					if (result == false)
						Close();

					return;
				}

				CARP_ERROR("unknow Content-Type: in http head:" << m_http_head);
				Close();
				return;
			}

			// handle option
			if (m_http_head.substr(0, 7) == "OPTIONS")
			{
				// receive completed and do not affect by heart beat
				m_receive_time = 0;

				// send empty message
				CarpHttpServerInterfacePtr server = m_server_system.lock();
				if (server) server->SenderSendString(m_sender, "");

				// read next to check sender is disconnected
				JustWait();
				return;
			}

			CARP_ERROR("unknow http method!" << m_http_head);

			Close();
			return;
		}

		// read next
		NextRead();
	}

private:
	void NextReadFile()
	{
		// update to current time
		m_receive_time = CarpTime::GetCurTime();

		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpReceiver::HandleReadFile, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadFile(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			// check file is open or not
			if (m_file)
			{
				fclose(m_file);
				m_file = nullptr;

				// receive completed and do not affect by heart beat
				m_receive_time = 0;

				std::string reason = "HttpReceiver::HandleReadFile failed:" + std::to_string(ec.value());
				CarpHttpServerInterfacePtr server = m_server_system.lock();
				if (server) server->HandleHttpFileCompletedMessage(m_sender, m_http_head, m_file_path, &reason);
			}
			Close();
			return;
		}

		// update current time
		m_receive_time = CarpTime::GetCurTime();

		// if last size lager than 0, then handle new bytes
		if (m_receive_size > 0)
		{
			if (m_boundary_or_file)
			{
				// store current size
				int current_size = static_cast<int>(m_boundary_temp.size());
				// add to buffer
				m_boundary_temp.append(m_http_buffer, actual_size);

				// stop receive if lager than max size
				if (m_boundary_temp.size() > HTTPS_RECEIVE_FILE_SIZE_MAX)
				{
					std::string reason = "HTTP m_boundary_temp is large than HTTPS_RECEIVE_FILE_SIZE_MAX:" + std::to_string(HTTPS_RECEIVE_FILE_SIZE_MAX);
					CarpHttpServerInterfacePtr server = m_server_system.lock();
					if (server) server->HandleHttpFileCompletedMessage(m_sender, m_http_head, m_file_path, &reason);
					Close();
					return;
				}

				// set start point to find
				int find_start_pos = current_size - static_cast<int>(strlen("\r\n\r\n"));
				if (find_start_pos < 0) find_start_pos = 0;

				// find \r\n\r\n
				const auto find_pos = m_boundary_temp.find("\r\n\r\n", find_start_pos);
				if (find_pos != std::string::npos)
				{
					// the end position of \r\n\r\n in m_boundary_temp
					const int head_size = static_cast<int>(find_pos) + static_cast<int>(strlen("\r\n\r\n"));

					// pos content is then count of bytes after current size
					const int receive_size = head_size - current_size;
					// calc content size
					const int last_size = static_cast<int>(actual_size) - receive_size;

					// dec receive_size
					m_receive_size -= receive_size;

					// ready to receive file content
					m_boundary_or_file = false;

					if (last_size > 0)
					{
						// adjust buffer
						for (int i = 0; i < last_size; ++i)
							m_http_buffer[i] = m_http_buffer[receive_size + i];

						// handle file
						HandleReadFile(asio::error_code(), last_size);
						return;
					}
				}
			}
			else
			{
				// write to file
				if (m_receive_size < static_cast<int>(actual_size))
					fwrite(m_http_buffer, 1, m_receive_size, m_file);
				else
					fwrite(m_http_buffer, 1, actual_size, m_file);

				// dec content size that is received now
				m_receive_size -= static_cast<int>(actual_size);

				// check is receive completed
				if (m_receive_size <= 0)
				{
					// close file
					if (m_file)
					{
						fclose(m_file);
						m_file = nullptr;
					}

					// receive completed and do not affect by heart beat
					m_receive_time = 0;

					CarpHttpServerInterfacePtr server = m_server_system.lock();
					if (server) server->HandleHttpFileCompletedMessage(m_sender, m_http_head, m_file_path, nullptr);

					JustWait();
					return;
				}
			}
		}

		// read next
		NextReadFile();
	}

private:
	void NextReadPost()
	{
		// update to current time
		m_receive_time = CarpTime::GetCurTime();

		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpReceiver::HandleReadPost, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void HandleReadPost(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			Close();
			return;
		}

		// update current time
		m_receive_time = CarpTime::GetCurTime();
		// add to buffer
		m_http_head.append(m_http_buffer, actual_size);

		// dec content size that is received now
		m_receive_size -= static_cast<int>(actual_size);

		// check is completed
		if (m_receive_size <= 0)
		{
			// receive completed and do not affect by heart beat
			m_receive_time = 0;

			// handle post message
			CarpHttpServerInterfacePtr server = m_server_system.lock();
			if (server) server->HandleHttpMessage(m_sender, m_http_head);

			JustWait();
			return;
		}

		// read next
		NextReadPost();
	}

public:
	// this func must invoke
	bool StartReceiveFile(const std::string& file_path, int start_size, std::string& reason)
	{
		if (!m_file_path.empty())
		{
			reason = "m_file_path is already set:" + m_file_path;
			return false;
		}

		m_file_path = file_path;

		// create file
		if (start_size <= 0)
		{
#ifdef _WIN32
			fopen_s(&m_file, m_file_path.c_str(), "wb");
#else
			m_file = fopen(m_file_path.c_str(), "wb");
#endif
		}
		else
		{
#ifdef _WIN32
			fopen_s(&m_file, m_file_path.c_str(), "ab");
#else
			m_file = fopen(m_file_path.c_str(), "ab");
#endif
		}
		if (!m_file)
		{
			reason = "open file failed:" + m_file_path;
			return false;
		}

		if (start_size > 0)
			fseek(m_file, start_size, SEEK_SET);

		// check is completed
		if (m_last_size_of_http_buffer > 0)
		{
			// handle buffer
			HandleReadFile(asio::error_code(), m_last_size_of_http_buffer);
			return true;
		}

		// read next
		NextReadFile();

		return true;
	}

private:
	void Clear()
	{
		m_http_head = "";
		m_receive_time = 0;
		if (m_file)
		{
			fclose(m_file);
			m_file = nullptr;
		}
	}

	void Close()
	{
		// clear buffer, reset status
		Clear();
		// save socket, this is import!(prevent count of smart point desc to 0)
		CarpHttpSocketPtr socket = m_socket;
		// remove from server
		CarpHttpServerInterfacePtr server_system = m_server_system.lock();
		if (server_system) server_system->ExecuteRemoveCallBack(socket);
	}

private:
	time_t m_receive_time = 0;			// receive time
	void Heartbeat(int second)
	{
		// 0: return
		if (m_receive_time == 0) return;

		// not wait second
		if (CarpTime::GetCurTime() - m_receive_time < second) return;

		// close
		Close();
	}

public:
	CarpHttpSenderPtr m_sender;		// sender

private:
	CarpHttpSocketPtr m_socket;			// socket
	CarpHttpServerInterfaceWeakPtr m_server_system;	// server that create this receiver

private:
	static const int HTTP_HEAD_BUFFER_SIZE_MAX = 1024;
	char m_http_buffer[HTTP_HEAD_BUFFER_SIZE_MAX];	// receive buffer
	int m_last_size_of_http_buffer = 0;						// used for file upload, when request file_path and start_size

private:
	std::string m_http_head;	// http head

private:
	static const int HTTPS_RECEIVE_FILE_SIZE_MAX = 1024 * 10000;
	bool m_boundary_or_file = false;	// post boundary
	std::string m_boundary_temp;// post boundary temp

private:
	std::string m_file_path;	// file path
	FILE* m_file = nullptr;		// file object
	int m_receive_size = 0;			// last size
};

class CarpHttpSender : public std::enable_shared_from_this<CarpHttpSender>
{
public:
	friend class CarpHttpServer;

public:
	CarpHttpSender(CarpHttpSocketPtr socket, CarpHttpServerInterfacePtr server)
		: m_socket(socket), m_server_system(server)
	{
		// save target ip
		CARPHTTPSOCKET_GetRemoteIp(socket, m_remote_ip);
		CARPHTTPSOCKET_GetRemotePort(socket, m_remote_port);
	}
	virtual ~CarpHttpSender()
	{
		Clear();
	}

private:
	static void HandleSendImpl(CarpHttpSenderPtr self, const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			CARP_INFO("error info:" << ec.value());
			self->Close();
			return;
		}

		self->HandleSend(bytes_transferred);
	}

public:
	/* send string message
	 * @param message: content to response
	 */
	void SendString(const std::string& message)
	{
		// if in sending, then return
		if (m_is_sending)
		{
			CARP_ERROR("Https Sender already in sending");
			Close();
			return;
		}

		// set current is in sending
		m_is_sending = true;

		// generate http content
		m_http_content = "";
		m_http_content += "HTTP/1.1 200 OK\r\n";
		m_http_content += "Access-Control-Allow-Origin: *\r\n";
		m_http_content += "Access-Control-Allow-Credentials: true\r\n";
		m_http_content += "Server: ALittle Https Server\r\n";
		m_http_content += "Content-Type: text/html\r\n";
		m_http_content += "Accept-Ranges: bytes\r\n";
		m_http_content = m_http_content + "Content-Length: " + std::to_string(message.size()) + "\r\n";
		m_http_content += "Connection: Close\r\n";
		m_http_content += "\r\n"; // ending code
		// add message content
		m_http_content += message;

		// send response
		CARPHTTPSOCKET_AsyncWrite(m_socket, m_http_content.c_str(), m_http_content.size(),
			std::bind(HandleSendImpl, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

public:
	/* send file
	 * @param path: the path of file to send
	 * @param content_type: type of content
	 * @param for_download: is notify browser to download
	 * @param start_size: start size of file
	 * @param use_cache: use cache or not
	 * @param show_name: show name for http
	 */
	void SendFile(const char* path, const char* content_type, bool for_download, int start_size, const char* show_name)
	{
		// if in sending, then return
		if (m_is_sending)
		{
			CARP_ERROR("Https Sender already in sending");
			Close();
			return;
		}

		// save file path
		m_file_path = path;

		// set is in sending
		m_is_sending = true;

		// open file
#ifdef _WIN32
		fopen_s(&m_file, path, "rb");
#else
		m_file = fopen(path, "rb");
#endif

		// if file is open failed, then send error response
		if (m_file == nullptr)
		{
			m_http_content = "";
			m_http_content += "HTTP/1.1 404 Not Found\r\n";
			m_http_content += "Access-Control-Allow-Origin: *\r\n";
			m_http_content += "Access-Control-Allow-Credentials: true\r\n";
			m_http_content += "Server: ALittle Https Server\r\n";
			m_http_content += "Connection: Close\r\n";
			m_http_content += "Content-Type: text/html\r\n";
			m_http_content += "Content-Length: 0\r\n";
			m_http_content += "\r\n";

			// send
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_http_content.c_str(), m_http_content.size(),
				std::bind(HandleSendImpl, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

			return;
		}

		// calc file size
		fseek(m_file, 0, SEEK_END);
		int size = static_cast<int>(ftell(m_file));
		fseek(m_file, 0, SEEK_SET);

		if (start_size > size)
		{
			CARP_ERROR("Https Sender start size:(" << start_size << ") is large than file size:" << size);
			Close();
			return;
		}

		if (start_size > 0)
		{
			fseek(m_file, start_size, SEEK_SET);
			size -= start_size;
		}

		// create buffer
		if (size < HTTPS_SEND_FILE_BUFFER_SIZE)
			m_file_buffer.resize(size);
		else
			m_file_buffer.resize(HTTPS_SEND_FILE_BUFFER_SIZE);

		// generate http response
		m_http_content = "";
		m_http_content += "HTTP/1.1 200 OK\r\n";
		m_http_content += "Access-Control-Allow-Origin: *\r\n";
		m_http_content += "Access-Control-Allow-Credentials: true\r\n";
		m_http_content += "Server: ALittle Https Server\r\n";
		if (for_download)
		{
			m_http_content += "Content-Disposition: attachment;filename=";
			if (show_name && strlen(show_name) > 0)
				m_http_content += CarpHttp::UrlEncode(show_name);
			else
				m_http_content += CarpHttp::UrlEncode(CarpFile::GetFileNameByPath(path));
			m_http_content += "\r\n";
		}
		if (content_type == 0 || content_type[0] == 0)
			m_http_content += "Content-Type: application/octet-stream\r\n";
		else
			m_http_content.append("Content-Type: ").append(content_type).append("\r\n");
		m_http_content += "Accept-Ranges: bytes\r\n";
		m_http_content += "Connection: Close\r\n";
		m_http_content += "Content-Length: " + std::to_string(size) + "\r\n";
		m_http_content += "\r\n"; // ending code

		// send
		CARPHTTPSOCKET_AsyncWrite(m_socket, m_http_content.c_str(), m_http_content.size(),
			std::bind(HandleSendImpl, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	void Close()
	{
		// clear buffer, reset status
		Clear();
		// save socket, this is import!(prevent count of smart point desc to 0)
		CarpHttpSocketPtr socket = m_socket;
		// remove from server
		CarpHttpServerInterfacePtr server_system = m_server_system.lock();
		if (server_system) server_system->ExecuteRemoveCallBack(socket);

		// send completed do not affect by heart beat
		m_end_time = 0;
	}

public:
	bool StartReceiveFile(const std::string& file_path, int start_size, std::string& reason)
	{
		CarpHttpReceiverPtr receiver = m_receiver.lock();
		if (!receiver)
		{
			reason = u8"Http请求方已经断开";
			return false;
		}

		return receiver->StartReceiveFile(file_path, start_size, reason);
	}

private:
	void Clear()
	{
		if (m_file)
		{
			fclose(m_file);
			m_file = nullptr;
		}
		m_http_content = "";
	}

public:
	const std::string& GetRemoteIP() const { return m_remote_ip; }
	int GetRemotePort() const { return m_remote_port; }

public:
	void HandleSend(std::size_t bytes_transferred)
	{
		m_http_content = "";

		// check is sending file
		if (m_file)
		{
			// read file
			int size = static_cast<int>(fread(&(m_file_buffer[0]), 1, m_file_buffer.size(), m_file));
			// if completed
			if (size == 0)
			{
				// close file
				fclose(m_file);
				m_file = nullptr;
				// send completed do not affect by heart beat
				m_end_time = CarpTime::GetCurTime();
			}
			else
			{
				// send file content
				CARPHTTPSOCKET_AsyncWrite(m_socket, &(m_file_buffer[0]), size,
					std::bind(HandleSendImpl, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			}
		}
		else
		{
			// send completed do not affect by heart beat
			m_end_time = CarpTime::GetCurTime();
		}
	}

public:
	void* GetSocket() const { return m_socket.get(); }
	CarpHttpSocketPtr GetSocketPtr() const { return m_socket; }

private:
	CarpHttpSocketPtr m_socket;			// socket

	std::string m_remote_ip;			// ip dress
	int m_remote_port = 0;

public:
	void* m_user_data = nullptr;
	int m_id = 0;

private:
	static const int HTTPS_SEND_FILE_BUFFER_SIZE = 1024 * 64;
	std::string m_http_content;	// send http response

	// file info
	FILE* m_file = nullptr;
	std::string m_file_path;
	std::vector<char> m_file_buffer;

private:
	// store last send time
	time_t m_end_time = 0;
	void Heartbeat(int second)
	{
		// 0: return
		if (m_end_time == 0) return;

		// not wait second
		if (CarpTime::GetCurTime() - m_end_time < second) return;

		// close
		Close();
	}

public:
	bool IsSending() const { return m_is_sending; }
	bool IsRemoved() const { return m_is_removed; }

private:
	bool m_is_sending = false;
	bool m_is_removed = false;

private:
	CarpHttpServerInterfaceWeakPtr m_server_system;

public:
	CarpHttpReceiverWeakPtr m_receiver;
};

class CarpHttpServer : public std::enable_shared_from_this<CarpHttpServer>, public CarpHttpServerInterface
{
public:
	CarpHttpServer()
#ifdef CARP_HAS_SSL
		: m_context(asio::ssl::context::sslv23)
#endif
	{}
	virtual ~CarpHttpServer() { Close(); }

public:
	friend CarpHttpReceiver;
	friend CarpHttpSender;

public:
	/* start server
	 * @param ip: local ip dress
	 * @param port: port
	 * @param heartbeat: interval to check dead socket
	 * @param is_ssl: is ssl
	 * @param server_pem_path: pem
	 * @param private_key_path: pem
	 * @param pem_password: password of pem
	 */
	bool Start(const std::string& yun_ip, const std::string& ip, unsigned int port, int heartbeat, bool is_ssl
		// asio
		, asio::io_service* io_service
		// sender, msg
		, std::function<void(CarpHttpSenderPtr, const std::string&)> http_message_func
	    // sender, msg
	    // return receive or not
	    , std::function<bool(CarpHttpSenderPtr, const std::string&)> http_file_func
	    // sender, file_path, succeed or not, reason
	    , std::function<void(CarpHttpSenderPtr, const std::string&, bool, const std::string&)> http_file_completed_func
		// ssl
		, const std::string& server_pem_path = "", const std::string& private_key_path = "", const std::string& pem_password = "")
	{
		m_io_service = io_service;
		m_http_message_func = http_message_func;
		m_http_file_func = http_file_func;
		m_http_file_completed_func = http_file_completed_func;

		// check is already started
		if (m_acceptor)
		{
			CARP_ERROR("http server already started(ip: " << m_ip << ", port:" << m_port << ")");
			return false;
		}

		try
		{
#ifdef CARP_HAS_SSL
			if (is_ssl)
			{
				if (!server_pem_path.empty())
					m_pem_password = pem_password;
				else
					m_pem_password = "test";

				m_context.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
				m_context.set_password_callback(std::bind(&CarpHttpServer::GetPemPassword, shared_from_this()));

				// if pem_path is empty
				if (!server_pem_path.empty())
				{
					// init context
					m_context.use_certificate_chain_file(server_pem_path);
					m_context.use_private_key_file(private_key_path, asio::ssl::context::pem);
				}
				else
				{
					// init context
					std::string dh512_pem = "Diffie-Hellman-Parameters: (512 bit)\n"
						"    prime:\n"
						"        00:a0:bc:d0:c2:c3:a8:c7:a5:62:13:cd:f0:63:39:\n"
						"        ea:85:e0:f7:ca:00:8a:57:ec:12:dd:92:2b:20:70:\n"
						"        38:6c:03:60:b1:19:e3:0b:e9:d3:05:f1:1b:cc:8c:\n"
						"        9c:1a:30:3e:91:de:db:17:a7:19:cf:da:a0:2d:ee:\n"
						"        ef:35:95:8b:4b\n"
						"    generator: 5 (0x5)\n"
						"-----BEGIN DH PARAMETERS-----\n"
						"MEYCQQCgvNDCw6jHpWITzfBjOeqF4PfKAIpX7BLdkisgcDhsA2CxGeML6dMF8RvM\n"
						"jJwaMD6R3tsXpxnP2qAt7u81lYtLAgEF\n"
						"-----END DH PARAMETERS-----";
					std::string server_pem = "-----BEGIN CERTIFICATE-----\n"
						"MIIB/jCCAWcCCQDlADUqOr8YCTANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJB\n"
						"VTEMMAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxDTALBgNVBAoTBGFzaW8w\n"
						"HhcNMDUxMTAyMTk1MjMzWhcNMTUxMTAyMTk1MjMzWjBMMQswCQYDVQQGEwJBVTEM\n"
						"MAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxDTALBgNVBAoTBGFzaW8xDzAN\n"
						"BgNVBAsTBnNlcnZlcjCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAxAkg4iw5\n"
						"X8pXhJ8GfjGRKVOClt/EI9ypQMqWQSRH0pGDFlIOQzsfhqsCyp/xcfCwPBvfMIeE\n"
						"IzYiZ5kH6YD9EfHmIA6hsd1i3layCpeiA2VXrUXsvL8Y3fUt1/3IAR+pmVI4XGas\n"
						"lCb0zs1n0K08v9AWBkVE5rirA5an263Sd+kCAwEAATANBgkqhkiG9w0BAQUFAAOB\n"
						"gQBzFX/efrhGVzfK4Ue+/7Og44r7OLzQepSyQavFhp+t3PX1hy4ifneTnqMyME8w\n"
						"WuB94G/gtST9ECVHRKUuBn4xT1rz5DO20h3VSAzTirkSFQPdWunyBbIva0Hsf6pF\n"
						"287CA1cM106X0Vs4dv2F2u0zSszYfOysAM1pIPcxdyboXA==\n"
						"-----END CERTIFICATE-----\n"
						"-----BEGIN RSA PRIVATE KEY-----\n"
						"Proc-Type: 4,ENCRYPTED\n"
						"DEK-Info: DES-EDE3-CBC,9A7CF9C13224C492\n"
						"\n"
						"w00sJ2/d79LRI+9LRsnQkBZwIo/NbprFtN3SVqcUAtncqowl9BnKZnQ2csnj8KZA\n"
						"STAL+PZAyJQTiJfJxecCkB8Tu4/apFe2V9/PxUirJzGtJ9FHBAjLgmpK4yWwSCMq\n"
						"txyy8hjm8ggR8SQNJ1Jf+O6uGotillq02TJ0RluRrhukkfevClXTfCl3ngGI3D1q\n"
						"v5dFZrU2r2pGpkxCxI8SoQCCaH6l/plR9IhKl1/cz9chiXUFmAb4kMazneZaFmiu\n"
						"B8AJMQcFDfKttdKmkxY1qdQI9bOKw2YSZT6aASZTHD84hDloQRdgok8VO1YBKfzj\n"
						"0jEYnaYuXZGD+h38giABPV0iTY7aC0RdcyP63ZQH95KXtSLGBfUHEOvR4jR4uLu9\n"
						"3Qfw4j1/HzmCrXVZNyGswwagVuVcvmBuHwGmnxMaWCGIC1CODf8X5BG4V932Akpl\n"
						"mCeQyLdkXHehZvSV1OCwxGko0FjVwnKIOplyMqCVpeDNwuddH02zo5h+kBSrTg0m\n"
						"XEMlVMAadS9vgwLCLL1GKeVpYT3jNbgDfUrXC6udycgGdYA6QtmeuiZ3Lv37TQ8K\n"
						"LD7ono+XsT38XvlKRqbPrlHNae+PRLudKWHsgrZg2MckyEpG/x9dygru561p1Dns\n"
						"Bm17CVkQUIZyAPXQB6/UmxPbJYsChiQXxy3/4U2eZLVVkHRP7gPWBnFjNoyd5OBw\n"
						"G+psOVLNgCnFh+z4NO5CB4mVNtrR1NAH6IFhnlrip4YFRk3XPHVlkrxn6fHeEDGE\n"
						"eVB3XJcgsGnVQCvF5vsymZWZ722xgLPkK8iG3QLayoM4c9RlrKMwwA==\n"
						"-----END RSA PRIVATE KEY-----\n"
						"-----BEGIN CERTIFICATE-----\n"
						"MIIB7TCCAVYCCQCxKhAUH1ygCDANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJB\n"
						"VTEMMAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxDTALBgNVBAoTBGFzaW8w\n"
						"HhcNMDUxMTAyMTk0ODU1WhcNMTUxMTAyMTk0ODU1WjA7MQswCQYDVQQGEwJBVTEM\n"
						"MAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxDTALBgNVBAoTBGFzaW8wgZ8w\n"
						"DQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAMkNbM2RjFdm48Wy1nBA3+exfJL5fR6H\n"
						"guRFu/7MKf6pQZGRqhzxIWYRoYQDx16BggHwqFVVls5hgoQF0fUqoHfE8MLGwr6m\n"
						"T6rIYBrIAGnH8eMhfwMNy4I0emkoWI+grEXlw54IUSijh8LokLWorElyGuPmxhn3\n"
						"IgZkgGe8dCQTAgMBAAEwDQYJKoZIhvcNAQEFBQADgYEAdpHw+r3X4NwzKn9nQs3h\n"
						"mQK2WeH6DVQ1r7fWqEq1Lq10qBdobbjDRE9jpezWdGMThbYtle6/8wHUJeq189PR\n"
						"XwZWyRvnfcI+pqX832yNRh24Ujwuv3wlx3JOVByybCoJc05N1THaHo0Q7j//8HsX\n"
						"VS/RFHuq3muy47cV9gbsCIw=\n"
						"-----END CERTIFICATE-----";

					m_context.use_certificate_chain(asio::buffer(server_pem.c_str(), server_pem.size()));
					m_context.use_private_key(asio::buffer(server_pem.c_str(), server_pem.size()), asio::ssl::context::pem);
					m_context.use_tmp_dh(asio::buffer(dh512_pem.c_str(), dh512_pem.size()));
				}
			}
#endif

			// create acceptor
			if (!ip.empty())
				m_acceptor = std::make_shared<asio::ip::tcp::acceptor>(*m_io_service
					, asio::ip::tcp::endpoint(
						asio::ip::address_v4::from_string(ip), port),
					false);
			else
				m_acceptor = std::make_shared<asio::ip::tcp::acceptor>(*m_io_service
					, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port),
					false);
		}
		catch (asio::error_code& ec)
		{
			m_acceptor = AcceptorPtr();
			CARP_SYSTEM("HttpServer: start failed at " << ip << ":" << port << " error: " << ec.value());
			return false;
		}

		// save info
		m_yun_ip = yun_ip;
		m_ip = ip;
		m_port = port;
		m_heartbeat_interval = heartbeat;
		m_is_ssl = is_ssl;

		// start timer for heartbeat
		m_heartbeat_timer = std::make_shared<AsioTimer>(*m_io_service,
			std::chrono::seconds(m_heartbeat_interval));
		m_heartbeat_timer->async_wait(std::bind(&CarpHttpServer::ServerSendHeartbeat, shared_from_this(), std::placeholders::_1));

		// start to accept next connect
		NextAccept(0);

		CARP_SYSTEM("HttpServer: start succeed at " << ip << ":" << port);
		return true;
	}

	/* close server
	 */
	void Close()
	{
		if (!m_acceptor) return;

		// reset
#ifdef CARP_HAS_SSL
		m_context = asio::ssl::context(asio::ssl::context::sslv23);
#endif

		// close accept
		m_acceptor->close();

		// release acceptor
		m_acceptor = AcceptorPtr();

		// release timer
		m_heartbeat_timer = AsioTimerPtr();

		// clear all sockets
		asio::error_code ec;
		const auto receiver_end = m_receiver_socket_map.end();
		for (auto receiver_it = m_receiver_socket_map.begin(); receiver_it != receiver_end; ++receiver_it)
			CARPHTTPSOCKET_Close(receiver_it->first);

		m_receiver_socket_map.clear();
		m_sender_socket_map.clear();

		CARP_SYSTEM("HttpServer stop succeed.");
	}

private:
	void CloseClient(CarpHttpSocketPtr socket)
	{
		if (!socket) return;
		// close client
		CARPHTTPSOCKET_Close(socket);

		// remove receiver
		m_receiver_socket_map.erase(socket);
		// remove sender
		const auto it = m_sender_socket_map.find(socket);
		if (it != m_sender_socket_map.end())
		{
			it->second->m_is_removed = true;
			m_sender_socket_map.erase(it);
		}
		// CARP_INFO("HttpServer ##CLOSE, socket count : " << m_receiver_socket_map.size());
	}

	void ExecuteRemoveCallBack(CarpHttpSocketPtr socket) override
	{
		// if socket is not exist, than is close by self, no need to invoke callback
		const auto it = m_sender_socket_map.find(socket);
		if (it == m_sender_socket_map.end()) return;

		// invoke callback
		// m_schedule->HandleRemoveClient(it->second);

		// close socket
		CloseClient(socket);
	}

private:
	void NextAccept(int error_count)
	{
		// check acceptor
		if (!m_acceptor) return;

		// create socket
		CarpHttpSocketPtr socket = std::make_shared<CarpHttpSocket>(m_is_ssl, m_io_service
#ifdef CARP_HAS_SSL
			, &m_context
#endif
			);

		// bind callback
		CARPHTTPSOCKET_AsyncAccept(socket, m_acceptor, std::bind(&CarpHttpServer::HandleAccept, shared_from_this(), socket, std::placeholders::_1, error_count));
	}

	void HandleAccept(CarpHttpSocketPtr socket, const asio::error_code& ec, int error_count)
	{
		if (ec)
		{
			CARP_ERROR("https server accept failed: " << ec.value());
			if (error_count > 100)
				Close();
			else
				NextAccept(error_count + 1);
			return;
		}

		// set no delay
		CARPHTTPSOCKET_SetNoDelay(socket);
#ifdef CARP_HAS_SSL
		if (socket->ssl_socket)
			socket->ssl_socket->async_handshake(asio::ssl::stream<asio::ip::tcp::socket>::server
				, std::bind(&CarpHttpServer::HandleHandShake, shared_from_this(), socket, std::placeholders::_1));
		else
#endif
			HandleHandShake(socket, asio::error_code());

		// accept next
		NextAccept(0);
	}

	void HandleHandShake(CarpHttpSocketPtr socket, const asio::error_code& ec)
	{
		if (ec)
		{
			CARP_ERROR("server hand shake failed: " << ec.value());
			return;
		}

		// create receiver
		auto receiver = std::make_shared<CarpHttpReceiver>(socket, shared_from_this());
		// save receiver
		m_receiver_socket_map[socket] = receiver;

		// create sender
		auto sender = std::make_shared<CarpHttpSender>(socket, shared_from_this());
		// save sender
		m_sender_socket_map[socket] = sender;

		receiver->m_sender = sender;
		sender->m_receiver = receiver;

		// receiver start to read next
		receiver->NextRead();

		// invoke callback
		// m_schedule->HandleAddClient(sender);
	}

public:
	// receive container
	typedef std::map<CarpHttpSocketPtr, CarpHttpReceiverPtr> SocketReceiverMap;
	// sender container
	typedef std::map<CarpHttpSocketPtr, CarpHttpSenderPtr> SocketSenderMap;

private:
	SocketReceiverMap m_receiver_socket_map;
	SocketSenderMap m_sender_socket_map;

public:
	/* get all sender
	 * @return all sender
	 */
	SocketSenderMap& GetAllSender() { return m_sender_socket_map; }
	/* get http receiver count
	 * @return count
	 */
	size_t GetReceiverCount() const { return m_receiver_socket_map.size(); }

	void SenderSendString(CarpHttpSenderPtr sender, const std::string& content) override { sender->SendString(content); }

	const std::string& GetYunIp() const { return m_yun_ip; }
	const std::string& GetIp() const { return m_ip; }
	bool IsSSL() const { return m_is_ssl; }
	unsigned int GetPort() const { return m_port; }

private:
	const std::string& GetPemPassword() const { return m_pem_password; }

private:
	AcceptorPtr m_acceptor;
#ifdef CARP_HAS_SSL
	asio::ssl::context m_context;
#endif
	asio::io_service* m_io_service = nullptr;
	bool m_is_ssl = false;
	std::string m_pem_password;
	std::string m_yun_ip;
	std::string m_ip;
	unsigned int m_port = 0;

private:
	int m_heartbeat_interval = 30;
	AsioTimerPtr m_heartbeat_timer;

	void ServerSendHeartbeat(const asio::error_code& ec)
	{
		{
			std::list<CarpHttpSenderPtr> sender_list;
			const auto map_end = m_sender_socket_map.end();
			for (auto map_it = m_sender_socket_map.begin(); map_it != map_end; ++map_it)
				sender_list.push_back(map_it->second);

			const auto list_end = sender_list.end();
			for (auto list_it = sender_list.begin(); list_it != list_end; ++list_it)
				(*list_it)->Heartbeat(m_heartbeat_interval);
		}

		{
			std::list<CarpHttpReceiverPtr> receiver_list;
			const auto map_end = m_receiver_socket_map.end();
			for (auto map_it = m_receiver_socket_map.begin(); map_it != map_end; ++map_it)
				receiver_list.push_back(map_it->second);

			const auto list_end = receiver_list.end();
			for (auto list_it = receiver_list.begin(); list_it != list_end; ++list_it)
				(*list_it)->Heartbeat(m_heartbeat_interval);
		}

		if (!m_heartbeat_timer) return;
		m_heartbeat_timer->expires_at(std::chrono::system_clock::now() + std::chrono::seconds(m_heartbeat_interval));
		m_heartbeat_timer->async_wait(std::bind(&CarpHttpServer::ServerSendHeartbeat, shared_from_this(), std::placeholders::_1));
	}

private:
	// handle http message
	void HandleHttpMessage(CarpHttpSenderPtr sender, const std::string& msg) override
	{
		if (m_http_message_func) m_http_message_func(sender, msg);
	}
	// handle http file message
	bool HandleHttpFileMessage(CarpHttpSenderPtr sender, const std::string& msg) override
	{
		if (m_http_file_func) return m_http_file_func(sender, msg);
		return false;
	}
	// handle http file completed message
	void HandleHttpFileCompletedMessage(CarpHttpSenderPtr sender, const std::string& msg, const std::string& file_path, const std::string* reason) override
	{
		if (m_http_file_completed_func)
		{
			if (reason == nullptr)
				m_http_file_completed_func(sender, file_path, true, "");
			else
				m_http_file_completed_func(sender, file_path, false, *reason);
		}
	}

private:
	// sender, msg
	std::function<void(CarpHttpSenderPtr, const std::string&)> m_http_message_func;
	// sender, msg
	// return receive or not
	std::function<bool(CarpHttpSenderPtr, const std::string&)> m_http_file_func;
	// sender, file_path, succeed or not, reason
	std::function<void(CarpHttpSenderPtr, const std::string&, bool, const std::string&)> m_http_file_completed_func;
};
typedef std::shared_ptr<CarpHttpServer> CarpHttpServerPtr;
typedef std::weak_ptr<CarpHttpServer> CarpHttpServerWeakPtr;

#endif