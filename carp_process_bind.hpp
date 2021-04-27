#ifndef CARP_PROCESS_BIND_INCLUDED
#define CARP_PROCESS_BIND_INCLUDED

#include "carp_lua.hpp"
#include "carp_process.hpp"

class CarpProcessBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addCFunction("CreateProcess", CreateCarpProcess)
			.addCFunction("GetProcessIDByPath", GetProcessIDByPath)
			.addFunction("KillProcessByID", CarpProcess::KillProcessByID)
			.addFunction("SendVirtualKey", CarpProcess::SendVirtualKey)
			.addFunction("GetCPUUsage", GetCPUUsage)
			.addFunction("GetMemoryUsage", GetMemoryUsage)
			.addFunction("GetIOUsage", GetIOUsage)
			.endNamespace();
	}

private:
	static int GetProcessIDByPath(lua_State* l_state)
	{
		const char* path = luaL_checkstring(l_state, 1);

		const auto result = CarpProcess::GetProcessIDByPath(path);
		lua_newtable(l_state);
		for (size_t i = 0; i < result.size(); ++i)
		{
			lua_pushinteger(l_state, result[i]);
			lua_rawseti(l_state, -2, static_cast<int>(i) + 1);
		}
		return 1;
	}

	static int CreateCarpProcess(lua_State* l_state)
	{
		const char* file_path = luaL_checkstring(l_state, 1);
		const char* param = luaL_checkstring(l_state, 2);
		size_t l = 0;
		const char* work_path = luaL_tolstring(l_state, 3, &l);

		if (CarpProcess::CreateCarpProcess(file_path, param, work_path))
			lua_pushboolean(l_state, 1);
		else
			lua_pushboolean(l_state, 0);
		return 1;
	}

	static int GetCPUUsage(lua_State* l_state)
	{
		const int process_id = static_cast<int>(luaL_checkinteger(l_state, 1));
		long long last_time = luaL_checkinteger(l_state, 2);
		long long last_system_time = luaL_checkinteger(l_state, 3);

		const auto cpu = CarpProcess::GetCPUUsage(process_id, last_time, last_system_time);
		lua_pushinteger(l_state, cpu);
		lua_pushinteger(l_state, last_time);
		lua_pushinteger(l_state, last_system_time);
		return 3;
	}

	static int GetMemoryUsage(lua_State* l_state)
	{
		const int process_id = static_cast<int>(luaL_checkinteger(l_state, 1));

		size_t mem = 0;
		size_t vmem = 0;
		if (CarpProcess::GetMemoryUsage(process_id, mem, vmem))
		{
			lua_pushboolean(l_state, 1);
			lua_pushinteger(l_state, mem);
			lua_pushinteger(l_state, vmem);
			return 3;
		}

		lua_pushboolean(l_state, 0);
		return 1;
	}

	static int GetIOUsage(lua_State* l_state)
	{
		const int process_id = static_cast<int>(luaL_checkinteger(l_state, 1));

		size_t read = 0;
		size_t write = 0;
		if (CarpProcess::GetIOUsage(process_id, read, write))
		{
			lua_pushboolean(l_state, 1);
			lua_pushinteger(l_state, read);
			lua_pushinteger(l_state, write);
			return 3;
		}

		lua_pushboolean(l_state, 0);
		return 1;
	}
};

#endif