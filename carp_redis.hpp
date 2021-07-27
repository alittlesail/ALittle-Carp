#ifndef CARP_REDIS_INCLUDED
#define CARP_REDIS_INCLUDED

#include <map>

#include "hiredis/hiredis.h"

#include "Carp/carp_log.hpp"

#ifdef _WIN32
#pragma comment(lib, "hiredis_static.lib")
#endif

class HiredisConnection
{
public:
	HiredisConnection() {}
	~HiredisConnection() { Close(); }

public:
	/* connect redis
	 * @param ip: target ip dress
	 * @param port: port
	 * @return succeed or not
	 */
	bool Open(const char* ip, unsigned int port)
	{
		// check param
		if (ip == nullptr) { CARP_ERROR("ip == nullptr"); return false; }

		if (m_redis) return true;

		// connect
		m_redis = redisConnect(ip, port);
		if (m_redis == nullptr)
		{
			CARP_ERROR("redisConnect(\"" << ip << "\", " << port << ") == nullptr");
			return false;
		}

		// check error
		if (m_redis->err)
		{
			redisFree(m_redis);
			m_redis = nullptr;
			CARP_ERROR("redisConnect(\"" << ip << "\", " << port << ").errstr = " << m_redis->errstr);
			return false;
		}

		m_ip = ip;
		m_port = port;
		return true;
	}

	bool ReOpen()
	{
		Close();
		return Open(m_ip.c_str(), m_port);
	}

	/* reconnect
	 * @param error_message: error message
	 * @return succeed or not
	 */
	bool Ping() const
	{
		if (!m_redis)
		{
			CARP_ERROR("m_redis is null!");
			return false;
		}

		auto* reply = static_cast<redisReply*>(redisCommand(m_redis, "PING"));
		if (reply == nullptr)
		{
			CARP_ERROR("redisCommand(m_redis, \"PING\") == nullptr");
			return false;
		}

		if (reply->type == REDIS_REPLY_STRING)
		{
			CARP_ERROR("reply->type != REDIS_REPLY_STRING");
			freeReplyObject(reply);
			return false;
		}

		if (reply->str == nullptr)
		{
			CARP_ERROR("reply->str == nullptr");
			freeReplyObject(reply);
			return false;
		}

		freeReplyObject(reply);
		return true;
	}

	// close
	void Close()
	{
		if (!m_redis) return;

		// release redis object
		redisFree(m_redis);
		m_redis = nullptr;
	}

	/* current is open
	 * @return open or not
	 */
	bool IsOpen() const { return m_redis != nullptr; }

public:
	// 删除某个键，如果删除成功返回true，否则是false
	bool Delete(const char* key)
	{
		if (key == nullptr) { CARP_ERROR("key == nullptr"); return false; }
		if (m_redis == nullptr) { CARP_ERROR("m_redis == nullptr"); return false; }

		std::string cmd = "DEL ";

		if (ContainSpace(key))
		{
			cmd.push_back('\"');
			cmd += DoubleQuoteEscape(key);
			cmd.push_back('\"');
		}
		else
		{
			cmd += key;
		}

		auto* reply = static_cast<redisReply*>(redisCommand(m_redis, cmd.c_str()));
		if (reply == nullptr)
		{
			CARP_ERROR("redisCommand(m_redis, " << cmd << ") == nullptr");
			return false;
		}

		if (reply->type == REDIS_REPLY_INTEGER)
		{
			CARP_ERROR("reply->type != REDIS_REPLY_INTEGER, cmd:" << cmd);
			freeReplyObject(reply);
			return false;
		}

		bool result = reply->integer == 1;
		freeReplyObject(reply);

		return result;
	}

	// 设置键值
	template <typename T>
	bool Set(const char* key, const T& value)
	{
		std::string content = Any2String(value);
		return SetImpl(key, content.c_str());
	}

	// Set
	bool SetImpl(const char* key, const char* content)
	{
		if (key == nullptr) { CARP_ERROR("key == nullptr"); return false; }
		if (content == nullptr) { CARP_ERROR("content == nullptr"); return false; }
		if (m_redis == nullptr) { CARP_ERROR("m_redis == nullptr"); return false; }

		std::string cmd = "SET ";

		if (ContainSpace(key))
		{
			cmd.push_back('\"');
			cmd += DoubleQuoteEscape(key);
			cmd.push_back('\"');
		}
		else
		{
			cmd += key;
		}

		cmd.push_back(' ');

		if (ContainSpace(content))
		{
			cmd.push_back('\"');
			cmd += DoubleQuoteEscape(content);
			cmd.push_back('\"');
		}
		else
		{
			cmd += content;
		}

		auto* reply = static_cast<redisReply*>(redisCommand(m_redis, cmd.c_str()));
		if (reply == nullptr)
		{
			CARP_ERROR("redisCommand(m_redis, " << cmd << ") == nullptr");
			return false;
		}

		if (reply->type == REDIS_REPLY_STRING)
		{
			CARP_ERROR("reply->type != REDIS_REPLY_STRING, cmd:" << cmd);
			freeReplyObject(reply);
			return false;
		}

		if (stricmp(reply->str, "OK") != 0)
		{
			CARP_ERROR("reply->str != OK, cmd:" << cmd);
			freeReplyObject(reply);
			return false;
		}

		freeReplyObject(reply);
		return true;
	}

	template <typename T>
	bool Get(const char* key, const T& value)
	{
		auto* result = GetImpl(key);
		if (result == nullptr) return false;
		String2Any(result, value);
		return true;
	}

	// Get
	const char* GetImpl(const char* key)
	{
		if (key == nullptr) { CARP_ERROR("key == nullptr"); return nullptr; }
		if (m_redis == nullptr) { CARP_ERROR("m_redis == nullptr"); return nullptr; }

		std::string cmd = "GET ";

		if (ContainSpace(key))
		{
			cmd.push_back('\"');
			cmd += DoubleQuoteEscape(key);
			cmd.push_back('\"');
		}
		else
		{
			cmd += key;
		}

		auto* reply = static_cast<redisReply*>(redisCommand(m_redis, cmd.c_str()));
		if (reply == nullptr)
		{
			CARP_ERROR("redisCommand(m_redis, " << cmd << ") == nullptr");
			return nullptr;
		}

		if (reply->type == REDIS_REPLY_NIL)
		{
			freeReplyObject(reply);
			return nullptr;
		}

		if (reply->type != REDIS_REPLY_STRING)
		{
			CARP_ERROR("reply->type != REDIS_REPLY_STRING, cmd:" << cmd);
			freeReplyObject(reply);
			return nullptr;
		}

		if (reply->str == nullptr)
		{
			CARP_ERROR("reply->str == nullptr, cmd:" << cmd);
			freeReplyObject(reply);
			return nullptr;
		}

		m_string.assign(reply->str);
		freeReplyObject(reply);
		return m_string.c_str();
	}

private:
	template <typename T>
	static void String2Any(const char* result, T& v) { v.ReadFromString(result); }
	static void String2Any(const char* result, bool& v) { v = stricmp(result, "true") == 0; }
	static void String2Any(const char* result, char& v) { char* end = nullptr; v = static_cast<char>(std::strtol(result, &end, 10)); }
	static void String2Any(const char* result, unsigned char& v) { char* end = nullptr; v = static_cast<unsigned char>(std::strtoul(result, &end, 10)); }
	static void String2Any(const char* result, short& v) { char* end = nullptr; v = static_cast<short>(std::strtol(result, &end, 10)); }
	static void String2Any(const char* result, unsigned short& v) { char* end = nullptr; v = static_cast<unsigned short>(std::strtoul(result, &end, 10)); }
	static void String2Any(const char* result, int& v) { char* end = nullptr; v = static_cast<int>(std::strtol(result, &end, 10)); }
	static void String2Any(const char* result, unsigned int& v) { char* end = nullptr; v = static_cast<unsigned int>(std::strtoul(result, &end, 10)); }
	static void String2Any(const char* result, long& v) { char* end = nullptr; v = static_cast<long>(std::strtol(result, &end, 10)); }
	static void String2Any(const char* result, unsigned long& v) { char* end = nullptr; v = static_cast<unsigned long>(std::strtoul(result, &end, 10)); }
	static void String2Any(const char* result, long long& v) { char* end = nullptr; v = static_cast<long long>(std::strtoll(result, &end, 10)); }
	static void String2Any(const char* result, unsigned long long& v) { char* end = nullptr; v = static_cast<unsigned long long>(std::strtoull(result, &end, 10)); }
	static void String2Any(const char* result, float& v) { char* end = nullptr; v = static_cast<float>(std::strtof(result, &end)); }
	static void String2Any(const char* result, double& v) { char* end = nullptr; v = static_cast<double>(std::strtod(result, &end)); }
	static void String2Any(const char* result, std::string& v) { v = result; }

	template <typename T>
	static void Any2String(const T& v, std::string& result) { v.WriteToString(result); }
	static void Any2String(const bool& v, std::string& result) { result = v ? "true" : "false"; }
	static void Any2String(const char& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const unsigned char& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const short& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const unsigned short& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const int& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const unsigned int& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const long& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const unsigned long& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const long long& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const unsigned long long& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const float& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const double& v, std::string& result) { result = std::to_string(v); }
	static void Any2String(const std::string& v, std::string& result) { result = v; }

	// 双引号转义
	static std::string DoubleQuoteEscape(const std::string& value)
	{
		std::string result;
		for (auto& c : value)
		{
			if (c == '\\')
			{
				result.push_back(c);
				result.push_back(c);
			}
			else if (c == '"')
			{
				result.push_back('\"');
				result.push_back(c);
			}
			else if (c == '\t')
			{
				result.push_back('\\');
				result.push_back('t');
			}
			else if (c == '\r')
			{
				result.push_back('\\');
				result.push_back('r');
			}
			else if (c == '\n')
			{
				result.push_back('\\');
				result.push_back('n');
			}
			else
			{
				result.push_back(c);
			}
		}
		return result;
	}

	// 包含空格
	static bool ContainSpace(const char* content)
	{
		while (*content)
		{
			if (*content == ' ') return true;
			++content;
		}

		return false;
	}

private:
	redisContext* m_redis = nullptr;

private:
	std::string m_ip;
	unsigned int m_port = 0;
	std::string m_string;
};

#endif