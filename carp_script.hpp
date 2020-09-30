#ifndef CARP_SCRIPT_INCLUDED
#define CARP_SCRIPT_INCLUDED

#include <string>
#include <set>

#include "carp_lua.hpp"

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib, "luad.lib")
#else
#pragma comment(lib, "lua.lib")
#endif
#endif

#include "carp_crypto.hpp"
#include "carp_file.hpp"
#include "carp_log.hpp"
#include "carp_rwops.hpp"

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

		// register script system
		luabridge::getGlobalNamespace(m_L)
			.beginNamespace("carp")
			.beginClass<CarpScript>("CarpScript")
			.addFunction("Require", &CarpScript::Require)
			.addFunction("RunScript", &CarpScript::RunScriptForLua)
			.endClass()
			.endNamespace();

		luabridge::setGlobal(m_L, this, "__CPPAPI_CarpScript");

		const std::string require = "core_require = function(path) return __CPPAPI_CarpScript:Require(path) end";
		RunScript(require.c_str(), require.size(), "ALittleBuild");
	}
	
	void Release()
	{
		// close lua state
		if (m_L)
		{
			lua_close(m_L);
			m_L = nullptr;
		}

		m_script_set.clear();
	}

public:
	// 运行脚本文件
	void RunScript(const char* script, size_t len, const char* file_path) const
	{
		lua_pushcclosure(m_L, OnError, 0);
		const int err_func = lua_gettop(m_L);
		std::string show_path;
		if (file_path != nullptr) show_path = file_path;
		if (show_path.size() > 48) show_path = show_path.substr(show_path.size() - 48);
		if (luaL_loadbuffer(m_L, script, len, show_path.c_str()) == 0)
		{
			lua_pushstring(m_L, file_path);
			lua_pcall(m_L, 1, 1, err_func);
		}
		else
			PrintError(m_L, file_path, lua_tostring(m_L, -1));
		lua_remove(m_L, err_func);
		lua_pop(m_L, 1);
	}
	void RunScriptForLua(const char* script, const char* file_path) const { RunScript(script, strlen(script), file_path); }

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

	// 边界条件
	inline int InvokePush(int count)
	{
		return count;
	}

	// 必须使用尾递归的写法，否则调用会产生非常大的代价
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
		const int err_func = lua_gettop(m_L);

		lua_getglobal(m_L, name);
		if (lua_isfunction(m_L, -1))
		{
			const int count = InvokePush(0, args...);
			lua_pcall(m_L, count, 0, err_func);
		}
		else
		{
			std::string content;
			content.append("attempt to call global '");
			content.append(name);
			content.append("' (not a function)");
			CARP_SCRIPT_ERROR(content);
		}

		lua_remove(m_L, err_func);
	}

	bool IsFunction(const char* name) const
	{
		if (m_L == nullptr) return false;
		lua_getglobal(m_L, name);
		const bool result = lua_isfunction(m_L, -1) != 0;
		lua_remove(m_L, -1);
		return result;
	}

	void InvokeMain(const char* name, const std::string& module_path, int argc, char* argv[]) const
	{
		if (m_L == nullptr) return;

		lua_pushcclosure(m_L, OnError, 0);
		const int err_func = lua_gettop(m_L);

		lua_getglobal(m_L, name);
		if (lua_isfunction(m_L, -1))
		{
			luabridge::push(m_L, module_path.c_str());
			for (int i = 1; i < argc; ++i)
				luabridge::push(m_L, static_cast<const char*>(argv[i]));
			lua_pcall(m_L, argc, 0, err_func);
		}
		else
		{
			std::string content;
			content.append("attempt to call global '");
			content.append(name);
			content.append("' (not a function)");
			CARP_SCRIPT_ERROR(content);
		}

		lua_remove(m_L, err_func);
	}

public:
	lua_State* GetLuaState() const { return m_L; }

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
		if (!CarpRWops::LoadFile(lua_path.c_str(), false, content))
		{
			CARP_ERROR("can't find lua file:" << lua_path);
			return false;
		}

		const std::string start_text = "-- ALittle Generate Lua";
		if (content.size() < start_text.size() || start_text != std::string(content.data(), start_text.size()))
			CarpCrypto::XXTeaDecodeMemory(content.data(), static_cast<int>(content.size()), nullptr);
		RunScript(content.data(), content.size(), lua_path.c_str());
		return true;
	}

protected:
	std::set<std::string> m_script_set;
};

#endif
