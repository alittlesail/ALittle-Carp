#ifndef CARP_GRID_JPS_BIND_INCLUDED
#define CARP_GRID_JPS_BIND_INCLUDED

#include "carp_square_jps.hpp"
#include "carp_lua.hpp"

class CarpSquareJPSForLua : public CarpSquareJPS
{
public:
	CarpSquareJPSForLua(int width, int height) : CarpSquareJPS(width, height) {}

	int SearchRouteForLua(lua_State* l_state)
	{
		const int start_x = static_cast<int>(luaL_checkinteger(l_state, 2));
		const int start_y = static_cast<int>(luaL_checkinteger(l_state, 3));

		const int end_x = static_cast<int>(luaL_checkinteger(l_state, 4));
		const int end_y = static_cast<int>(luaL_checkinteger(l_state, 5));

		std::list<int> x_list;
		std::list<int> y_list;

		const auto result = SearchRoute(start_x, start_y, end_x, end_y, x_list, y_list);
		if (!result)
		{
			lua_pushboolean(l_state, 0);
			lua_pushnil(l_state);
			lua_pushnil(l_state);
		}
		else
		{
			lua_pushboolean(l_state, 1);
			lua_newtable(l_state);
			size_t index = 0;
			for (auto& value : x_list)
			{
				lua_pushinteger(l_state, value);
				lua_rawseti(l_state, -2, static_cast<int>(index) + 1);
				++index;
			}

			lua_newtable(l_state);
			index = 0;
			for (auto& value : y_list)
			{
				lua_pushinteger(l_state, value);
				lua_rawseti(l_state, -2, static_cast<int>(index) + 1);
				++index;
			}
		}
		return 3;
	}

	bool IsEmpty(int x, int y) const override { return CarpSquareJPS::IsEmpty(x, y); }
	void SetEmpty(int x, int y, bool value) override { CarpSquareJPS::SetEmpty(x, y, value); }
	void SetAllEmpty() override { CarpSquareJPS::SetAllEmpty(); }
};

class CarpSquareJPSBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpSquareJPSForLua>("CarpSquareJPS")
			.addConstructor<void(*)(int,int)>()
			.addFunction("IsEmpty", &CarpSquareJPSForLua::IsEmpty)
			.addFunction("SetEmpty", &CarpSquareJPSForLua::SetEmpty)
			.addFunction("SetAllEmpty", &CarpSquareJPSForLua::SetAllEmpty)
		    .addCFunction("SearchRoute", &CarpSquareJPSForLua::SearchRouteForLua)
			.endClass()
			.endNamespace();
	}
};
#endif
