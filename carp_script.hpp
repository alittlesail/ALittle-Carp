#ifndef CARP_SCRIPT_INCLUDED
#define CARP_SCRIPT_INCLUDED (1)

#include <string>
#include <set>

#include "carp_crypt.hpp"
#include "carp_file.hpp"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib, "luad.lib")
#else
#pragma comment(lib, "lua.lib")
#endif
#endif

#include "LuaBridge/Source/LuaBridge/LuaBridge.h"
#include "Carp/carp_log.hpp"

class CarpScript
{
public:
	CarpScript() {}
	virtual ~CarpScript() { Release(); }

public:
	void Init()
	{
		// create state
		m_L = ::luaL_newstate();
		if (!m_L)
		{
			CARP_SCRIPT_ERROR("lua_open failed!");
			return;
		}

		// open all library
		luaL_openlibs(m_L); lua_settop(m_L, 0);
	}
	void Release()
	{
		// close lua state
		if (m_L)
		{
			lua_close(m_L);
			m_L = nullptr;
		}
	}

public:
	// ���нű��ļ�
	void RunScript(const char* script, size_t len, const char* file_path)
	{
		lua_pushcclosure(m_L, OnError, 0);
		int errfunc = lua_gettop(m_L);
		std::string show_path;
		if (file_path != nullptr) show_path = file_path;
		if (show_path.size() > 48) show_path = show_path.substr(show_path.size() - 48);
		if (luaL_loadbuffer(m_L, script, len, show_path.c_str()) == 0)
		{
			lua_pushstring(m_L, file_path);
			lua_pcall(m_L, 1, 1, errfunc);
		}
		else
			PrintError(m_L, file_path, lua_tostring(m_L, -1));
		lua_remove(m_L, errfunc);
		lua_pop(m_L, 1);
	}
	void RunScriptForLua(const char* script, const char* file_path) { RunScript(script, strlen(script), file_path); }

private:
	static void CallStack(lua_State* L, int n, std::string& stack_info)
	{
		if (n >= 100) return;
		lua_Debug ar;
		if (lua_getstack(L, n, &ar) != 1) return;

		lua_getinfo(L, "nSlu", &ar);

		char text[32] = { 0 };
#ifdef _WIN32
		sprintf_s(text, "%d", ar.currentline);
#else
		sprintf(text, "%d", ar.currentline);
#endif

		if (ar.name)
			stack_info.append(ar.name).append("() : line ").append(text).append(" [").append(ar.source).append("]\n");
		else
			stack_info.append("unknown : line ").append(text).append(" [").append(ar.source).append("]\n");

		CallStack(L, n + 1, stack_info);
	}
	static int OnError(lua_State* L)
	{
		std::string content;
		const char* error = lua_tostring(L, -1);
		if (error != nullptr) content = error;
		lua_pop(L, 1);

		content.append("\n");
		CallStack(L, 0, content);
		CARP_SCRIPT_ERROR(content);

		return 0;
	}
	static void PrintError(lua_State* L, const char* name, const char* message)
	{
		std::string content = name;
		content.append("\n");
		content.append(message);
		lua_pop(L, 1);
		content.append("\n");
		CallStack(L, 0, content);
		CARP_SCRIPT_ERROR(content);
	}

	// �߽�����
	inline int InvokePush(int count)
	{
		return count;
	}

	// ����ʹ��β�ݹ��д����������û�����ǳ���Ĵ���
	template<typename T1, typename ...T2>
	inline int InvokePush(int count, T1 arg, T2 ...args)
	{
		luabridge::push(m_L, arg);
		return InvokePush(count + 1, args...);
	}

public:
	template<typename ...T>
	void Invoke(const char* name, T ...args)
	{
		if (m_L == nullptr) return;

		lua_pushcclosure(m_L, OnError, 0);
		int errfunc = lua_gettop(m_L);

		lua_getglobal(m_L, name);
		if (lua_isfunction(m_L, -1))
		{
			int count = InvokePush(0, args...);
			lua_pcall(m_L, count, 0, errfunc);
		}
		else
		{
			std::string content;
			content.append("attempt to call global '");
			content.append(name);
			content.append("' (not a function)");
			CARP_SCRIPT_ERROR(content);
		}

		lua_remove(m_L, errfunc);
	}

	bool IsFunction(const char* name)
	{
		if (m_L == nullptr) return false;
		lua_getglobal(m_L, name);
		bool result = lua_isfunction(m_L, -1) != 0;
		lua_remove(m_L, -1);
		return result;
	}

	void InvokeMain(const char* name, const std::string& module_path, int argc, char* argv[])
	{
		if (m_L == nullptr) return;

		lua_pushcclosure(m_L, OnError, 0);
		int errfunc = lua_gettop(m_L);

		lua_getglobal(m_L, name);
		if (lua_isfunction(m_L, -1))
		{
			luabridge::push(m_L, module_path.c_str());
			for (int i = 1; i < argc; ++i)
				luabridge::push(m_L, static_cast<const char*>(argv[i]));
			lua_pcall(m_L, argc, 0, errfunc);
		}
		else
		{
			std::string content;
			content.append("attempt to call global '");
			content.append(name);
			content.append("' (not a function)");
			CARP_SCRIPT_ERROR(content);
		}

		lua_remove(m_L, errfunc);
	}

public:
	lua_State* GetLuaState() { return m_L; }

protected:
	lua_State* m_L = nullptr;					// lua state

public:
	bool Require(const char* file_path)
	{
		std::string lua_path = file_path;
		lua_path += ".lua";

		if (m_script_set.count(lua_path) > 0) return true;
		m_script_set.insert(lua_path);

		std::vector<char> content;
		if (!LoadFile(lua_path.c_str(), content))
		{
			CARP_ERROR("can't find lua file:" << lua_path);
			return false;
		}

		std::string start_text = "-- ALittle Generate Lua";
		if (content.size() < start_text.size() || start_text != std::string(content.data(), start_text.size()))
			CarpCrypt::XXTeaDecodeMemory(content.data(), static_cast<int>(content.size()), 0);
		RunScript(content.data(), content.size(), lua_path.c_str());
		return true;
	}

private:	
	virtual bool LoadFile(const char* file_path, std::vector<char>& content)
	{
		return CarpFile::LoadStdFile(file_path, content);
	}

protected:
	std::set<std::string> m_script_set;
};

#endif