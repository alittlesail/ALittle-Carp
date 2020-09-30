#ifndef CARP_TASK_CONSUMER_BIND_INCLUDED
#define CARP_TASK_CONSUMER_BIND_INCLUDED

#include "carp_task_consumer.hpp"
#include "carp_lua.hpp"

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
