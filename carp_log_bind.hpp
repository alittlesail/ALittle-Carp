#ifndef CARP_LOG_BIND_INCLUDED
#define CARP_LOG_BIND_INCLUDED (1)

#include "carp_log.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"

class CarpLogBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addFunction("Log", Log)
			.endNamespace();
	}

private:
	static void Log(const char* content, int level)
	{
		s_carp_log.Log(content, level);
	}
};

#endif


