#ifndef CARP_BIT_BIND_INCLUDED
#define CARP_BIT_BIND_INCLUDED

#include "carp_lua.hpp"

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

private:
	static int BitAnd(int a, int b) { return a & b; }
	static int BitOr(int a, int b) { return a | b; }
};

#endif