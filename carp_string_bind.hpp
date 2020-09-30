#ifndef CARP_STRING_BIND_INCLUDED
#define CARP_STRING_BIND_INCLUDED

#include "carp_string.hpp"
#include "carp_lua.hpp"

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