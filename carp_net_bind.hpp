#ifndef CARP_NET_BIND_INCLUDED
#define CARP_NET_BIND_INCLUDED

#include <unordered_map>

#include "carp_lua.hpp"
#include "carp_http.hpp"
#include "carp_connect_client.hpp"
#include "carp_schedule.hpp"
#include "carp_message.hpp"
#include "carp_file.hpp"

class CarpNet
{
public:
	enum EventTypes
	{
		HTTP_SUCCEED = 1,
		HTTP_FAILED = 2,

		HTTP_FILE_SUCCEED = 11,
		HTTP_FILE_FAILED = 12,
		HTTP_FILE_PROGRESS = 13,

		TIMER = 21,     // ¶¨Ê±Æ÷

		MSG_CONNECT_SUCCEED = 31,
		MSG_CONNECT_FAILED = 32,
		MSG_DISCONNECTED = 33,
		MSG_MESSAGE = 34,
	};

	struct EventInfo
	{
		int type = 0;
		int id = 0;
		int cur_size = 0;
		int total_size = 0;
		std::string content;
		std::string error;
		CarpMessageReadFactory* factory = nullptr;
		int time = 0;
	};
	
public:
	static int Run(lua_State* l_state)
	{
		auto* c = static_cast<CarpNet*>(lua_touserdata(l_state, 1));
		luaL_argcheck(l_state, c != nullptr, 1, "net object is null");
		if (c == nullptr) return 0;
		c->m_schedule.RunOne();
		return c->HandleEvent(l_state);
	}
	
	static int Poll(lua_State* l_state)
	{
		auto* c = static_cast<CarpNet*>(lua_touserdata(l_state, 1));
		luaL_argcheck(l_state, c != nullptr, 1, "net object is null");
		if (c == nullptr) return 0;
		c->m_schedule.PollOne();
		return c->HandleEvent(l_state);
	}
	
	void Exit() { m_schedule.Exit(); }

	int HandleEvent(lua_State* L)
	{
		if (m_event_list.empty()) return 0;
		EventInfo& event = m_event_list.front();

		lua_newtable(L);
		lua_pushinteger(L, event.type);
		lua_setfield(L, -2, "type");
		lua_pushinteger(L, event.id);
		lua_setfield(L, -2, "id");
		if (event.type == HTTP_SUCCEED)
		{
			lua_pushstring(L, event.content.c_str());
			lua_setfield(L, -2, "content");
		}
		if (!event.error.empty())
		{
			lua_pushstring(L, event.error.c_str());
			lua_setfield(L, -2, "error");
		}
		if (event.type == TIMER)
		{
			lua_pushinteger(L, event.time);
			lua_setfield(L, -2, "time");
		}
		else if (event.type == HTTP_FILE_PROGRESS)
		{
			lua_pushinteger(L, event.cur_size);
			lua_setfield(L, -2, "cur_size");
			lua_pushinteger(L, event.total_size);
			lua_setfield(L, -2, "total_size");
		}
		else if (event.type == MSG_MESSAGE)
		{
			lua_pushinteger(L, event.factory->GetID());
			lua_setfield(L, -2, "msg_id");
			lua_pushinteger(L, event.factory->GetRpcID());
			lua_setfield(L, -2, "rpc_id");
			lua_pushlightuserdata(L, event.factory);
			lua_setfield(L, -2, "factory");
		}
		m_event_list.pop_front();
		return 1;
	}

public:
	void HttpGet(int id, const char* url)
	{
		CarpHttpClientTextPtr client = CarpHttpClientTextPtr(new CarpHttpClientText);
		m_get_map[id] = client;
		client->SendRequest(url, true, "text/html", "", 0
			, [this, id](bool result, const std::string& body, const std::string& head, const std::string& error)
			{
				m_get_map.erase(id);
				EventInfo event;
				event.id = id;
				if (result)
				{
					event.content = body;
					event.type = HTTP_SUCCEED;
				}
				else
				{
					event.error = error;
					event.type = HTTP_FAILED;
				}
				m_event_list.push_back(event);
			}, nullptr, &m_schedule.GetIOService(), "", 0, "");
	}
	void HttpPost(int id, const char* url, const char* type, const char* content)
	{
		CarpHttpClientTextPtr client = CarpHttpClientTextPtr(new CarpHttpClientText);
		m_post_map[id] = client;
		client->SendRequest(url, false, type, content, strlen(content)
			, [this, id](bool result, const std::string& body, const std::string& head, const std::string& error)
			{
				m_post_map.erase(id);
				EventInfo event;
				event.id = id;
				if (result)
				{
					event.content = body;
					event.type = HTTP_SUCCEED;
				}
				else
				{
					event.error = error;
					event.type = HTTP_FAILED;
				}
				m_event_list.push_back(event);
			}, nullptr, &m_schedule.GetIOService(), "", 0, "");
	}
	void HttpDownload(int id, const char* url, const char* file_path, int start_size)
	{
		CarpHttpClientTextPtr client = CarpHttpClientTextPtr(new CarpHttpClientText);
		m_download_map[id] = client;
		client->SendRequest(url, true, "text/html", "", 0
			, [this, id](bool result, const std::string& body, const std::string& head, const std::string& error)
			{
				m_download_map.erase(id);
				EventInfo event;
				event.id = id;
				if (result)
					event.type = HTTP_FILE_SUCCEED;
				else
				{
					event.error = error;
					event.type = HTTP_FILE_FAILED;
				}
				m_event_list.push_back(event);
			}
			, [this, id](int total_size, int cur_size)
			{
				EventInfo event;
				event.id = id;
				event.type = HTTP_FILE_PROGRESS;
				event.total_size = total_size;
				event.cur_size = cur_size;
				m_event_list.push_back(event);
			}, &m_schedule.GetIOService(), file_path, start_size, "");
	}
	void HttpUpload(int id, const char* url, const char* file_path, int start_size)
	{
		CarpHttpClientPostPtr client = CarpHttpClientPostPtr(new CarpHttpClientPost);
		m_upload_map[id] = client;
		client->SendRequest(url, std::map<std::string, std::string>()
			, CarpFile::GetJustFileNameByPath(file_path), file_path
			, start_size
			,[this, id](bool result, const std::string& body, const std::string& head, const std::string& error)
			{
				m_upload_map.erase(id);
				EventInfo event;
				event.id = id;
				if (result)
					event.type = HTTP_FILE_SUCCEED;
				else
				{
					event.error = error;
					event.type = HTTP_FILE_FAILED;
				}
				m_event_list.push_back(event);
			}
			, [this, id](int total_size, int cur_size)
			{
				EventInfo event;
				event.id = id;
				event.total_size = total_size;
				event.cur_size = cur_size;
				event.type = HTTP_FILE_PROGRESS;
				m_event_list.push_back(event);
			}, &m_schedule.GetIOService(), "");
	}

	void HttpStopGet(int id)
	{
		auto it = m_get_map.find(id);
		if (it == m_get_map.end()) return;

		it->second->Stop();
	}
	void HttpStopPost(int id)
	{
		auto it = m_post_map.find(id);
		if (it == m_post_map.end()) return;

		it->second->Stop();
	}
	void HttpStopDownload(int id)
	{
		auto it = m_download_map.find(id);
		if (it == m_download_map.end()) return;

		it->second->Stop();
	}
	void HttpStopUpload(int id)
	{
		auto it = m_upload_map.find(id);
		if (it == m_upload_map.end()) return;

		it->second->Stop();
	}

public:
	void Connect(int id, const char* ip, int port)
	{
		CarpConnectClientPtr client;
		const auto it = m_connect_map.find(id);
		if (it != m_connect_map.end())
			client = it->second;
		else
			client = std::make_shared<CarpConnectClient>();
		client->Connect(ip, port, &m_schedule.GetIOService()
			, [this, id]()
			{
				EventInfo event;
				event.type = MSG_CONNECT_FAILED;
				event.id = id;
				m_event_list.push_back(event);
			}, [this, id]()
			{
				EventInfo event;
				event.type = MSG_CONNECT_SUCCEED;
				event.id = id;
				m_event_list.push_back(event);
			}, [this, id]()
			{
				EventInfo event;
				event.type = MSG_DISCONNECTED;
				event.id = id;
				m_event_list.push_back(event);
			}, [this, id](void* memory, int memory_size)
			{
				auto* factory = new CarpMessageReadFactory();
				factory->DeserializeFromTotalMessage(memory);
				
				EventInfo event;
				event.type = MSG_MESSAGE;
				event.id = id;
				event.factory = factory;
				m_event_list.push_back(event);
			});
	}
	bool IsConnected(int id)
	{
		const auto it = m_connect_map.find(id);
		if (it == m_connect_map.end()) return false;
		return it->second->IsConnected();
	}
	bool IsConnecting(int id)
	{
		const auto it = m_connect_map.find(id);
		if (it == m_connect_map.end()) return false;
		return it->second->IsConnecting();
	}
	void Close(int id)
	{
		auto it = m_connect_map.find(id);
		if (it == m_connect_map.end()) return;
		it->second->Close();
		m_connect_map.erase(it);
	}
	void Send(int id, CarpMessageWriteFactory* factory)
	{
		auto it = m_connect_map.find(id);
		if (it == m_connect_map.end()) return;

		int size;
		void* memory = factory->CreateMemoryForSend(&size);
		it->second->SendPocket(memory, size);
	}

	static void FreeReadFactory(CarpMessageReadFactory* factory)
	{
		delete factory;
	}
	
	size_t GetConnectCount() const { return m_connect_map.size(); }

	void Timer(int delay_ms)
	{
		m_schedule.Timer(delay_ms, [this](time_t time)
		{
			EventInfo event;
			event.type = TIMER;
			event.time = (int)time;
			m_event_list.push_back(event);
		});
	}

private:
	std::unordered_map<int, CarpHttpClientTextPtr> m_get_map;
	std::unordered_map<int, CarpHttpClientTextPtr> m_post_map;
	std::unordered_map<int, CarpHttpClientTextPtr> m_download_map;
	std::unordered_map<int, CarpHttpClientPostPtr> m_upload_map;
	std::unordered_map<int, CarpConnectClientPtr> m_connect_map;
	CarpSchedule m_schedule;

private:
	std::list<EventInfo> m_event_list;
};

class CarpNetBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpNet>("CarpNet")
			.addConstructor<void(*)()>()
			.addStaticCFunction("Poll", CarpNet::Poll)
			.addStaticCFunction("Run", CarpNet::Run)
			.addStaticFunction("FreeReadFactory", CarpNet::FreeReadFactory)
			.addFunction("HttpGet", &CarpNet::HttpGet)
			.addFunction("HttpStopGet", &CarpNet::HttpStopGet)
			.addFunction("HttpPost", &CarpNet::HttpPost)
			.addFunction("HttpStopPost", &CarpNet::HttpStopPost)
			.addFunction("HttpDownload", &CarpNet::HttpDownload)
			.addFunction("HttpStopDownload", &CarpNet::HttpStopDownload)
			.addFunction("HttpUpload", &CarpNet::HttpUpload)
			.addFunction("HttpStopUpload", &CarpNet::HttpStopUpload)

			.addFunction("Connect", &CarpNet::Connect)
			.addFunction("IsConnected", &CarpNet::IsConnected)
			.addFunction("IsConnecting", &CarpNet::IsConnecting)
			.addFunction("Close", &CarpNet::Close)
			.addFunction("Send", &CarpNet::Send)
			.addFunction("Timer", &CarpNet::Timer)
			.addFunction("Exit", &CarpNet::Exit)
			.endClass()
			.endNamespace();
	}
};

#endif