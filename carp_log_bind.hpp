#ifndef CARP_LOG_BIND_INCLUDED
#define CARP_LOG_BIND_INCLUDED

#include "carp_log.hpp"
#include "carp_lua.hpp"

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


