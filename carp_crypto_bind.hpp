
#ifndef CARP_CRYPT_BIND_INCLUDED
#define CARP_CRYPT_BIND_INCLUDED (1)

#include "carp_crypto.hpp"
#include "carp_string.hpp"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "LuaBridge/Source/LuaBridge/LuaBridge.h"

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
            .addCFunction("FileMd5", FileMd5)
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
	
    static int FileMd5(lua_State* l_state)
    {
        size_t l;
        const char* file_path = luaL_checklstring(l_state, 1, &l);

        CarpCrypto::MD5_HASH Digest;
        CarpCrypto::Md5Context context;

        CarpCrypto::Md5Initialise(&context);

#ifdef _WIN32
        std::wstring wfile_path = CarpString::UTF82Unicode(file_path);
        FILE* file = 0;
        _wfopen_s(&file, wfile_path.c_str(), L"rb");
#else
        FILE* file = fopen(file_path.c_str(), "rb");
#endif
        if (file == nullptr)
        {
            lua_pushstring(l_state, "");
            return 1;
        }

        char buffer[1024];
        while (true)
        {
            size_t read_size = fread(buffer, 1, sizeof(buffer), file);
            if (read_size == 0) break;
            CarpCrypto::Md5Update(&context, buffer, (int)read_size);
        }
        fclose(file);
		
        CarpCrypto::Md5Finalise(&context, &Digest);
        lua_pushstring(l_state, CarpCrypto::Md4HashToString(&Digest).c_str());
        return 1;
    }
};

#endif