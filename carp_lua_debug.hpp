#ifndef CARP_LUA_DEBUG_INCLUDED
#define CARP_LUA_DEBUG_INCLUDED

#include <unordered_map>
#include <unordered_set>

#include "carp_lua.hpp"
#include "carp_connect_client.hpp"
#include "carp_connect_server.hpp"

#define _CarpLuaDebugBreakPoint 0

#define _CarpLuaDebugAddBreakPoint 1
#define _CarpLuaDebugRemoveBreakPoint 2
#define _CarpLuaDebugClearBreakPoint 3

#define _CarpLuaDebugContinue 4	// ������ǰ�ϵ�
#define _CarpLuaDebugNextLine 5	// ��һ��

CARP_MESSAGE_MACRO(CarpLuaDebugBreakPoint, std::string, file_path, int, file_line);

CARP_MESSAGE_MACRO(CarpLuaDebugAddBreakPoint, std::vector<CarpLuaDebugBreakPoint>, list);
CARP_MESSAGE_MACRO(CarpLuaDebugRemoveBreakPoint, std::vector<CarpLuaDebugBreakPoint>, list);
CARP_MESSAGE_MACRO(CarpLuaDebugClearBreakPoint);

CARP_MESSAGE_MACRO(CarpLuaDebugContinue);
CARP_MESSAGE_MACRO(CarpLuaDebugNextLine);

class CarpLuaDebugClient : public CarpSchedule
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpLuaDebugClient>("CarpLuaDebugClient")
			.addConstructor<void(*)()>()
			.addFunction("Start", &CarpLuaDebugClient::Start)
			.addFunction("Close", &CarpLuaDebugClient::Close)
			.addFunction("AddBreakPoint", &CarpLuaDebugClient::AddBreakPoint)
			.addFunction("RemoveBreakPoint", &CarpLuaDebugClient::RemoveBreakPoint)
			.addFunction("ClearBreakPoint", &CarpLuaDebugClient::ClearBreakPoint)
			.endClass()
			.endNamespace();
	}
	
public:
	void Start(const char* ip, int port)
	{
		if (m_start) return;
		
		m_start = true;
		if (m_client == nullptr) m_client = std::make_shared<CarpConnectClient>();

		Run(true);
		m_client->Connect(ip, port, &GetIOService()
			, std::bind(&CarpLuaDebugClient::HandleConnectFailed, this, std::string(ip), port)
			, std::bind(&CarpLuaDebugClient::HandleConnectSucceed, this, std::string(ip), port)
			, std::bind(&CarpLuaDebugClient::HandleDisconnected, this)
			, std::bind(&CarpLuaDebugClient::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	}

	void Close()
	{
		if (!m_start) return;
		
		m_start = false;

		if (m_client) m_client->Close();

		Exit();
	}

private:
	void HandleConnectFailed(const std::string& ip, int port)
	{
		if (!m_start) return;
		
		TimerOnce(1000, [this, ip, port](time_t cur_time)
			{
				m_client->Connect(ip, port, &GetIOService()
					, std::bind(&CarpLuaDebugClient::HandleConnectFailed, this, ip, port)
					, std::bind(&CarpLuaDebugClient::HandleConnectSucceed, this, ip, port)
					, std::bind(&CarpLuaDebugClient::HandleDisconnected, this)
					, std::bind(&CarpLuaDebugClient::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
			});
	}

	void HandleConnectSucceed(const std::string& ip, int port)
	{
		m_ip = ip;
		m_port = port;
		
		CarpLuaDebugAddBreakPoint msg;
		for (auto& pair : m_break_points)
		{
			for (auto& line : pair.second)
			{
				CarpLuaDebugBreakPoint info;
				info.file_path = pair.first;
				info.file_line = line;
				msg.list.push_back(info);
			}
		}
		Send(msg);
	}

	void HandleDisconnected()
	{
		if (!m_start) return;

		TimerOnce(1000, [this](time_t cur_time)
			{
				m_client->Connect(m_ip, m_port, &GetIOService()
					, std::bind(&CarpLuaDebugClient::HandleConnectFailed, this, m_ip, m_port)
					, std::bind(&CarpLuaDebugClient::HandleConnectSucceed, this, m_ip, m_port)
					, std::bind(&CarpLuaDebugClient::HandleDisconnected, this)
					, std::bind(&CarpLuaDebugClient::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
			});
	}

	void HandleMessage(void* data, int size)
	{
		const char* body = static_cast<const char*>(data);
		// get message size
		CARP_MESSAGE_SIZE len = 0;
		memcpy(&len, body, sizeof(CARP_MESSAGE_SIZE));
		body += sizeof(CARP_MESSAGE_SIZE);
		CARP_MESSAGE_ID head_id = 0;
		memcpy(&head_id, body, sizeof(CARP_MESSAGE_ID));
		body += sizeof(CARP_MESSAGE_ID);
		CARP_MESSAGE_RPCID head_rpc_id = 0;
		memcpy(&head_rpc_id, body, sizeof(CARP_MESSAGE_RPCID));
		body += sizeof(CARP_MESSAGE_RPCID);
	}
	
public:
	void AddBreakPoint(const char* file_path, int file_line)
	{
		if (file_path == nullptr) return;
		m_break_points[file_path].insert(file_line);

		CarpLuaDebugAddBreakPoint msg;
		CarpLuaDebugBreakPoint info;
		info.file_path = file_path;
		info.file_line = file_line;
		msg.list.push_back(info);
		Send(msg);
	}

	void RemoveBreakPoint(const char* file_path, int file_line)
	{
		if (file_path == nullptr) return;

		auto it = m_break_points.find(file_path);
		if (it == m_break_points.end()) return;

		it->second.erase(file_line);
		if (it->second.empty()) m_break_points.erase(it);

		CarpLuaDebugRemoveBreakPoint msg;
		CarpLuaDebugBreakPoint info;
		info.file_path = file_path;
		info.file_line = file_line;
		msg.list.push_back(info);
		Send(msg);
	}

	void ClearBreakPoint()
	{
		m_break_points.clear();

		Send(CarpLuaDebugClearBreakPoint());
	}

	void Send(const CarpMessage& message) const
	{
		if (!m_client) return;

		int size = 0;
		void* memory = message.CreateMemoryForSend(&size);
		m_client->SendPocket(memory, size);
	}

private:
	std::unordered_map<std::string, std::unordered_set<int>> m_break_points;

private:
	CarpConnectClientPtr m_client;
	std::string m_ip;
	int m_port = 0;
	bool m_start = false;
};

class CarpLuaDebugServer : public CarpConnectSchedule
{
public:
	~CarpLuaDebugServer() { Stop(); }
	
public:
	void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpLuaDebugServer>("CarpLuaDebugServer")
			.addCFunction("Start", &CarpLuaDebugServer::Start)
			.addFunction("Stop", &CarpLuaDebugServer::Stop)
			.endClass()
			.endNamespace();

		luabridge::setGlobal(l_state, this, "carp_CarpLuaDebugServer");
	}
	
public:
	int Start(lua_State* l_state)
	{
		m_L = l_state;

		const char* yun_ip = luaL_checkstring(l_state, 2);
		const char* ip = luaL_checkstring(l_state, 3);
		int port = (int)luaL_checkinteger(l_state, 4);

		m_server = std::make_shared<CarpConnectServerImpl>();
		m_server->Start(yun_ip, ip, port, 30, this);
		Run(true);

		return 0;
	}

	void Stop()
	{
		{
			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_points.clear();
			if (m_L != nullptr) lua_sethook(m_L, nullptr, 0, 0);
			m_break_cv.notify_one();
		}
		m_L = nullptr;

		if (m_client)
		{
			m_client->Close();
			m_client = nullptr;
		}
		
		if (m_server)
		{
			m_server->Close();
			m_server = nullptr;
		}

		Exit();
	}

	void HandleClientConnect(CarpConnectReceiverPtr sender) override
	{
		if (m_client != nullptr)
		{
			sender->Close();
			return;
		}

		m_client = sender;
		CARP_SYSTEM("debug client connected!");

		{
			std::unique_lock<std::mutex> lock(m_break_mutex);
			lua_sethook(m_L, DebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
		}
	}

	void HandleClientDisconnect(CarpConnectReceiverPtr sender) override
	{
		if (m_client != sender) return;

		m_client = nullptr;

		{
			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_points.clear();
			m_break_cv.notify_one();
			lua_sethook(m_L, nullptr, 0, 0);
		}
	}

	void HandleClientMessage(CarpConnectReceiverPtr sender, int message_size, int message_id, int message_rpcid, void* memory) override
	{
		if (m_client != sender) return;

		if (message_id == CarpLuaDebugAddBreakPoint::GetStaticID())
		{
			CarpLuaDebugAddBreakPoint msg;
			msg.Deserialize(memory, message_size);

			std::unique_lock<std::mutex> lock(m_break_mutex);
			for (auto& value : msg.list)
				m_break_points[value.file_path].insert(value.file_line);
		}
		else if (message_id == CarpLuaDebugRemoveBreakPoint::GetStaticID())
		{
			CarpLuaDebugRemoveBreakPoint msg;
			msg.Deserialize(memory, message_size);

			std::unique_lock<std::mutex> lock(m_break_mutex);
			for (auto& value : msg.list)
			{
				auto it = m_break_points.find(value.file_path);
				if (it == m_break_points.end()) continue;
				
				it->second.erase(value.file_line);
				if (it->second.empty()) m_break_points.erase(it);
			}	
		}
		else if (message_id == CarpLuaDebugClearBreakPoint::GetStaticID())
		{
			CarpLuaDebugClearBreakPoint msg;
			msg.Deserialize(memory, message_size);

			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_points.clear();
		}
		else if (message_id == CarpLuaDebugContinue::GetStaticID())
		{
			CarpLuaDebugContinue msg;
			msg.Deserialize(memory, message_size);

			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_cv.notify_one();
		}
		else if (message_id == CarpLuaDebugNextLine::GetStaticID())
		{
			CarpLuaDebugNextLine msg;
			msg.Deserialize(memory, message_size);

			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_next_line = true;
			m_break_cv.notify_one();
		}
	}

	void DebugHookImpl(lua_State* L, lua_Debug* ar)
	{
		std::unique_lock<std::mutex> lock(m_break_mutex);
		auto it = m_break_points.find(ar->source);
		if (it != m_break_points.end() && it->second.find(ar->currentline) != it->second.end())
		{
			const auto file_path = it->first;
			const auto file_line = ar->currentline;
			m_break_cv.wait(lock);
			if (m_break_next_line)
			{
				m_break_next_line = false;
				m_break_next_file_path = file_path;
				m_break_next_file_line = file_line + 1;
			}
		}
		else if (!m_break_next_file_path.empty() && m_break_next_file_path == ar->source && m_break_next_file_line == ar->currentline)
		{
			const auto file_path = m_break_next_file_path;
			m_break_next_file_path.resize(0);
			m_break_cv.wait(lock);
			if (m_break_next_line)
			{
				m_break_next_line = false;
				m_break_next_file_path = file_path;
				m_break_next_file_line ++;
			}
		}
	}

	static void DebugHook(lua_State* L, lua_Debug* ar)
	{
		lua_getglobal(L, "carp_CarpLuaDebugServer");
		CarpLuaDebugServer* server = (CarpLuaDebugServer*)lua_touserdata(L, -1);
		lua_pop(L, 1);
		server->DebugHookImpl(L, ar);
	}

private:
	lua_State* m_L = nullptr;
	
private:
	bool m_break_next_line = false;
	std::string m_break_next_file_path;
	int m_break_next_file_line = 0;

private:
	std::mutex m_break_mutex;                 // ������
	std::condition_variable m_break_cv;       // ��������
	std::unordered_map<std::string, std::unordered_set<int>> m_break_points;

private:
	CarpConnectReceiverPtr m_client;
	CarpConnectServerPtr m_server;
};

#endif