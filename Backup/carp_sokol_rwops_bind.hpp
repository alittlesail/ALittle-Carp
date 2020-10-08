#ifndef CARP_RWOPS_BIND_INCLUDED
#define CARP_RWOPS_BIND_INCLUDED

#include "carp_rwops.hpp"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"

class CarpRWopsBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.addCFunction("FileMd5", FileMd5)
			.addFunction("CopyFile", CarpRWops::CpFile)
			.addFunction("SaveFile", CarpRWops::SaveFile)
			.addCFunction("BaseFilePath", BaseFilePath)
			.addCFunction("ExternalFilePath", ExternalFilePath)

			.beginClass<CarpLocalFile>("CarpLocalFile")
			.addConstructor<void(*)()>()
			.addFunction("SetPath", &CarpLocalFile::SetPath)
			.addFunction("Load", &CarpLocalFile::Load)
			.addFunction("Clear", &CarpLocalFile::Clear)
			.addFunction("Decrypt", &CarpLocalFile::Decrypt)
			.addFunction("Encrypt", &CarpLocalFile::Encrypt)
			.addFunction("Save", &CarpLocalFile::Save)
			.addFunction("GetContent", &CarpLocalFile::GetContent)
			.addFunction("GetPath", &CarpLocalFile::GetPath)
			.addFunction("GetSize", &CarpLocalFile::GetSize)
			.addFunction("ReadChar", &CarpLocalFile::ReadChar)
			.addFunction("ReadUInt", &CarpLocalFile::ReadUInt)
			.addFunction("ReadInt", &CarpLocalFile::ReadInt)
			.addFunction("ReadFloat", &CarpLocalFile::ReadFloat)
			.addFunction("ReadDouble", &CarpLocalFile::ReadDouble)
			.endClass()
		
			.endNamespace();
	}

	static int FileMd5(lua_State* l_state)
	{
		size_t l;
		const char* file_path = luaL_checklstring(l_state, 1, &l);
		auto result = CarpRWops::FileMd5(file_path);
		lua_pushstring(l_state, result.c_str());
		return 1;
	}

	// 获取基本路径
	static int BaseFilePath(lua_State* l_state)
	{
		auto result = CarpRWops::BaseFilePath();
		lua_pushstring(l_state, result.c_str());
		return 1;
	}

	// 获取外部路径
	static int ExternalFilePath(lua_State* l_state)
	{
		auto result = CarpRWops::ExternalFilePath();
		lua_pushstring(l_state, result.c_str());
		return 1;
	}
};

#endif
	
