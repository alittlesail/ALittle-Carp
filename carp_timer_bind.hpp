#ifndef CARP_TIMER_BIND_INCLUDED
#define CARP_TIMER_BIND_INCLUDED (1)

#include "carp_timer.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"

class CarpTimerBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpTimer>("CarpTimer")
			.addConstructor<void(*)()>()
			.addFunction("Add", &CarpTimer::Add)
			.addFunction("Remove", &CarpTimer::Remove)
			.addFunction("Update", &CarpTimer::Update)
			.addFunction("Poll", &CarpTimer::Poll)
			.endClass()
			.endNamespace();
	}
};

#endif