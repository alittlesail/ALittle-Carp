
#ifndef CARP_CSV_BIND_INCLUDED
#define CARP_CSV_BIND_INCLUDED (1)

#include "carp_csv.hpp"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

class CarpCsvBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpCsv>("CarpCsv")
			.addConstructor<void(*)()>()
			.addFunction("Load", &CarpCsv::Load)
			.addFunction("GetColCount", &CarpCsv::GetColCount)
			.addFunction("GetRowCount", &CarpCsv::GetRowCount)
			.addFunction("Clear", &CarpCsv::Clear)
			.addFunction("GetCell", &CarpCsv::GetCell)
			.addFunction("GetPath", &CarpCsv::GetPath)
			.endClass()
			.endNamespace();
	}
};

#endif