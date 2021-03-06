
#ifndef CARP_CSV_BIND_INCLUDED
#define CARP_CSV_BIND_INCLUDED

#include "carp_lua.hpp"
#include "carp_csv.hpp"

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
			.addFunction("Close", &CarpCsv::Close)
			.addFunction("ReadCell", &CarpCsv::ReadCell)
			.addFunction("GetPath", &CarpCsv::GetPath)
			.endClass()
			.endNamespace();
	}
};

#endif