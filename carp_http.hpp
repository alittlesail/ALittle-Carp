#ifndef CARP_HTTP_INCLUDED
#define CARP_HTTP_INCLUDED

#include <asio.hpp>
#include <string>

class CarpHttp
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

private:
	template <typename T>
	static void AddElement(T& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(v); }
	static void AddElement(std::list<int>& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(std::atoi(v.c_str())); }
	static void AddElement(std::vector<int>& list, const std::string& v, bool ignore_empty) { if (ignore_empty && v.empty()) return; list.push_back(std::atoi(v.c_str())); }

public:
	// 切割字符串
	template <typename T>
	static void Split(const std::string& content, const std::string& split, bool ignore_empty, T& list)
	{
		list.resize(0);
		if (content.empty()) return;

		size_t start_index = 0;
		while (true)
		{
			const size_t pos = content.find(split, start_index);
			if (pos != std::string::npos)
			{
				AddElement(list, content.substr(start_index, pos - start_index), ignore_empty);
				start_index = pos + split.size();
				continue;
			}
			break;
		}

		if (start_index == 0)
			AddElement(list, content, ignore_empty);
		else
			AddElement(list, content.substr(start_index), ignore_empty);
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

	static void AnalysisParam(const std::string& param, std::map<std::string, std::string>& value_map)
	{
		std::vector<std::string> param_list;
		Split(param, "&", true, param_list);
		for (auto& value : param_list)
		{
			std::vector<std::string> value_list;
			Split(value, "=", true, value_list);
			if (value_list.size() != 2) continue;

			value_map[value_list[0]] = value_list[1];
		}
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

		return !status.empty();
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
			length = std::atoi(content.c_str());

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
		const auto* start_domain_pos = url.c_str();

		port = 80;
		if (url.substr(0, 7) == "http://")
			start_domain_pos += 7;

		if (url.size() >= 8 && url.substr(0, 8) == "https://")
		{
			start_domain_pos += 8;
			port = 443;
		}

		const auto* end_domain_pos = start_domain_pos;
		while (*end_domain_pos && *end_domain_pos != '/')
			++end_domain_pos;

		const auto new_url = url.substr(start_domain_pos - url.c_str(), end_domain_pos - start_domain_pos);
		domain = new_url;
		const auto port_pos = new_url.find(':');
		if (port_pos != std::string::npos)
		{
			domain = new_url.substr(0, port_pos);
			const auto str_port = new_url.substr(port_pos + 1);
			port = std::atoi(str_port.c_str());
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


#endif