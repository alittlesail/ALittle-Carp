#ifndef CARP_DARKNET_BIND_INCLUDED
#define CARP_DARKNET_BIND_INCLUDED

#include "carp_lua.hpp"
#include "carp_darknet.hpp"

class CarpDarknetBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpDarknet>("CarpDarknet")
			.addConstructor<void(*)()>()
			.addFunction("Load", &CarpDarknet::Load)
			.addFunction("Predict", &CarpDarknet::Predict)
			.endClass()
			.endNamespace();
	}
};

#endif