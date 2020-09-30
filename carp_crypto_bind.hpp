
#ifndef CARP_CRYPT_BIND_INCLUDED
#define CARP_CRYPT_BIND_INCLUDED

#include "carp_crypto.hpp"
#include "carp_lua.hpp"

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
        char* out = static_cast<char*>(malloc(CARP_BASE64_ENCODE_OUT_SIZE(l)));
        CarpCrypto::Base64EncodeImpl(reinterpret_cast<const unsigned char*>(message), static_cast<int>(l), out);
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

    static int Base64Decode(lua_State* l_state)
    {
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        char* out = static_cast<char*>(malloc(CARP_BASE64_DECODE_OUT_SIZE(l)));
        CarpCrypto::Base64DecodeImpl(message, static_cast<int>(l), reinterpret_cast<unsigned char*>(out));
        lua_pushstring(l_state, out);
        free(out);
        return 1;
    }

private:
	static int StringMd5(lua_State* l_state)
	{
        size_t l;
        const char* message = luaL_checklstring(l_state, 1, &l);
        CarpCrypto::MD5_HASH digest;
        CarpCrypto::Md5Calculate(message, static_cast<int>(l), &digest);
        lua_pushstring(l_state, CarpCrypto::Md4HashToString(&digest).c_str());
        return 1;
	}
};

#endif