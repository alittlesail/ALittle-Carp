#ifndef CARP_HTTP_INCLUDED
#define CARP_HTTP_INCLUDED

#include <string>
#include <map>
#include <asio.hpp>
#include <string>
#include <memory>
#include <fstream>

class CarpHttpHelper
{
public:
	enum class ResponseType
	{
		RESPONSE_TYPE_CONTENT_LENGTH,
		RESPONSE_TYPE_CHUNK,
		RESPONSE_TYPE_DATA_FOLLOW
	};

	// 把十六进制数转成数值，如果出现非法字符就返回false
	static bool String2HexNumber(int& result, const std::string& content)
	{
		result = 0;
		for (size_t i = 0; i < content.size(); ++i)
		{
			auto num = 0;
			if (content[i] >= '0' && content[i] <= '9')
				num = content[i] - '0';
			else if (content[i] >= 'a' && content[i] <= 'f')
				num = content[i] - 'a' + 10;
			else if (content[i] >= 'A' && content[i] <= 'F')
				num = content[i] - 'A' + 10;
			else
				return false;

			result = result * 16 + num;
		}

		return true;
	}

	// 删除左边的空格
	static void TrimLeft(std::string& target)
	{
		const auto pos = target.find_first_not_of(' ');
		if (pos == std::string::npos) return;
		if (pos == 0) return;

		target = target.substr(pos);
	}

	// 删除右边的空格
	static void TrimRight(std::string& target)
	{
		const auto pos = target.find_last_not_of(' ');
		if (pos == std::string::npos) return;
		if (pos + 1 == target.size()) return;

		target = target.substr(0, pos + 1);
	}

	// 改为大写
	static void UpperString(std::string& target)
	{
		for (size_t i = 0; i < target.size(); ++i)
		{
			char c = target[i];
			if (c >= 'a' && c <= 'z')
			{
				c -= 'a' - 'A';
				target[i] = c;
			}
		}
	}

public:
	// url加密
	static std::string UrlEncode(const std::string& url)
	{
		static char hex[] = "0123456789ABCDEF";
		std::string dst;

		for (size_t i = 0; i < url.size(); ++i)
		{
			const unsigned char cc = url[i];
			if (isascii(cc))
			{
				if (cc == ' ')
					dst += "%20";
				else
					dst += cc;
			}
			else
			{
				dst += '%';
				dst += hex[cc / 16];
				dst += hex[cc % 16];
			}
		}
		return dst;
	}
	// url解密
	static std::string UrlDecode(const std::string& url)
	{
		std::string result;
		for (size_t i = 0; i < url.size(); ++i)
		{
			switch (url[i])
			{
			case '+': result += ' ';
				break;
			case '%':
				if (isxdigit(url[i + 1]) && isxdigit(url[i + 2]))
				{
					auto hex_str = url.substr(i + 1, 2);
					const int hex = strtol(hex_str.c_str(), nullptr, 16);
					if (!((hex >= 48 && hex <= 57) ||
						(hex >= 97 && hex <= 122) ||
						(hex >= 65 && hex <= 90) ||
						hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
						|| hex == 0x2a || hex == 0x2b || hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
						|| hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
						))
					{
						result += static_cast<char>(hex);
						i += 2;
					}
					else result += '%';
				}
				else {
					result += '%';
				}

				break;
			default: result += url[i];
				break;
			}
		}

		return result;
	}

public:
	// 生成Get请求头
	static void GenerateGetRequestHead(const std::string& domain, const std::string& path, std::string& result)
	{
		result = "";
		if (!path.empty())
			result.append("GET ").append(UrlEncode(path)).append(" HTTP/1.1\r\n");
		else
			result.append("GET / HTTP/1.1\r\n");
		result.append("Accept: */*\r\n");
		result.append("User-Agent: ALittle Client\r\n");
		result.append("Host: ").append(domain).append("\r\n");

		result.append("Connection: Close\r\n");
		result.append("\r\n");
	}
	// 生成Post请求头
	static void GeneratePostRequestHead(const std::string& domain, const std::string& path, const std::string& content, std::string& result)
	{
		result = "";
		if (!path.empty())
			result.append("POST ").append(UrlEncode(path)).append(" HTTP/1.1\r\n");
		else
			result.append("POST / HTTP/1.1\r\n");
		result.append("Accept: */*\r\n");
		result.append("User-Agent: ALittle Client\r\n");
		result.append("Host: ").append(domain).append("\r\n");
		result.append("Content-Type: application/x-www-form-urlencoded\r\n");

		char text[32] = { 0 };
#ifdef _WIN32
		sprintf_s(text, "%zd", content.size());
#else
		sprintf(text, "%zd", content.size());
#endif
		result.append("Content-Length: ").append(text).append("\r\n");

		result.append("Connection: Close\r\n");
		result.append("\r\n");
		result.append(content);
	}

public:
	static bool AnalysisRequest(const std::string& request, std::string& url, std::string& method, std::string& path, std::string* param, std::string* content_type, std::string* content)
	{
		const auto method_end_pos = request.find(' ');
		if (method_end_pos == std::string::npos)
			return false;

		method = request.substr(0, method_end_pos);

		const auto url_end_pos = request.find(' ', method_end_pos + 1);
		if (url_end_pos == std::string::npos)
			return false;

		url = UrlDecode(request.substr(method_end_pos + 1, url_end_pos - method_end_pos - 1));

		const auto path_end_pos = url.find('?');
		if (path_end_pos != std::string::npos)
		{
			path = url.substr(0, path_end_pos);
			if (param) *param = url.substr(path_end_pos + 1);
		}
		else
			path = url;

		auto content_pos = request.find("\r\n\r\n");

		if (content_type)
			CalcContentTypeFromHttp(request.substr(0, content_pos), *content_type);

		if (content)
		{
			if (content_pos != std::string::npos)
			{
				content_pos += strlen("\r\n\r\n");
				*content = request.substr(content_pos);
			}
		}
		return true;
	}

public:
	// 从http头获取status
	static bool CalcStatusFromHttp(const std::string& response, std::string& status)
	{
		status = "";
		auto pos = response.find("HTTP/");
		if (pos == std::string::npos) return false;

		pos = response.find(' ', pos);

		++pos;

		const auto pos_end = response.find(' ', pos);
		if (pos_end == std::string::npos) return false;

		status = response.substr(pos, pos_end - pos);

		return status.size() != 0;
	}
	// 从http头获取文件大小
	static bool CalcFileSizeFromHttp(const std::string& response, int& length, ResponseType& type)
	{
		auto new_response = response;
		UpperString(new_response);

		length = 0;
		std::string::size_type content_length_pos = new_response.find("CONTENT-LENGTH:");
		if (content_length_pos != std::string::npos)
		{
			const auto content_length_pos_end = new_response.find("\r\n", content_length_pos);
			if (content_length_pos_end == std::string::npos)
				return false;

			content_length_pos += strlen("CONTENT-LENGTH:");
			std::string content = response.substr(content_length_pos, content_length_pos_end - content_length_pos);
			TrimRight(content); TrimLeft(content);
			length = atoi(content.c_str());

			type = ResponseType::RESPONSE_TYPE_CONTENT_LENGTH;
			return true;
		}

		const auto content_chunk_pos = new_response.find("CHUNKED");
		if (content_chunk_pos != std::string::npos)
		{
			type = ResponseType::RESPONSE_TYPE_CHUNK;
			return true;
		}

		type = ResponseType::RESPONSE_TYPE_DATA_FOLLOW;
		return true;
	}
	// 从http头获取域名和端口
	static bool CalcDomainAndPortByUrl(const std::string& url, std::string& domain, int& port, std::string& path)
	{
		auto start_domain_pos = url.c_str();

		port = 80;
		if (url.substr(0, 7) == "http://")
			start_domain_pos += 7;

		if (url.size() >= 8 && url.substr(0, 8) == "https://")
		{
			start_domain_pos += 8;
			port = 443;
		}

		auto end_domain_pos = start_domain_pos;
		while (*end_domain_pos && *end_domain_pos != '/')
			++end_domain_pos;

		const auto new_url = url.substr(start_domain_pos - url.c_str(), end_domain_pos - start_domain_pos);
		domain = new_url;
		const auto port_pos = new_url.find(":");
		if (port_pos != std::string::npos)
		{
			domain = new_url.substr(0, port_pos);
			const auto str_port = new_url.substr(port_pos + 1);
			port = atoi(str_port.c_str());
		}

		path = url.substr(end_domain_pos - url.c_str());

		return true;
	}
	// 从http头获取获取文件类型
	static bool CalcContentTypeFromHttp(const std::string& response, std::string& type)
	{
		auto new_response = response;
		UpperString(new_response);

		type = "";
		auto content_length_pos = new_response.find("CONTENT-TYPE:");
		if (content_length_pos == std::string::npos)
			return false;

		const auto content_length_pos_end = new_response.find("\r\n", content_length_pos);
		if (content_length_pos_end == std::string::npos)
			return false;

		content_length_pos += strlen("CONTENT-TYPE:");
		type = response.substr(content_length_pos, content_length_pos_end - content_length_pos);
		TrimRight(type); TrimLeft(type);
		return true;
	}

public:
	// 根据后缀名查找对应的文件文件类型
	static std::string GetContentTypeByExt(const std::string& ext)
	{
		auto upper_ext = ext;
		UpperString(upper_ext);

		if (upper_ext == "HTML" || upper_ext == "HTM" || upper_ext == "HTX") return "text/html";
		if (upper_ext == "ICO") return "image/x-icon";
		if (upper_ext == "CSS") return "text/css";
		if (upper_ext == "XML" || upper_ext == "plist") return "text/xml";
		if (upper_ext == "JSON") return "application/json";
		if (upper_ext == "JS") return "application/x-javascript";
		if (upper_ext == "PNG") return "image/png";
		if (upper_ext == "JPG" || upper_ext == "JPEG") return "image/jpeg";
		if (upper_ext == "GIF") return "image/gif";
		if (upper_ext == "BMP") return "application/x-bmp";
		return "";
	}
};

/// <summary>
/// Socket
/// </summary>

typedef std::shared_ptr<asio::io_service> CarpHttpServicePtr;
typedef std::shared_ptr<asio::ip::tcp::resolver> CarpHttpResolverPtr;

typedef std::shared_ptr<asio::ip::tcp::socket> CarpHttpNTVSocketPtr;
typedef std::shared_ptr<asio::ip::tcp::acceptor> CarpHttpAcceptorPtr;

class CarpHttpSocket;
typedef std::shared_ptr<CarpHttpSocket> CarpHttpSocketPtr;

#ifdef _WIN32
#define CARP_HAS_SSL
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#endif

#ifdef CARP_HAS_SSL
#include <asio/ssl.hpp>

typedef std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> CarpHttpSSLSocketPtr;
class CarpHttpSocket
{
public:
	CarpHttpSocket(const bool is_ssl, asio::io_service* service, const std::string& domain)
	{
		if (is_ssl)
		{
			asio::ssl::context cxt(asio::ssl::context::sslv23);
			ssl_socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*service, cxt);
			// Set SNI Hostname (many hosts need this to handshake successfully)
			SSL_set_tlsext_host_name(ssl_socket->native_handle(), domain.c_str());
		}
		else
			ntv_socket = std::make_shared<asio::ip::tcp::socket>(*service);
	}

	CarpHttpSocket(const bool is_ssl, asio::io_service* service, asio::ssl::context* context)
	{
		if (is_ssl)
			ssl_socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*service, *context);
		else
			ntv_socket = std::make_shared<asio::ip::tcp::socket>(*service);
	}

public:
	CarpHttpNTVSocketPtr ntv_socket;
	CarpHttpSSLSocketPtr ssl_socket{};
};

#define CARPHTTPSOCKET_Connect(self, it, ec) \
do { \
	if (self->ntv_socket) \
		self->ntv_socket->connect(*it, ec); \
	else \
		asio::connect(self->ssl_socket->lowest_layer(), it, ec); \
} while (0)

#define CARPHTTPSOCKET_IsOpen(self, result) \
do { \
	if (self->ntv_socket) \
		result = self->ntv_socket->is_open(); \
	else \
		result = self->ssl_socket->lowest_layer().is_open(); \
} while (0)

#define CARPHTTPSOCKET_SetNoDelay(self) \
do { \
	if (self->ntv_socket) \
		self->ntv_socket->set_option(asio::ip::tcp::no_delay(true)); \
	else \
		self->ssl_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
} while (0)

#define CARPHTTPSOCKET_AfterConnect(self) \
do { \
	if (self->ntv_socket) \
	{ \
		self->ntv_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
	} \
	else \
	{ \
		self->ssl_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
		asio::error_code ec; \
		self->ssl_socket->handshake(asio::ssl::stream<asio::ip::tcp::socket>::client, ec); \
	} \
} while (0)

#define CARPHTTPSOCKET_AfterAsyncConnect(self) \
do { \
	if (self->ntv_socket) \
	{ \
		self->ntv_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
	} \
	else \
	{ \
		self->ssl_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
	} \
} while (0)

#define CARPHTTPSOCKET_Close(self) \
do { \
	asio::error_code ec; \
	if (self->ntv_socket) \
		self->ntv_socket->close(ec); \
	else \
		self->ssl_socket->shutdown(ec); \
} while (0)

#define CARPHTTPSOCKET_Write(self, content, size, ec) \
do { \
	if (self->ntv_socket) \
		asio::write(*self->ntv_socket, asio::buffer(content, size), ec); \
	else \
		asio::write(*self->ssl_socket, asio::buffer(content, size), ec); \
} while (0)

#define CARPHTTPSOCKET_ReadSome(self, content, size, ec, result) \
do { \
	if (self->ntv_socket) \
		result = self->ntv_socket->read_some(asio::buffer(content, size), ec); \
	else \
		result = self->ssl_socket->read_some(asio::buffer(content, size), ec); \
} while (0)

#define CARPHTTPSOCKET_AsyncConnect(self, it, callback) \
do { \
	if (self->ntv_socket) \
		self->ntv_socket->async_connect(*it, callback); \
	else \
		asio::async_connect(self->ssl_socket->lowest_layer(), it, callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncWrite(self, content, size, callback) \
do { \
	if (self->ntv_socket) \
		asio::async_write(*self->ntv_socket, asio::buffer(content, size), callback); \
	else \
		asio::async_write(*self->ssl_socket, asio::buffer(content, size), callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncReadSome(self, content, size, callback) \
do { \
	if (self->ntv_socket) \
		self->ntv_socket->async_read_some(asio::buffer(content, size), callback); \
	else \
		self->ssl_socket->async_read_some(asio::buffer(content, size), callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncAccept(self, acceptor, callback) \
do { \
	if (self->ntv_socket) \
		acceptor->async_accept(*self->ntv_socket, callback); \
	else \
		acceptor->async_accept(self->ssl_socket->lowest_layer(), callback); \
} while(0)

#define CARPHTTPSOCKET_GetRemoteIp(self, result) \
do { \
	if (self->ntv_socket) \
		result = self->ntv_socket->remote_endpoint().address().to_string(); \
	else \
		result = self->ssl_socket->lowest_layer().remote_endpoint().address().to_string(); \
} while(0)

#define CARPHTTPSOCKET_GetRemotePort(self, result) \
do { \
	if (self->ntv_socket) \
		result = self->ntv_socket->remote_endpoint().port(); \
	else \
		result = self->ssl_socket->lowest_layer().remote_endpoint().port(); \
} while(0)
#else
class CarpHttpSocket
{
public:
	CarpHttpSocket(bool is_ssl, asio::io_service* service, const std::string& domain)
	{
		ntv_socket = std::make_shared<asio::ip::tcp::socket>(*service);
	}

	CarpHttpSocket(bool is_ssl, asio::io_service* service)
	{
		ntv_socket = std::make_shared<asio::ip::tcp::socket>(*service);
	}

public:
	CarpHttpNTVSocketPtr ntv_socket;
};

#define CARPHTTPSOCKET_Connect(self, it, ec) \
do { \
	self->ntv_socket->connect(*it, ec); \
} while (0)

#define CARPHTTPSOCKET_IsOpen(self, result) \
do { \
	result = self->ntv_socket->is_open(); \
} while (0)

#define CARPHTTPSOCKET_SetNoDelay(self) \
do { \
	self->ntv_socket->set_option(asio::ip::tcp::no_delay(true)); \
} while (0)

#define CARPHTTPSOCKET_AfterConnect(self) \
do { \
	self->ntv_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
} while (0)

#define CARPHTTPSOCKET_AfterAsyncConnect(self) \
do { \
	self->ntv_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true)); \
} while (0)

#define CARPHTTPSOCKET_Close(self) \
do { \
	asio::error_code ec; \
	self->ntv_socket->close(ec); \
} while (0)

#define CARPHTTPSOCKET_Write(self, content, size, ec) \
do { \
	asio::write(*self->ntv_socket, asio::buffer(content, size), ec); \
} while (0)

#define CARPHTTPSOCKET_ReadSome(self, content, size, ec, result) \
do { \
	result = self->ntv_socket->read_some(asio::buffer(content, size), ec); \
} while (0)

#define CARPHTTPSOCKET_AsyncConnect(self, it, callback) \
do { \
	self->ntv_socket->async_connect(*it, callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncWrite(self, content, size, callback) \
do { \
	asio::async_write(*self->ntv_socket, asio::buffer(content, size), callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncReadSome(self, content, size, callback) \
do { \
	self->ntv_socket->async_read_some(asio::buffer(content, size), callback); \
} while(0)

#define CARPHTTPSOCKET_AsyncAccept(self, acceptor, callback) \
do { \
	acceptor->async_accept(*self->ntv_socket, callback); \
} while(0)

#define CARPHTTPSOCKET_GetRemoteIp(self, result) \
do { \
	result = self->ntv_socket->remote_endpoint().address().to_string(); \
} while(0)

#define CARPHTTPSOCKET_GetRemotePort(self, result) \
do { \
	result = self->ntv_socket->remote_endpoint().port(); \
} while(0)
#endif

/// <summary>
/// HttpClientText
/// </summary>

class CarpHttpClientText;
typedef std::shared_ptr<CarpHttpClientText> CarpHttpClientTextPtr;
typedef std::shared_ptr<asio::ip::tcp::resolver> CarpHttpResolverPtr;

#define CARP_NET_HTTP_HEAD_BUFFER_SIZE 1024

class CarpHttpClientText : public std::enable_shared_from_this<CarpHttpClientText>
{
public:
	/* send request
	 * @param get_or_post: true: get method, false: post method
	 * @param type type of content. ie text/xml. text/html application/json
	 * @param file_path: write content of response to file(if file_path is not empry, then string of callback is empty）
	 * @param func: callback function, bool:succeed or not, string:content of response, string:head of response, string:error
	 * @param is_ssl: is ssl
	 */
	void SendRequest(const std::string& url
		, bool get_or_post, const std::string& type, const char* content, size_t content_len
		, std::function<void(bool, const std::string&, const std::string&, const std::string&)> complete_func
		, std::function<void(int, int)> progress_func
		, asio::io_service* io_service
		, const std::string& file_path = "", const int start_size = 0, const std::string& add_header = "")
	{
		// get domain and port from url
		std::string domain;
		int port;
		std::string path;
		const auto result = CarpHttpHelper::CalcDomainAndPortByUrl(url, domain, port, path);
		if (!result)
		{
			m_error = "can't find domain and port in url:" + url;
			complete_func(false, "", m_response_head, m_error);
			return;
		}

		const bool is_ssl = port == 443;

		// save info
		m_complete_callback = complete_func;
		m_progress_callback = progress_func;
		m_url = url;
		m_get_or_post = get_or_post;
		m_type = type;
		m_content.resize(content_len);
		if (content && content_len > 0) memcpy(&m_content[0], content, content_len);
		m_file_path = file_path;
		m_start_size = start_size;
		m_domain = domain;
		m_io_service = io_service;

		// calc request head
		if (!GenerateRequestHead(domain, add_header))
		{
			m_error = "generate post request failed: " + url;
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		m_socket = std::make_shared<CarpHttpSocket>(is_ssl, m_io_service, domain);

		// get ip dress by domain
		m_resolver = std::make_shared<asio::ip::tcp::resolver>(*m_io_service);
		const asio::ip::tcp::resolver::query ip_query(domain, std::to_string(port));

		std::function<void(const asio::error_code&, asio::ip::tcp::resolver::iterator)> query_func = std::bind(
			&CarpHttpClientText::HandleQueryIPByDomain, this->shared_from_this()
			, std::placeholders::_1, std::placeholders::_2, domain, std::to_string(port));

		// query
		m_resolver->async_resolve(ip_query, query_func);
	}

	void Stop()
	{
		m_stopped = true;
	}

private:
	void HandleQueryIPByDomain(const asio::error_code& ec, asio::ip::tcp::resolver::iterator endpoint_iterator
		, const std::string& domain, const std::string& port)
	{
		if (ec)
		{
			m_error = "query ip by domain failed and try again:" + domain + ", " + port;
			// try again
			m_resolver = std::make_shared<asio::ip::tcp::resolver>(*m_io_service);
			const asio::ip::tcp::resolver::query ip_query(domain, port);

			std::function<void(const asio::error_code&
			                   , asio::ip::tcp::resolver::iterator)> query_func = std::bind(
				&CarpHttpClientText::HandleQueryIPByDomainAgain, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2);

			m_resolver->async_resolve(ip_query, query_func);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		const auto endpoint = endpoint_iterator;
		CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
			, std::bind(&CarpHttpClientText::HandleSocketConnect, this->shared_from_this()
				, std::placeholders::_1, ++endpoint_iterator));
	}

	void HandleQueryIPByDomainAgain(const asio::error_code& ec, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (ec)
		{
			m_error = "query ip by domain failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		const auto endpoint = endpoint_iterator;
		CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
			, std::bind(&CarpHttpClientText::HandleSocketConnect, this->shared_from_this()
				, std::placeholders::_1, ++endpoint_iterator));
	}

	void HandleSocketConnect(const asio::error_code& ec, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (!ec)
		{
			CARPHTTPSOCKET_AfterAsyncConnect(m_socket);
#ifdef CARP_HAS_SSL
			if (m_socket->ssl_socket)
				m_socket->ssl_socket->async_handshake(asio::ssl::stream<asio::ip::tcp::socket>::client
					, std::bind(&CarpHttpClientText::HandleSSLHandShake, this->shared_from_this(), std::placeholders::_1));
			else
#endif
				HandleSSLHandShake(asio::error_code());
		}
		else if (endpoint_iterator != asio::ip::tcp::resolver::iterator())
		{
			const auto endpoint = endpoint_iterator;
			CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
				, std::bind(&CarpHttpClientText::HandleSocketConnect, this->shared_from_this()
					, std::placeholders::_1, ++endpoint_iterator));
		}
		else
		{
			m_error = "connect domain failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
		}
	}

	void HandleSSLHandShake(const asio::error_code& ec)
	{
		if (ec)
		{
			m_error = "ssl hand shake failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// connect succeed and send request
		CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_head.c_str(), m_request_head.size()
			, std::bind(&CarpHttpClientText::HandleSocketSendRequestHead1, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	bool GenerateRequestHead(const std::string& domain, const std::string& add_header = "")
	{
		// cut string after https://
		std::string new_url;
		if (m_url.substr(0, 8) == "https://")
			new_url = m_url.substr(8);
		else if (m_url.substr(0, 7) == "http://")
			new_url = m_url.substr(7);
		else
			new_url = m_url;

		// find position of path
		const auto path_pos = new_url.find('/');
		if (path_pos != std::string::npos)
			new_url = CarpHttpHelper::UrlEncode(new_url.substr(path_pos));
		else
			new_url = "";

		const auto total_size = m_content.size();
		std::string method = "GET";
		if (m_get_or_post == false) method = "POST";

		// add header
		auto upper_add_header = add_header;
		CarpHttpHelper::UpperString(upper_add_header);

		// generate request
		m_request_head = "";
		if (new_url.size())
			m_request_head.append(method).append(" ").append(new_url).append(" HTTP/1.1\r\n");
		else
			m_request_head.append(method).append(" /").append(" HTTP/1.1\r\n");
		if (upper_add_header.find("ACCEPT") == std::string::npos)
			m_request_head.append("Accept: */*\r\n");
		if (upper_add_header.find("USER-AGENT") == std::string::npos)
			m_request_head.append("User-Agent: ALittle Client\r\n");
		m_request_head.append("Host: ").append(domain).append("\r\n");
		if (upper_add_header.find("CONTENT-TYPE") == std::string::npos)
			m_request_head.append("Content-Type: ").append(m_type).append("\r\n");
		m_request_head.append("Content-Length: ").append(std::to_string(total_size)).append("\r\n");
		if (upper_add_header.find("CONNECTION") == std::string::npos)
			m_request_head.append("Connection: Keep-Alive\r\n");
		m_request_head.append(add_header);
		m_request_head.append("\r\n");

		return true;
	}

	void HandleSocketSendRequestHead1(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request file end failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		if (!m_content.empty())
		{
			// connect succeed and send request
			CARPHTTPSOCKET_AsyncWrite(m_socket, &m_content[0], m_content.size()
			                          , std::bind(&CarpHttpClientText::HandleSocketSendRequestHead2, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			// start receive http response
			CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			                             , std::bind(&CarpHttpClientText::HandleResponseHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void HandleSocketSendRequestHead2(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request file end failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// start receive http response
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientText::HandleResponseHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	void HandleResponseHead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// store current size of response
		const auto current_size = static_cast<int>(m_response.size());
		// add new byte to response
		m_response.append(m_http_buffer, actual_size);

		// set start point to find
		auto find_start_pos = current_size - static_cast<int>(strlen("\r\n\r\n"));
		if (find_start_pos < 0) find_start_pos = 0;

		// find \r\n\r\n
		const auto find_pos = m_response.find("\r\n\r\n", find_start_pos);
		if (find_pos != std::string::npos)
		{
			// save response head
			m_response_head = m_response.substr(0, find_pos + strlen("\r\n\r\n"));

			// read status of head
			// std::string status;
			if (!CarpHttpHelper::CalcStatusFromHttp(m_response_head, m_status))
			{
				m_error = "http status calc failed:" + m_response;
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}

			// check status is 200
	// 		if (status != "200")
	// 		{
	// 			ALITTLE_ERROR("http status error:" << m_response);
	// 			m_complete_callback(false, "", m_response_head);
	// 			return;
	// 		}

			m_response_size = 0;
			// read content size
			if (!CarpHttpHelper::CalcFileSizeFromHttp(m_response_head, m_response_size, m_response_type))
			{
				m_error = "http file size calc failed:" + m_response;
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}
			m_total_size = m_response_size;

			// pos content is then count of bytes after current size
			const auto content_pos = static_cast<int>(find_pos) + static_cast<int>(strlen("\r\n\r\n")) - current_size;
			// calc content size
			const auto content_size = static_cast<int>(actual_size) - content_pos;

			// clear response
			m_response = "";

			// if download to file then create it
			if (m_file_path.size())
			{
				// open file
				if (m_start_size > 0)
					m_file.open(m_file_path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
				else
					m_file.open(m_file_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
				if (!m_file)
				{
					m_error = "file create failed!" + m_file_path;
					m_complete_callback(false, "", m_response_head, m_error);
					return;
				}
				if (m_start_size > 0) m_file.seekp(m_start_size);
			}

			// split handle
			if (m_response_type == CarpHttpHelper::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH)
				HandleResponseByContentLength(ec, content_size, content_pos);
			else if (m_response_type == CarpHttpHelper::ResponseType::RESPONSE_TYPE_CHUNK)
				HandleResponseByChunk(ec, content_size, content_pos);
			else
				HandleResponseByDataFollow(ec, content_size, content_pos);
		}
		else
		{
			// read next bytes
			CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
				, std::bind(&CarpHttpClientText::HandleResponseHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

private:
	void HandleResponseByContentLength(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// dec content size that is received now
		m_response_size -= static_cast<int>(actual_size);

		// write new content to file or buffer
		if (m_file_path.size())
		{
			m_file.write(m_http_buffer + buffer_offset, actual_size);
			if (m_total_size > 0 && m_progress_callback)
				m_progress_callback(m_total_size, m_total_size - m_response_size);

			if (m_response_size <= 0)
			{
				m_file.close();
				const bool result = m_status == "200";
				if (!result) m_error = "status != 200";
				m_complete_callback(result, "", m_response_head, m_error);
				return;
			}
		}
		else
		{
			m_response.append(m_http_buffer + buffer_offset, actual_size);

			if (m_response_size <= 0)
			{
				const bool result = m_status == "200";
				if (!result) m_error = "status != 200";
				m_complete_callback(result, m_response, m_response_head, m_error);
				return;
			}
		}

		// read next bytes
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientText::HandleResponseByContentLength, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
	}
	void HandleResponseByChunk(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// calc chunk size
		if (m_response_size <= 0)
		{
			// save old len of chunk size string
			const auto old_chunk_size_len = static_cast<int>(m_chunk_size.size());
			// append new buffer
			m_chunk_size.append(m_http_buffer + buffer_offset, actual_size);

			// check size complete
			auto chunk_pos = m_chunk_size.find("\r\n");
			if (chunk_pos == std::string::npos)
			{
				// if chunk is too large then fail
				if (m_chunk_size.size() >= CARP_NET_HTTP_HEAD_BUFFER_SIZE)
				{
					m_error = "read response failed: chunk size is too large! " + std::to_string(m_chunk_size.size());
					m_complete_callback(false, "", m_response_head, m_error);
					return;
				}

				CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
					, std::bind(&CarpHttpClientText::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
				return;
			}
			else if (chunk_pos == 0)
			{
				m_chunk_size = "";
				const auto add_chunk_size = static_cast<int>(strlen("\r\n"));
				if (static_cast<int>(actual_size) - add_chunk_size > 0)
					HandleResponseByChunk(ec, static_cast<int>(actual_size) - add_chunk_size, buffer_offset + add_chunk_size);
				else
					CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
						, std::bind(&CarpHttpClientText::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
				return;
			}

			const auto chunk_space = m_chunk_size.find(" ");
			if (chunk_space != std::string::npos && chunk_space < chunk_pos)
				chunk_pos = chunk_space;

			// calc chunk size
			auto response_size = 0;
			const std::string number(m_chunk_size.c_str(), chunk_pos);
			if (chunk_pos == 0 || CarpHttpHelper::String2HexNumber(response_size, number) == false)
			{
				m_error = "read chunk size calc failed:" + std::to_string(m_chunk_size.size());
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}
			m_response_size = response_size;

			// receive complete
			if (m_response_size == 0)
			{
				if (m_file_path.size())
				{
					m_file.close();
					const auto result = m_status == "200";
					if (!result) m_error = "status != 200";
					m_complete_callback(result, "", m_response_head, m_error);
				}
				else
				{
					const auto result = m_status == "200";
					if (!result) m_error = "status != 200";
					m_complete_callback(result, m_response, m_response_head, m_error);
				}
				return;
			}

			// clear chunk size string
			m_chunk_size = "";
			const auto add_chunk_size = static_cast<int>(chunk_pos) + static_cast<int>(strlen("\r\n")) - static_cast<int>(old_chunk_size_len);
			if (static_cast<int>(actual_size) - add_chunk_size > 0)
				HandleResponseByChunk(ec, static_cast<int>(actual_size) - add_chunk_size, buffer_offset + add_chunk_size);
			else
				CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
					, std::bind(&CarpHttpClientText::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
			return;
		}

		if (static_cast<int>(actual_size) <= m_response_size)
		{
			if (m_file_path.size())
				m_file.write(m_http_buffer + buffer_offset, actual_size);
			else
				m_response.append(m_http_buffer + buffer_offset, actual_size);

			m_response_size -= static_cast<int>(actual_size);
			CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
				, std::bind(&CarpHttpClientText::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
			return;
		}

		if (m_file_path.size())
			m_file.write(m_http_buffer + buffer_offset, m_response_size);
		else
			m_response.append(m_http_buffer + buffer_offset, m_response_size);

		buffer_offset += m_response_size;
		actual_size -= m_response_size;
		m_response_size = 0;

		HandleResponseByChunk(ec, actual_size, buffer_offset);
	}
	void HandleResponseByDataFollow(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			if (m_file_path.size())
			{
				m_file.close();
				const auto result = m_status == "200";
				if (!result) m_error = "status != 200";
				m_complete_callback(result, "", m_response_head, m_error);
			}
			else
			{
				const auto result = m_status == "200";
				if (!result) m_error = "status != 200";
				m_complete_callback(result, m_response, m_response_head, m_error);
			}
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// write new content to file or buffer
		if (m_file_path.size())
			m_file.write(m_http_buffer + buffer_offset, actual_size);
		else
			m_response.append(m_http_buffer + buffer_offset, actual_size);

		// read next bytes
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientText::HandleResponseByDataFollow, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
	}

private:
	std::string m_url;				// url
	std::string m_domain;			// domain
	bool		m_get_or_post = false;		// get or post
	std::string m_type;				// type of content
	std::vector<char> m_content;	// content
	std::string m_request_head;		// request head
	std::string m_response;			// response
	std::string m_response_head;	// response head
	CarpHttpHelper::ResponseType m_response_type = CarpHttpHelper::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH;			// response type
	std::string m_chunk_size;		// save for chunk size string
	int m_response_size = 0;			// size of response
	int m_total_size = 0;

	CarpHttpSocketPtr m_socket;		// socket
	CarpHttpResolverPtr m_resolver;			// resolver
	asio::io_service* m_io_service = nullptr;	// io_service

	std::function<void(bool, const std::string&, const std::string&, const std::string&)> m_complete_callback; // callback
	std::function<void(int, int)> m_progress_callback; // callback
	std::string m_file_path;		// file path to write
	int m_start_size = 0;
	std::ofstream m_file;			// file object
	std::string m_error;

private:
	char m_http_buffer[CARP_NET_HTTP_HEAD_BUFFER_SIZE] = {}; // receive buffer
	std::string m_status;
	bool m_stopped = false;
};

class CarpHttpClientPost;
typedef std::shared_ptr<CarpHttpClientPost> CarpHttpClientPostPtr;

#define CARP_CONTENT_TYPE_BOUNDARY "----WebKitFormBoundarywP43vY132opdlHoz"


class CarpHttpClientPost : public std::enable_shared_from_this<CarpHttpClientPost>
{
public:
	~CarpHttpClientPost() { if (m_file) { fclose(m_file); m_file = nullptr; } }

public:
	/* send request
	 * @param value_map: params(KEY-VALUE)
	 * @param file_name: rename the file to upload
	 * @param file_path: the path of file to upload
	 * @param func: callback function, bool:succeed or not, string:content of response, string:head of response
	 */
	void SendRequest(const std::string& url
		, const std::map<std::string, std::string>& value_map
		, const std::string& file_name
		, const std::string& file_path
		, int start_size
		, std::function<void(bool, const std::string&, const std::string&, const std::string&)> complete_func
		, std::function<void(int, int)> progress_func
		, asio::io_service* io_service
		, const std::string& add_header = "")
	{
		// get domain and port from url
		std::string domain;
		int port = 0;
		std::string path;
		const auto result = CarpHttpHelper::CalcDomainAndPortByUrl(url, domain, port, path);
		if (!result)
		{
			m_error = "can't find domain and port in url:" + url;
			complete_func(false, "", m_response_head, m_error);
			return;
		}

		const auto is_ssl = (port == 443);

		// save info
		m_complete_callback = complete_func;
		m_progress_callback = progress_func;
		m_url = url;
		m_file_path = file_path;
		m_start_size = start_size;
		m_file_name = file_name;
		m_value_map = value_map;
		m_io_service = io_service;

		// calc request head
		if (!GeneratePostRequest(domain, add_header))
		{
			m_error = "generate post request failed: " + url;
			complete_func(false, "", m_response_head, m_error);
			return;
		}

		m_socket = std::make_shared<CarpHttpSocket>(is_ssl, m_io_service, domain);

		// get ip dress by domain
		m_resolver = std::make_shared<asio::ip::tcp::resolver>(*m_io_service);
		const asio::ip::tcp::resolver::query ip_query(domain, std::to_string(port));

		std::function<void(const asio::error_code&
		                   , asio::ip::tcp::resolver::iterator)> query_func = std::bind(
			&CarpHttpClientPost::HandleQueryIPByDomain, this->shared_from_this()
			, std::placeholders::_1, std::placeholders::_2, domain, std::to_string(port));

		// query
		m_resolver->async_resolve(ip_query, query_func);
	}

	void Stop()
	{
		m_stopped = true;
	}

private:
	void HandleQueryIPByDomain(const asio::error_code& ec, asio::ip::tcp::resolver::iterator endpoint_iterator
		, const std::string& domain, const std::string& port)
	{
		if (ec)
		{
			m_error = "query ip by domain failed and try again:" + domain + ", " + port;
			// try again
			m_resolver = std::make_shared<asio::ip::tcp::resolver>(*m_io_service);
			const asio::ip::tcp::resolver::query ip_query(domain, port);

			std::function<void(const asio::error_code&
			                   , asio::ip::tcp::resolver::iterator)> query_func = std::bind(
				&CarpHttpClientPost::HandleQueryIPByDomainAgain, this->shared_from_this()
				, std::placeholders::_1, std::placeholders::_2);

			m_resolver->async_resolve(ip_query, query_func);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// start connect
		const auto endpoint = endpoint_iterator;
		CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
			, std::bind(&CarpHttpClientPost::HandleSocketConnect, this->shared_from_this()
				, std::placeholders::_1, ++endpoint_iterator));
	}

	void HandleQueryIPByDomainAgain(const asio::error_code& ec
		, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (ec)
		{
			m_error = "query ip by domain failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// start connect
		const auto endpoint = endpoint_iterator;
		CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
			, std::bind(&CarpHttpClientPost::HandleSocketConnect, this->shared_from_this()
				, std::placeholders::_1, ++endpoint_iterator));
	}

	void HandleSocketConnect(const asio::error_code& ec
		, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (!ec)
		{
			CARPHTTPSOCKET_AfterAsyncConnect(m_socket);
#ifdef CARP_HAS_SSL
			if (m_socket->ssl_socket)
				m_socket->ssl_socket->async_handshake(asio::ssl::stream<asio::ip::tcp::socket>::client
					, std::bind(&CarpHttpClientPost::HandleSSLHandShake, this->shared_from_this(), std::placeholders::_1));
			else
#endif
				HandleSSLHandShake(asio::error_code());
		}
		else if (endpoint_iterator != asio::ip::tcp::resolver::iterator())
		{
			const auto endpoint = endpoint_iterator;
			CARPHTTPSOCKET_AsyncConnect(m_socket, endpoint
				, std::bind(&CarpHttpClientPost::HandleSocketConnect, this->shared_from_this()
					, std::placeholders::_1, ++endpoint_iterator));
		}
		else
		{
			m_error = "connect domain failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
		}
	}

	void HandleSSLHandShake(const asio::error_code& ec)
	{
		if (ec)
		{
			m_error = "ssl hand shake failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// connect succeed and send request
		CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_head.c_str(), m_request_head.size()
			, std::bind(&CarpHttpClientPost::HandleSocketSendRequestHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

#ifdef _WIN32
	static std::wstring UTF82Unicode(const std::string& utf8)
	{
		const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		std::wstring result;
		if (len >= 1) result.resize(len - 1);
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, const_cast<wchar_t*>(result.c_str()), len);
		return result;
	}
#endif
	
	bool GeneratePostRequest(const std::string& domain, const std::string& add_header = "")
	{
		// cut string after http://
		std::string new_url;
		if (m_url.substr(0, 8) == "https://")
			new_url = m_url.substr(8);
		else if (m_url.substr(0, 7) == "http://")
			new_url = m_url.substr(7);
		else
			new_url = m_url;

		// find position of path
		const auto path_pos = new_url.find('/');
		if (path_pos != std::string::npos)
			new_url = CarpHttpHelper::UrlEncode(new_url.substr(path_pos));
		else
			new_url = "";

		// store total size
		size_t total_size = 0;

		// init param
		m_request_param = "";
		m_request_file_begin = "";
		m_request_file_end = "";

		// generate param part
		for (auto& pair : m_value_map)
		{
			m_request_param.append("--").append(CARP_CONTENT_TYPE_BOUNDARY).append("\r\n");
			m_request_param.append("Content-Disposition: form-data; name=\"").append(pair.first).append("\"\r\n\r\n");
			m_request_param.append(pair.second).append("\r\n");
		}

		// add param size to total size
		total_size += m_request_param.size();

		// handle file part
		if (m_file_path.size())
		{
			// open file
#ifdef _WIN32
			_wfopen_s(&m_file, UTF82Unicode(m_file_path).c_str(), L"rb");
#else
			m_file = fopen(m_file_path.c_str(), "rb");
#endif
			if (!m_file)
			{
				m_error = "can't open file:" + m_file_path;
				return false;
			}

			// calc size of file
			fseek(m_file, 0, SEEK_END);
			m_total_size = static_cast<int>(ftell(m_file));
			if (m_start_size > m_total_size) m_start_size = m_total_size;
			m_cur_size = m_start_size;
			total_size += m_total_size;
			fseek(m_file, m_start_size, SEEK_SET);

			// generate file begin param
			m_request_file_begin.append("--").append(CARP_CONTENT_TYPE_BOUNDARY).append("\r\n");
			m_request_file_begin.append("Content-Disposition: form-data; name=\"").append(m_file_name)
				.append("\"; filename=\"").append(m_file_name).append("\"\r\n");
			m_request_file_begin.append("Content-Type: application/octet-stream\r\n\r\n");

			// generate end param
			m_request_file_end.append("\r\n--").append(CARP_CONTENT_TYPE_BOUNDARY).append("--\r\n");

			// add to total size
			total_size += m_request_file_begin.size();
			total_size += m_request_file_end.size();
		}
		// handle param part
		else if (m_request_param.size())
		{
			// generate end param
			m_request_file_end.append("--").append(CARP_CONTENT_TYPE_BOUNDARY).append("--\r\n");

			// add to total size
			total_size += m_request_file_end.size();
		}

		// add header
		std::string upper_add_header = add_header;
		CarpHttpHelper::UpperString(upper_add_header);

		// generate request head
		m_request_head = "";
		if (new_url.size())
			m_request_head.append("POST ").append(new_url).append(" HTTP/1.1\r\n");
		else
			m_request_head.append("POST /").append(" HTTP/1.1\r\n");
		if (upper_add_header.find("ACCEPT") == std::string::npos)
			m_request_head.append("Accept: */*\r\n");
		if (upper_add_header.find("USER-AGENT") == std::string::npos)
			m_request_head.append("User-Agent: ALittle Client\r\n");
		m_request_head.append("Host: ").append(domain).append("\r\n");
		if (total_size > 0)
		{
			m_request_head.append("Content-Type: multipart/form-data; boundary=").append(CARP_CONTENT_TYPE_BOUNDARY).append("\r\n");
			m_request_head.append("Content-Length: ").append(std::to_string(total_size)).append("\r\n");
		}
		if (upper_add_header.find("CONNECTION") == std::string::npos)
			m_request_head.append("Connection: Keep-Alive\r\n");
		m_request_head.append(add_header);
		m_request_head.append("\r\n");

		return true;
	}

	void HandleSocketSendRequestHead(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request head failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}
		
		if (m_request_param.size())
		{
			// send param
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_param.c_str(), m_request_param.size()
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestParam, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			return;
		}

		if (m_request_file_begin.size())
		{
			// send file begin param
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_file_begin.c_str(), m_request_file_begin.size()
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFileBegin, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			return;
		}
	}

	void HandleSocketSendRequestParam(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request param failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		if (m_request_file_begin.size())
		{
			// send file begin param
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_file_begin.c_str(), m_request_file_begin.size()
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFileBegin, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			return;
		}

		// send end param
		CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_file_end.c_str(), m_request_file_end.size()
			, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFileEnd, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	void HandleSocketSendRequestFileBegin(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request file begin failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		if (m_file == nullptr)
		{
			m_error = "why m_file is null";
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}
		
		const auto read_size = fread(m_http_buffer, 1, sizeof(m_http_buffer), m_file);
		if (read_size > 0)
		{
			// send file content
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_http_buffer, read_size
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFile, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			if (m_file) { fclose(m_file); m_file = nullptr; }

			// send end param
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_file_end.c_str(), m_request_file_end.size()
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFileEnd, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void HandleSocketSendRequestFile(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request file failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		if (m_file == nullptr)
		{
			m_error = "why m_file is null";
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		m_cur_size += static_cast<int>(bytes_transferred);
		if (m_progress_callback) m_progress_callback(m_total_size, m_cur_size);

		const auto read_size = fread(m_http_buffer, 1, sizeof(m_http_buffer), m_file);
		if (read_size > 0)
		{
			// send file content
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_http_buffer, read_size
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFile, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			if (m_file) { fclose(m_file); m_file = nullptr; }

			// send end param
			CARPHTTPSOCKET_AsyncWrite(m_socket, m_request_file_end.c_str(), m_request_file_end.size()
				, std::bind(&CarpHttpClientPost::HandleSocketSendRequestFileEnd, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void HandleSocketSendRequestFileEnd(const asio::error_code& ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			m_error = "socket send post request file end failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// start receive http response
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientPost::HandleResponseHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	void HandleResponseHead(const asio::error_code& ec, std::size_t actual_size)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// store current size of response
		const auto current_size = static_cast<int>(m_response.size());
		// add new byte to response
		m_response.append(m_http_buffer, actual_size);

		// set start point to find
		auto find_start_pos = current_size - static_cast<int>(strlen("\r\n\r\n"));
		if (find_start_pos < 0) find_start_pos = 0;

		// find \r\n\r\n
		const auto find_pos = m_response.find("\r\n\r\n", find_start_pos);
		if (find_pos != std::string::npos)
		{
			// save response head
			m_response_head = m_response.substr(0, find_pos + strlen("\r\n\r\n"));

			// read status of head
			std::string status;
			if (!CarpHttpHelper::CalcStatusFromHttp(m_response_head, status))
			{
				m_error = "http status calc failed:" + m_response;
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}

			// check status is 200
			if (status != "200")
			{
				m_error = "http status error:" + status;
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}

			m_response_size = 0;
			// read content size
			if (!CarpHttpHelper::CalcFileSizeFromHttp(m_response_head, m_response_size, m_response_type))
			{
				m_error = "http file size calc failed:" + m_response;
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}

			// pos content is then count of bytes after current size
			const auto content_pos = static_cast<int>(find_pos) + static_cast<int>(strlen("\r\n\r\n")) - current_size;
			// calc content size
			const auto content_size = static_cast<int>(actual_size) - content_pos;

			// clear response
			m_response = "";

			// split handle
			if (m_response_type == CarpHttpHelper::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH)
				HandleResponseByContentLength(ec, content_size, content_pos);
			else if (m_response_type == CarpHttpHelper::ResponseType::RESPONSE_TYPE_CHUNK)
				HandleResponseByChunk(ec, content_size, content_pos);
			else
				HandleResponseByDataFollow(ec, content_size, content_pos);
		}
		else
		{
			// read next bytes
			CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
				, std::bind(&CarpHttpClientPost::HandleResponseHead, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

private:
	void HandleResponseByContentLength(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// dec content size that is received now
		m_response_size -= static_cast<int>(actual_size);

		m_response.append(m_http_buffer + buffer_offset, actual_size);

		if (m_response_size <= 0)
		{
			m_complete_callback(true, m_response, m_response_head, m_error);
			return;
		}

		// read next bytes
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientPost::HandleResponseByContentLength, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
	}
	void HandleResponseByChunk(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			m_error = "read response failed:"; m_error += std::to_string(ec.value());
			m_complete_callback(false, "", m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// calc chunk size
		if (m_response_size <= 0)
		{
			// save old len of chunk size string
			const auto old_chunk_size_len = static_cast<int>(m_chunk_size.size());
			// append new buffer
			m_chunk_size.append(m_http_buffer + buffer_offset, actual_size);

			// check size complete
			auto chunk_pos = m_chunk_size.find("\r\n");
			if (chunk_pos == std::string::npos)
			{
				// if chunk is too large then fail
				if (m_chunk_size.size() >= CARP_NET_HTTP_HEAD_BUFFER_SIZE)
				{
					m_error = "read response failed: chunk size is too large! " + std::to_string(m_chunk_size.size());
					m_complete_callback(false, "", m_response_head, m_error);
					return;
				}

				CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
					, std::bind(&CarpHttpClientPost::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
				return;
			}
			else if (chunk_pos == 0)
			{
				m_chunk_size = "";
				const auto add_chunk_size = static_cast<int>(strlen("\r\n")) - static_cast<int>(old_chunk_size_len);
				if (actual_size - add_chunk_size > 0)
					HandleResponseByChunk(ec, actual_size - add_chunk_size, buffer_offset + add_chunk_size);
				else
					CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
						, std::bind(&CarpHttpClientPost::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
				return;
			}

			const auto chunk_space = m_chunk_size.find(" ");
			if (chunk_space != std::string::npos && chunk_space < chunk_pos)
				chunk_pos = chunk_space;

			// calc chunk size
			auto result = 0;
			const std::string number(m_chunk_size.c_str(), chunk_pos);
			if (chunk_pos == 0 || CarpHttpHelper::String2HexNumber(result, number) == false)
			{
				m_error = "read chunk size calc failed:" + std::to_string(m_chunk_size.size());
				m_complete_callback(false, "", m_response_head, m_error);
				return;
			}
			m_response_size = result;

			// receive complete
			if (m_response_size == 0)
			{
				m_complete_callback(true, m_response, m_response_head, m_error);
				return;
			}

			// clear chunk size string
			m_chunk_size = "";
			const auto add_chunk_size = static_cast<int>(chunk_pos) + static_cast<int>(strlen("\r\n")) - static_cast<int>(old_chunk_size_len);
			if (static_cast<int>(actual_size) - add_chunk_size > 0)
				HandleResponseByChunk(ec, actual_size - add_chunk_size, buffer_offset + add_chunk_size);
			else
				CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
					, std::bind(&CarpHttpClientPost::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
			return;
		}

		if (static_cast<int>(actual_size) <= m_response_size)
		{
			m_response.append(m_http_buffer + buffer_offset, actual_size);

			m_response_size -= static_cast<int>(actual_size);
			CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
				, std::bind(&CarpHttpClientPost::HandleResponseByChunk, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
			return;
		}

		m_response.append(m_http_buffer + buffer_offset, m_response_size);

		buffer_offset += m_response_size;
		actual_size -= m_response_size;
		m_response_size = 0;

		HandleResponseByChunk(ec, actual_size, buffer_offset);
	}

	void HandleResponseByDataFollow(const asio::error_code& ec, std::size_t actual_size, int buffer_offset)
	{
		if (ec)
		{
			m_complete_callback(true, m_response, m_response_head, m_error);
			return;
		}

		if (m_stopped)
		{
			m_error = "stopped";
			m_complete_callback(false, "", "", m_error);
			return;
		}

		// write new content to file or buffer
		m_response.append(m_http_buffer + buffer_offset, actual_size);

		// read next bytes
		CARPHTTPSOCKET_AsyncReadSome(m_socket, m_http_buffer, sizeof(m_http_buffer)
			, std::bind(&CarpHttpClientPost::HandleResponseByDataFollow, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, 0));
	}

private:
	std::string m_url;					// url
	std::string m_request_head;			// request head
	std::string m_request_param;		// param of request
	std::string m_request_file_begin;	// head param of file
	std::string m_request_file_end;		// tail param of file
	std::string m_response;				// response
	std::string m_response_head;		// response head
	CarpHttpHelper::ResponseType m_response_type = CarpHttpHelper::ResponseType::RESPONSE_TYPE_CONTENT_LENGTH;	// response type
	std::string m_chunk_size;			// save for chunk size string
	int m_response_size = 0;				// size of response
	int m_total_size = 0;					// size of file
	int m_cur_size = 0;						// upload of file

	CarpHttpSocketPtr m_socket;			// Socket
	CarpHttpResolverPtr m_resolver;				// resolver
	asio::io_service* m_io_service = nullptr;		// io_service

	std::function<void(bool, const std::string&, const std::string&, const std::string&)> m_complete_callback; // callback
	std::function<void(int, int)> m_progress_callback;
	std::string m_file_path;			// the path of file to upload
	int m_start_size = 0;
	std::string m_file_name;			// rename the file to upload
	FILE* m_file = nullptr;						// file object

	std::map<std::string, std::string> m_value_map;	// KEY-VALUE
	std::string m_error;

private:
	char m_http_buffer[CARP_NET_HTTP_HEAD_BUFFER_SIZE] = {}; // receive buffer
	bool m_stopped = false;
};

#endif