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
};

#endif