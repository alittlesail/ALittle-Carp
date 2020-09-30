#ifndef CARP_FILE_BIND_INCLUDED
#define CARP_FILE_BIND_INCLUDED

#include "carp_file.hpp"
#include "carp_lua.hpp"

class CarpFileBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
            .addFunction("CreateFolder", CreateFolder)
            .addFunction("DeleteFolder", DeleteFolder)
			.addCFunction("GetPathAttribute", GetPathAttribute)
            .addCFunction("GetFileNameListInFolder", GetFileNameListInFolder)
            .addCFunction("GetFolderNameListInFolder", GetFolderNameListInFolder)
			.endNamespace();
	}

private:
    static void CreateFolder(const char* path)
    {
        CarpFile::CreateFolder(path);
    }

    static void DeleteFolder(const char* path)
    {
        CarpFile::DeleteFolder(path);
    }
	
	static int GetPathAttribute(lua_State* l_state)
    {
        CarpFile::PathAttribute attr;
        const char* file_path = luaL_checkstring(l_state, 1);
        if (file_path == nullptr) return 0;

        if (!CarpFile::GetPathAttribute(file_path, attr)) return 0;

        lua_newtable(l_state);
        lua_pushboolean(l_state, attr.directory ? 1 : 0);
        lua_setfield(l_state, -2, "directory");
        lua_pushinteger(l_state, attr.create_time);
        lua_setfield(l_state, -2, "create_time");
        lua_pushinteger(l_state, attr.modify_time);
        lua_setfield(l_state, -2, "modify_time");
        lua_pushinteger(l_state, attr.size);
        lua_setfield(l_state, -2, "size");
        return 1;
    }

	static int GetFileNameListInFolder(lua_State* l_state)
	{
        const char* file_path = luaL_checkstring(l_state, 1);
        if (file_path == nullptr) return 0;

        std::vector<std::string> file_list;
        std::vector<std::string> dir_list;
        CarpFile::GetNameListInFolder(file_path, file_list, dir_list);

        lua_newtable(l_state);
        for (size_t i = 0; i < file_list.size(); ++i)
        {
            lua_pushstring(l_state, file_list[i].c_str());
            lua_rawseti(l_state, -2, static_cast<int>(i) + 1);
        }
        return 1;
	}

    static int GetFolderNameListInFolder(lua_State* l_state)
    {
        const char* file_path = luaL_checkstring(l_state, 1);
        if (file_path == nullptr) return 0;

        std::vector<std::string> file_list;
        std::vector<std::string> dir_list;
        CarpFile::GetNameListInFolder(file_path, file_list, dir_list);

        lua_newtable(l_state);
        for (size_t i = 0; i < dir_list.size(); ++i)
        {
            lua_pushstring(l_state, dir_list[i].c_str());
            lua_rawseti(l_state, -2, static_cast<int>(i) + 1);
        }
        return 1;
    }
};

#endif