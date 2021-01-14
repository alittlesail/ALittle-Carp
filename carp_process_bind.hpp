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
			.addCFunction("GetProcessIDByPath", GetProcessIDByPath)
			.addFunction("KillProcessByID", CarpProcess::KillProcessByID)
			.addFunction("SendVirtualKey", CarpProcess::SendVirtualKey)
			.endNamespace();
	}

private:
	static int GetProcessIDByPath(lua_State* l_state)
	{
		const char* path = luaL_checkstring(l_state, 1);
		luaL_argcheck(l_state, path != nullptr, 1, "path is null");

		const auto result = CarpProcess::GetProcessIDByPath(path);
		lua_newtable(l_state);
		for (size_t i = 0; i < result.size(); ++i)
		{
			lua_pushinteger(l_state, result[i]);
			lua_rawseti(l_state, -2, static_cast<int>(i) + 1);
		}
		return 1;
	}
};

#endif