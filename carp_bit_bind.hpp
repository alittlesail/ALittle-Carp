#ifndef CARP_BIT_BIND_INCLUDED
#define CARP_BIT_BIND_INCLUDED (1)

extern "C" {
#include "lua.h"
}
#include "LuaBridge/LuaBridge.h"

class CarpBitBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addFunction("BitAnd", BitAnd)
			.addFunction("BitOr", BitOr)
			.endNamespace();
	}

	static int BitAnd(int a, int b) { return a & b; }
	static int BitOr(int a, int b) { return a | b; }
};

#endif