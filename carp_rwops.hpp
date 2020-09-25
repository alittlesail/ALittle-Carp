#ifndef CARP_RWOPS_HPP_INCLUDED
#define CARP_RWOPS_HPP_INCLUDED (1)

#include <string>
#include <vector>


#include "carp_crypto.hpp"
#include "carp_rwops.h"

class CarpRWops
{
public:
	// 获取基本路径
	static std::string BaseFilePath()
	{
		std::string path = CARP_GetInternalDataPath();
		if (!path.empty()) path.push_back('/');
		return path;
	}

	// 获取外部路径
	static std::string ExternalFilePath()
	{
		std::string path = CARP_GetExternalDataPath();
		if (!path.empty()) path.push_back('/');
		return path;
	}

	// 计算文件的md5
	static std::string FileMd5(const char* file_path)
	{
		CARP_RWops* file = OpenFile(file_path, "rb", false);
		if (file == 0) return "";

		CarpCrypto::MD5_HASH Digest;
		CarpCrypto::Md5Context context;

		CarpCrypto::Md5Initialise(&context);

		unsigned char buffer[1024];
		while (true)
		{
			size_t read_size = CARP_RWread(file, buffer, 1, sizeof(buffer));
			if (read_size == 0) break;
			CarpCrypto::Md5Update(&context, buffer, (int)read_size);
		}
		CARP_RWclose(file);

		CarpCrypto::Md5Finalise(&context, &Digest);
		return CarpCrypto::Md4HashToString(&Digest);
	}

	// 打开文件
	static CARP_RWops* OpenFile(const std::string& path, const char* mode, bool only_asset=false)
	{
		// only from asset
		return CARP_RWFromFile(path.c_str(), mode, only_asset ? 1 : 0);
	}

	// 复制文件
	static bool CpFile(const char* src_path, const char* dest_path, bool only_asset)
	{
		if (src_path == 0 || dest_path == 0) return false;

		// open src file
		CARP_RWops* src_file = OpenFile(src_path, "rb", only_asset);
		if (src_file == 0) return false;

		// open dest file
		CARP_RWops* dest_file = OpenFile(dest_path, "wb", false);
		if (dest_file == 0)
		{
			CARP_RWclose(src_file);
			return false;
		}

		// define buffer, read and write
		char buff[1024];
		size_t read_size = 0;
		do
		{
			read_size = CARP_RWread(src_file, buff, 1, sizeof(buff));
			if (read_size == 0) break;
			CARP_RWwrite(dest_file, buff, 1, read_size);
		} while (true);

		// close all file
		CARP_RWclose(src_file);
		CARP_RWclose(dest_file);

		return true;
	}

	// 加载文件
	static bool LoadFile(const std::string& path, bool only_asset, std::vector<char>& memory)
	{
		// open src file
		CARP_RWops* file = OpenFile(path, "rb", only_asset);
		if (file == 0) return false;

		// get file size
		int size = (unsigned int)CARP_RWsize(file);
		memory.resize(size, 0);

		// read from memory
		if (size > 0) CARP_RWread(file, &(memory[0]), size, 1);
		// close file
		CARP_RWclose(file);

		return true;
	}

	// 保存到文件
	static bool SaveFile(const char* target_path, const char* content, int size)
	{
		if (content == 0) return false;

		CARP_RWops* file = OpenFile(target_path, "wb", false);
		if (file == 0) return false;

		if (size <= 0) size = static_cast<int>(strlen(content));
		if (size > 0) CARP_RWwrite(file, content, 1, size);
		CARP_RWclose(file);

		return true;
	}
};

#endif
	
