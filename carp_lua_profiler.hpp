#ifndef CARP_LUA_PROFILER_INCLUDED
#define CARP_LUA_PROFILER_INCLUDED

#include <string>
#include <memory>

#include "carp_lua.hpp"
#include "carp_script.hpp"

class CarpLuaProfiler
{
public:
	~CarpLuaProfiler() { }
	
public:
	void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpLuaProfiler>("CarpLuaProfiler")
			.addCFunction("Start", &CarpLuaProfiler::Start)
			.addFunction("Stop", &CarpLuaProfiler::Stop)
			.endClass()
			.endNamespace();

		luabridge::setGlobal(l_state, this, "carp_CarpLuaProfiler");
	}

public:
	struct CarpLuaRecord;
	typedef std::shared_ptr<CarpLuaRecord> CarpLuaRecordPtr;

	struct CarpLuaRecord
	{
		std::string source;
		std::string name;
		int linedefined;
		time_t total_time = 0;
		int count = 0;
		std::unordered_map<std::string, std::unordered_map<int, CarpLuaRecordPtr>> record_map;
	};

	struct CarpLuaStack
	{
		time_t start_time;
		CarpLuaRecordPtr record;
	};
	
public:
	int Start(lua_State* l_state)
	{
		auto* script = luabridge::Stack<CarpScript*>::get(l_state, 2);
		if (script == nullptr)
		{
			lua_pushboolean(l_state, 0);
			return 1;
		}

		auto* file_path = luabridge::Stack<const char*>::get(l_state, 3);
		if (file_path == nullptr)
		{
			lua_pushboolean(l_state, 0);
			return 1;
		}

		m_file_path.clear();
		if (file_path != nullptr) m_file_path = file_path;

		m_stack.resize(100);
		m_stack_size = 0;
		m_record_map.clear();

		m_global_l = script->GetLuaState();
		lua_sethook(m_global_l, DebugHook, LUA_MASKCALL | LUA_MASKRET, 0);
		m_cur_l = l_state;
		lua_sethook(m_cur_l, DebugHook, LUA_MASKCALL | LUA_MASKRET, 0);

		lua_pushboolean(l_state, 1);
		return 1;
	}

	void Stop()
	{
		if (m_cur_l == nullptr) return;

		lua_sethook(m_global_l, nullptr, 0, 0);
		m_global_l = nullptr;
		lua_sethook(m_cur_l, nullptr, 0, 0);
		m_cur_l = nullptr;

		auto* file = CarpRWops::OpenFile(m_file_path, "wb");
		if (file != nullptr)
		{
			WriteToFile(file, "", m_record_map);
			SDL_RWclose(file);
		}

		m_record_map.clear();
	}

	void WriteToFile(SDL_RWops* file, const std::string& tab, const std::unordered_map<std::string, std::unordered_map<int, CarpLuaRecordPtr>>& record_map)
	{
		std::vector<CarpLuaRecordPtr> sort_list;
		for (auto& pair : record_map)
		{
			for (auto& sub_pair : pair.second)
				sort_list.push_back(sub_pair.second);
		}
		std::sort(sort_list.begin(), sort_list.end(), [](const CarpLuaRecordPtr& a, const CarpLuaRecordPtr& b)->bool { return a->total_time > b->total_time; });

		for (auto& record : sort_list)
		{
			SDL_RWwrite(file, tab.c_str(), 1, tab.size());
			std::string desc = std::to_string(record->total_time) + "\t" + record->name + "\t" + std::to_string(record->count) +"\t" + record->source + "\t" + std::to_string(record->linedefined) + "\n";
			SDL_RWwrite(file, desc.c_str(), 1, desc.size());
			WriteToFile(file, tab + "\t", record->record_map);
		}
	}

	void DebugHookImpl(lua_State* L, lua_Debug* ar)
	{
		if (m_cur_l == nullptr) return;
		
		lua_getinfo(L, "nSl", ar);
		
		if (ar->event == LUA_HOOKCALL)
		{
			// 取出前一个栈的信息
			CarpLuaStack* pre = nullptr;
			if (m_stack_size > 0) pre = &m_stack[m_stack_size - 1];
			++m_stack_size;

			// 压栈
			if (m_stack_size > m_stack.size())
				m_stack.emplace_back(CarpLuaStack());

			// 填充信息
			CarpLuaStack& back = m_stack[m_stack_size - 1];
			back.start_time = GetCurTime();

			if (ar->source != nullptr)
			{
				if (pre != nullptr)
				{
					back.record = pre->record->record_map[ar->source][ar->linedefined];
					if (back.record == nullptr)
					{
						back.record = std::make_shared<CarpLuaRecord>();
						pre->record->record_map[ar->source][ar->linedefined] = back.record;
					}
				}
				else
				{
					back.record = m_record_map[ar->source][ar->linedefined];
					if (back.record == nullptr)
					{
						back.record = std::make_shared<CarpLuaRecord>();
						m_record_map[ar->source][ar->linedefined] = back.record;
					}
				}

				if (back.record->source.empty())
				{
					back.record->source = ar->source;
					if (ar->name != nullptr) back.record->name = ar->name;
					back.record->linedefined = ar->linedefined;
				}
			}
			else
			{
				if (pre != nullptr)
				{
					back.record = pre->record->record_map["unknown"][ar->linedefined];
					if (back.record == nullptr)
					{
						back.record = std::make_shared<CarpLuaRecord>();
						pre->record->record_map["unknown"][ar->linedefined] = back.record;
					}
				}
				else
				{
					back.record = m_record_map["unknown"][ar->linedefined];
					if (back.record == nullptr)
					{
						back.record = std::make_shared<CarpLuaRecord>();
						m_record_map["unknown"][ar->linedefined] = back.record;
					}
				}
					
				if (back.record->source.empty())
				{
					back.record->source = "unknown";
					if (ar->name != nullptr) back.record->name = ar->name;
					back.record->linedefined = ar->linedefined;
				}
			}
			
		}
		else
		{
			if (m_stack_size > 0)
			{
				CarpLuaStack& back = m_stack[m_stack_size - 1];
				--m_stack_size;

				if (back.record != nullptr)
				{
					back.record->total_time += GetCurTime() - back.start_time;
					back.record->count++;
				}
			}
		}
	}

	static void DebugHook(lua_State* L, lua_Debug* ar)
	{
		const auto ref = luabridge::getGlobal(L, "carp_CarpLuaProfiler");
		auto* server = ref.cast<CarpLuaProfiler*>();
		if (server) server->DebugHookImpl(L, ar);
	}

private:
	static time_t GetCurTime() { return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

private:
	lua_State* m_global_l = nullptr;
	lua_State* m_cur_l = nullptr;

private:
	std::string m_file_path;

private:
	std::vector<CarpLuaStack> m_stack;
	size_t m_stack_size = 0;
	std::unordered_map<std::string, std::unordered_map<int, CarpLuaRecordPtr>> m_record_map;
};

#endif
