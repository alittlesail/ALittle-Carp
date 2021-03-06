#ifndef CARP_MESSAGE_BIND_INCLUDED
#define CARP_MESSAGE_BIND_INCLUDED

#include "carp_message.hpp"
#include "carp_lua.hpp"

class CarpMessageBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpMessageReadFactory>("CarpMessageReadFactory")
			.addConstructor<void(*)()>()
			.addFunction("ReadFromStdFile", &CarpMessageReadFactory::ReadFromStdFile)
			.addFunction("ReadBool", &CarpMessageReadFactory::ReadBool)
			.addFunction("ReadInt", &CarpMessageReadFactory::ReadInt)
			.addFunction("ReadI64", &CarpMessageReadFactory::ReadLongLong)
			.addFunction("ReadString", &CarpMessageReadFactory::ReadString)
			.addFunction("ReadDouble", &CarpMessageReadFactory::ReadDouble)
			.addFunction("GetReadSize", &CarpMessageReadFactory::GetReadSize)
			.addFunction("GetDataSize", &CarpMessageReadFactory::GetDataSize)
			.endClass()
			.beginClass<CarpMessageWriteFactory>("CarpMessageWriteFactory")
			.addConstructor<void(*)()>()
			.addFunction("WriteToStdFile", &CarpMessageWriteFactory::WriteToStdFile)
			.addFunction("SetID", &CarpMessageWriteFactory::SetID)
			.addFunction("SetRpcID", &CarpMessageWriteFactory::SetRpcID)
			.addFunction("ResetOffset", &CarpMessageWriteFactory::ResetOffset)
			.addFunction("WriteBool", &CarpMessageWriteFactory::WriteBool)
			.addFunction("WriteInt", &CarpMessageWriteFactory::WriteInt)
			.addFunction("WriteI64", &CarpMessageWriteFactory::WriteLongLong)
			.addFunction("WriteString", &CarpMessageWriteFactory::WriteString)
			.addFunction("WriteDouble", &CarpMessageWriteFactory::WriteDouble)
			.addFunction("SetInt", &CarpMessageWriteFactory::SetInt)
			.addFunction("GetOffset", &CarpMessageWriteFactory::GetOffset)
			.endClass()
			.endNamespace();
	}
};


#endif