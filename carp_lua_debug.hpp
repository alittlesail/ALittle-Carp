#ifndef CARP_LUA_DEBUG_INCLUDED
#define CARP_LUA_DEBUG_INCLUDED

#include <unordered_map>
#include <unordered_set>

#include "carp_lua.hpp"
#include "carp_connect_client.hpp"
#include "carp_connect_server.hpp"
#include "carp_script.hpp"

#define _CarpLuaDebugBreakPoint 0

#define _CarpLuaDebugAddBreakPoint 1
#define _CarpLuaDebugRemoveBreakPoint 2
#define _CarpLuaDebugClearBreakPoint 3

#define _CarpLuaDebugContinue 4	// 跳过当前断点
#define _CarpLuaDebugNextLine 5	// 下一行

#define _CarpLuaDebugBreakIn 6	// 断点
#define _CarpLuaDebugBreakOut 7	// 继续

CARP_MESSAGE_MACRO(CarpLuaDebugBreakPoint, std::string, file_path, int, file_line);

CARP_MESSAGE_MACRO(CarpLuaDebugAddBreakPoint, std::vector<CarpLuaDebugBreakPoint>, list);
CARP_MESSAGE_MACRO(CarpLuaDebugRemoveBreakPoint, std::vector<CarpLuaDebugBreakPoint>, list);
CARP_MESSAGE_MACRO(CarpLuaDebugClearBreakPoint);

CARP_MESSAGE_MACRO(CarpLuaDebugContinue);
CARP_MESSAGE_MACRO(CarpLuaDebugNextLine);

CARP_MESSAGE_MACRO(CarpLuaDebugBreakIn, std::string, file_path, int, file_line);
CARP_MESSAGE_MACRO(CarpLuaDebugBreakOut);

enum CarpLuaDebugEvent
{
	CLDE_BREAK_IN = 1,
	CLDE_BREAK_OUT = 2,
};

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
			.addCFunction("HandleEvent", &CarpLuaDebugClient::HandleEvent)
			.addFunction("DoContinue", &CarpLuaDebugClient::DoContinue)
			.addFunction("DoNextLine", &CarpLuaDebugClient::DoNextLine)
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

		std::string ip_temp = ip;
		m_client->Connect(ip_temp, port, &GetIOService()
			, std::bind(&CarpLuaDebugClient::HandleConnectFailed, this, ip_temp, port)
			, std::bind(&CarpLuaDebugClient::HandleConnectSucceed, this, ip_temp, port)
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

		if (head_id == CarpLuaDebugBreakOut::GetStaticID())
		{
			CarpLuaDebugBreakOut msg;
			msg.Deserialize(body, len);

			PushEvent([this](lua_State* L)->int
				{
					lua_newtable(L);
					lua_pushinteger(L, CLDE_BREAK_OUT);
					lua_setfield(L, -2, "type");
					return 1;
				});
		}
		else if (head_id == CarpLuaDebugBreakIn::GetStaticID())
		{
			CarpLuaDebugBreakIn msg;
			msg.Deserialize(body, len);

			PushEvent([this, msg](lua_State* L)->int
				{
					lua_newtable(L);
					lua_pushinteger(L, CLDE_BREAK_IN);
					lua_setfield(L, -2, "type");

					lua_pushstring(L, msg.file_path.c_str());
					lua_setfield(L, -2, "file_path");

					lua_pushinteger(L, msg.file_line);
					lua_setfield(L, -2, "file_line");
					return 1;
				});
		}
	}
	
public:
	void PushEvent(const std::function<int(lua_State*)>& event)
	{
		m_mutex.lock();
		m_event_list.push_back(event);
		m_mutex.unlock();
	}
	
	int HandleEvent(lua_State* L)
	{
		std::function<int(lua_State*)> event;
		m_mutex.lock();
		if (m_event_list.empty())
		{
			m_mutex.unlock();
			return 0;
		}
		event = m_event_list.front();
		m_event_list.pop_front();
		m_mutex.unlock();
		
		return event(L);
	}

	void DoContinue()
	{
		Execute([this]()
			{
				Send(CarpLuaDebugContinue());
			});
	}

	void DoNextLine()
	{
		Execute([this]()
			{
				Send(CarpLuaDebugNextLine());
			});
	}
	
	void AddBreakPoint(const char* file_path, int file_line)
	{
		if (file_path == nullptr) return;

		std::string file_path_temp = file_path;
		Execute([this, file_path_temp, file_line]()
			{
				m_break_points[file_path_temp].insert(file_line);

				CarpLuaDebugAddBreakPoint msg;
				CarpLuaDebugBreakPoint info;
				info.file_path = file_path_temp;
				info.file_line = file_line;
				msg.list.push_back(info);
				Send(msg);
			});
	}

	void RemoveBreakPoint(const char* file_path, int file_line)
	{
		if (file_path == nullptr) return;

		std::string file_path_temp = file_path;
		Execute([this, file_path_temp, file_line]()
			{
				auto it = m_break_points.find(file_path_temp);
				if (it == m_break_points.end()) return;

				it->second.erase(file_line);
				if (it->second.empty()) m_break_points.erase(it);

				CarpLuaDebugRemoveBreakPoint msg;
				CarpLuaDebugBreakPoint info;
				info.file_path = file_path_temp;
				info.file_line = file_line;
				msg.list.push_back(info);
				Send(msg);
			});
	}

	void ClearBreakPoint()
	{
		Execute([this]()
			{
				m_break_points.clear();
				Send(CarpLuaDebugClearBreakPoint());
			});
	}

	void Send(const CarpMessage& message) const
	{
		if (!m_client || !m_client->IsConnected()) return;

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

private:
	std::mutex m_mutex;
	std::list<std::function<int(lua_State*)>> m_event_list;
};

class CarpLuaDebugServer : public CarpConnectInterface, public CarpSchedule
{
public:
	~CarpLuaDebugServer() { Stop(); }
	
public:
	void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpLuaDebugServer>("CarpLuaDebugServer")
			.addFunction("Start", &CarpLuaDebugServer::Start)
			.addFunction("Stop", &CarpLuaDebugServer::Stop)
			.endClass()
			.endNamespace();

		luabridge::setGlobal(l_state, this, "carp_CarpLuaDebugServer");
	}
	
public:
	bool Start(CarpScript* script, const char* yun_ip, const char* ip, int port)
	{
		if (script == nullptr || ip == nullptr) return false;
		
		CARP_SYSTEM("debug server start");

		m_L = script->GetLuaState();
		lua_sethook(m_L, DebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);

		m_server = std::make_shared<CarpConnectServerImpl>();
		m_server->Start(yun_ip, ip, port, 30, this, this);
		Run(true);

		return true;
	}

	void Stop()
	{
		CARP_SYSTEM("debug server stop");
		
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
			m_break_points.clear();
			m_break_cv.notify_one();
		}
	}

	void HandleClientDisconnect(CarpConnectReceiverPtr sender) override
	{
		if (m_client != sender) return;

		CARP_SYSTEM("debug client disconnected!");
		m_client = nullptr;

		{
			std::unique_lock<std::mutex> lock(m_break_mutex);
			m_break_points.clear();
			m_break_cv.notify_one();
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

	void InBreak(const std::string& file_path, int file_line)
	{
		if (!m_client) return;
		CarpLuaDebugBreakIn msg;
		msg.file_path = file_path;
		msg.file_line = file_line;
		m_client->Send(msg);
	}

	void OutBreak()
	{
		if (!m_client) return;
		m_client->Send(CarpLuaDebugBreakOut());
	}

	void DebugHookImpl(lua_State* L, lua_Debug* ar)
	{
		std::unique_lock<std::mutex> lock(m_break_mutex);
		if (m_break_points.empty() && m_break_next_file_path.empty()) return;

		lua_getinfo(L, "nSlu", ar);
		auto it = m_break_points.find(ar->source);
		if (it != m_break_points.end() && it->second.find(ar->currentline) != it->second.end())
		{
			const auto file_path = it->first;
			const auto file_line = ar->currentline;
			m_in_break = true;
			Execute([this, file_path, file_line]() {InBreak(file_path, file_line); });
			m_break_cv.wait(lock);
			m_in_break = false;
			Execute([this]() {OutBreak(); });
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
			m_in_break = true;
			Execute([this, file_path, file_line = m_break_next_file_line]() {InBreak(file_path, file_line); });
			m_break_cv.wait(lock);
			m_in_break = false;
			Execute([this]() {OutBreak(); });
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
		const auto ref = luabridge::getGlobal(L, "carp_CarpLuaDebugServer");
		auto* server = ref.cast<CarpLuaDebugServer*>();
		if (server) server->DebugHookImpl(L, ar);
	}

private:
	lua_State* m_L = nullptr;
	
private:
	bool m_break_next_line = false;
	std::string m_break_next_file_path;
	int m_break_next_file_line = 0;

private:
	std::mutex m_break_mutex;                 // 互斥锁
	std::condition_variable m_break_cv;       // 条件变量
	std::unordered_map<std::string, std::unordered_set<int>> m_break_points;
	bool m_in_break = false;

private:
	CarpConnectReceiverPtr m_client;
	CarpConnectServerPtr m_server;
};

#endif
