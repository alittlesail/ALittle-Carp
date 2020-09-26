
#ifndef CARP_CRYPT_BIND_INCLUDED
#define CARP_CRYPT_BIND_INCLUDED (1)

#include "carp_crypto.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"

class CarpCryptoBind
{
public:
	static void Bind(lua_State* l_state)
	{
        luabridge::getGlobalNamespace(l_state)
            .beginNamespace("carp")
			.addCFunction("Base64Encode", Base64Encode)
            .addCFunction("Base64Decode", Base64Decode)
            .addCFunction("StringMd5", StringMd5)
			.addFunction("JSHash", CarpCrypto::JSHash)
            .endNamespace();
	}

private:
    static int Base64Encode(lua_State* l_state)
    {
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        char* out = (char*)malloc(CARP_BASE64_ENCODE_OUT_SIZE(l));
        CarpCrypto::Base64EncodeImpl((const unsigned char*)message, (int)l, out);
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

    static int Base64Decode(lua_State* l_state)
    {
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        char* out = (char*)malloc(CARP_BASE64_DECODE_OUT_SIZE(l));
        CarpCrypto::Base64DecodeImpl(message, (int)l, (unsigned char*)out);
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

private:
	static int StringMd5(lua_State* l_state)
	{
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        CarpCrypto::MD5_HASH Digest;
        CarpCrypto::Md5Calculate(message, (int)l, &Digest);
        lua_pushstring(l_state, CarpCrypto::Md4HashToString(&Digest).c_str());
        return 1;
	}
};

#endif