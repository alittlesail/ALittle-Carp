#ifndef CARP_RWOPS_BIND_INCLUDED
#define CARP_RWOPS_BIND_INCLUDED (1)

#include "carp_rwops.hpp"
extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

class CarpRWopsBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addCFunction("FileMd5", FileMd5)
			.addFunction("CopyFile", CarpRWops::CpFile)
			.addFunction("SaveFile", CarpRWops::SaveFile)
			.addCFunction("BaseFilePath", BaseFilePath)
			.addCFunction("ExternalFilePath", ExternalFilePath)
			.endNamespace();
	}

	static int FileMd5(lua_State* l_state)
	{
		size_t l;
		const char* file_path = luaL_checklstring(l_state, 1, &l);
		auto result = CarpRWops::FileMd5(file_path);
		lua_pushstring(l_state, result.c_str());
		return 1;
	}

	// 获取基本路径
	static int BaseFilePath(lua_State* l_state)
	{
		auto result = CarpRWops::BaseFilePath();
		lua_pushstring(l_state, result.c_str());
		return 1;
	}

	// 获取外部路径
	static int ExternalFilePath(lua_State* l_state)
	{
		auto result = CarpRWops::ExternalFilePath();
		lua_pushstring(l_state, result.c_str());
		return 1;
	}
};

#endif
	
