
#ifndef CARP_CRYPT_BIND_INCLUDED
#define CARP_CRYPT_BIND_INCLUDED (1)

#include "carp_crypt.hpp"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

class CarpCryptBind
{
public:
	static void Bind(lua_State* l_state)
	{
        luabridge::getGlobalNamespace(l_state)
            .beginNamespace("carp")
			.addCFunction("Base64Encode", Base64Encode)
            .addCFunction("Base64Decode", Base64Decode)
            .addCFunction("Md5", Md5)
			.addFunction("JSHash", CarpCrypt::JSHash)
            .endNamespace();
	}

private:
    static int Base64Encode(lua_State* l_state)
    {
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        char* out = (char*)malloc(CARP_BASE64_ENCODE_OUT_SIZE(l));
        CarpCrypt::Base64EncodeImpl((const unsigned char*)message, (int)l, out);
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

    static int Base64Decode(lua_State* l_state)
    {
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        char* out = (char*)malloc(CARP_BASE64_DECODE_OUT_SIZE(l));
        CarpCrypt::Base64DecodeImpl(message, (int)l, (unsigned char*)out);
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

private:
	static int Md5(lua_State* l_state)
	{
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        CarpCrypt::MD5_HASH hash;
        CarpCrypt::Md5Calculate(message, (int)l, &hash);
        lua_pushstring(l_state, CarpCrypt::Md4HashToString(&hash).c_str());
        return 1;
	}
};

#endif