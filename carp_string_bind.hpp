#ifndef CARP_STRING_BIND_INCLUDED
#define CARP_STRING_BIND_INCLUDED (1)

#include "carp_string.hpp"
extern "C" {
#include "lua/lua.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

class CarpStringBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addFunction("UTF8WordCount", CarpString::UTF8CalcWordCount)
			.addFunction("UTF8ByteCount", CarpString::UTF8CalcByteCountByWordCount)
			.endNamespace();
	}
};

#endif