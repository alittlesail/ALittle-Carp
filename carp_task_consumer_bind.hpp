#ifndef CARP_TASK_CONSUMER_BIND_INCLUDED
#define CARP_TASK_CONSUMER_BIND_INCLUDED (1)

#include "carp_task_consumer.hpp"
extern "C" {
#include "lua/lua.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

class CarpTaskConsumerBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addFunction("SetThreadCount", SetThreadCount)
			.addFunction("GetThreadCount", GetThreadCount)
			.endNamespace();
	}

private:
	static void SetThreadCount(int count)
	{
		s_carp_task_consumer.SetThreadCount(count);
	}

	static int GetThreadCount()
	{
		return s_carp_task_consumer.GetThreadCount();
	}
};

#endif
