#ifndef CARP_SOKOL_RWOPS_HPP_INCLUDED
#define CARP_SOKOL_RWOPS_HPP_INCLUDED

#include <string>
#include <vector>

#include "carp_sokol_rwops.h"
#include "carp_crypto.hpp"

class CarpRWops
{
public:
	// 获取基本路径
	static std::string BaseFilePath()
	{
		std::string path = Carp_GetInternalDataPath();
		if (!path.empty()) path.push_back('/');
		return path;
	}

	// 获取外部路径
	static std::string ExternalFilePath()
	{
		std::string path = Carp_GetExternalDataPath();
		if (!path.empty()) path.push_back('/');
		return path;
	}

	// 计算文件的md5
	static std::string FileMd5(const char* file_path)
	{
		Carp_RWops* file = OpenFile(file_path, "rb", false);
		if (file == 0) return "";

		CarpCrypto::MD5_HASH Digest;
		CarpCrypto::Md5Context context;

		CarpCrypto::Md5Initialise(&context);

		unsigned char buffer[1024];
		while (true)
		{
			size_t read_size = Carp_RWread(file, buffer, 1, sizeof(buffer));
			if (read_size == 0) break;
			CarpCrypto::Md5Update(&context, buffer, (int)read_size);
		}
		Carp_RWclose(file);

		CarpCrypto::Md5Finalise(&context, &Digest);
		return CarpCrypto::Md4HashToString(&Digest);
	}

	// 打开文件
	static Carp_RWops* OpenFile(const std::string& path, const char* mode, bool only_asset=false)
	{
		// only from asset
		return Carp_RWFromFile(path.c_str(), mode, only_asset ? 1 : 0);
	}

	// 复制文件
	static bool CpFile(const char* src_path, const char* dest_path, bool only_asset)
	{
		if (src_path == 0 || dest_path == 0) return false;

		// open src file
		Carp_RWops* src_file = OpenFile(src_path, "rb", only_asset);
		if (src_file == 0) return false;

		// open dest file
		Carp_RWops* dest_file = OpenFile(dest_path, "wb", false);
		if (dest_file == 0)
		{
			Carp_RWclose(src_file);
			return false;
		}

		// define buffer, read and write
		char buff[1024];
		size_t read_size = 0;
		do
		{
			read_size = Carp_RWread(src_file, buff, 1, sizeof(buff));
			if (read_size == 0) break;
			Carp_RWwrite(dest_file, buff, 1, read_size);
		} while (true);

		// close all file
		Carp_RWclose(src_file);
		Carp_RWclose(dest_file);

		return true;
	}

	// 加载文件
	static bool LoadFile(const std::string& path, bool only_asset, std::vector<char>& memory)
	{
		// open src file
		Carp_RWops* file = OpenFile(path, "rb", only_asset);
		if (file == 0) return false;

		// get file size
		int size = (unsigned int)Carp_RWsize(file);
		memory.resize(size, 0);

		// read from memory
		if (size > 0) Carp_RWread(file, &(memory[0]), size, 1);
		// close file
		Carp_RWclose(file);

		return true;
	}

	// 保存到文件
	static bool SaveFile(const char* target_path, const char* content, int size)
	{
		if (content == 0) return false;

		Carp_RWops* file = OpenFile(target_path, "wb", false);
		if (file == 0) return false;

		if (size <= 0) size = static_cast<int>(strlen(content));
		if (size > 0) Carp_RWwrite(file, content, 1, size);
		Carp_RWclose(file);

		return true;
	}
};

class CarpLocalFile
{
public:
	CarpLocalFile() { }
	virtual ~CarpLocalFile() { Clear(); }

public:
	/**
	* set file path
	*/
	void SetPath(const char* path)
	{
		Clear();

		m_path.resize(0);
		if (path == nullptr) return;
		m_path = path;
	}
	/**
	* load from normal dir, if not exist then load from asset dir
	* @return succeed or not
	*/
	bool Load(bool only_assets)
	{
		ClearMemory();

		// open file from local directory first
		Carp_RWops* src_file = CarpRWops::OpenFile(m_path, "rb", only_assets);
		// check exist or not
		if (!src_file) return false;

		// get file size
		m_size = (unsigned int)Carp_RWsize(src_file);

		// malloc memory
		m_memory = (char*)malloc(m_size + 1);
		// write to memory
		Carp_RWread(src_file, (char*)m_memory, m_size, 1);
		// adjust to string
		m_memory[m_size] = 0;
		// close file
		Carp_RWclose(src_file);

		return true;
	}

	/**
	* Decrypt
	*/
	void Decrypt(const char* key)
	{
		if (!m_memory) return;
		CarpCrypto::XXTeaDecodeMemory(m_memory, m_size, key);
	}

	/**
	* Crypt
	*/
	void Encrypt(const char* key)
	{
		if (!m_memory) return;
		CarpCrypto::XXTeaEncodeMemory(m_memory, m_size, key);
	}

	/**
	* clear
	*/
	void Clear()
	{
		m_path = "";
		m_size = 0;

		ClearMemory();
	}

	/**
	* save to
	*/
	void Save(const char* file_path)
	{
		if (!m_memory) return;
		CarpRWops::SaveFile(file_path, m_memory, m_size);
	}

private:
	/**
	* clear
	*/
	void ClearMemory()
	{
		if (m_memory)
		{
			free(m_memory);
			m_memory = 0;
		}

		if (m_read_rwops)
		{
			Carp_RWclose(m_read_rwops);
			m_read_rwops = 0;
		}

		m_cur_offset = 0;
	}

public:
	const char* GetContent() const { return m_memory; }
	const char* GetPath() const { return m_path.c_str(); }
	int GetSize() const { return m_size; }

public:
	char ReadChar(int offset) const
	{
		if (offset + (int)sizeof(char) > m_size) return 0;
		return *(m_memory + offset);
	}
	unsigned int ReadUInt(int offset) const
	{
		if (offset + (int)sizeof(unsigned int) > m_size) return 0;
		return *((unsigned int*)(m_memory + offset));
	}
	int ReadInt(int offset) const
	{
		if (offset + (int)sizeof(int) > m_size) return 0;
		return *((int*)(m_memory + offset));
	}
	float ReadFloat(int offset) const
	{
		if (offset + (int)sizeof(float) > m_size) return 0;
		return *((float*)(m_memory + offset));
	}
	double ReadDouble(int offset) const
	{
		if (offset + (int)sizeof(double) > m_size) return 0;
		return *((double*)(m_memory + offset));
	}

private:
	std::string m_path;		// file path

private:
	char* m_memory = 0;			// content
	int m_size = 0;				// file_size

private:
	int m_cur_offset = 0;
	Carp_RWops* m_read_rwops = nullptr;
};

#endif

#ifdef CARP_SOKOL_RWOPS_HPP_IMPL
#ifndef CARP_SOKOL_RWOPS_HPP_IMPL_INCLUDE
#define CARP_SOKOL_RWOPS_HPP_IMPL_INCLUDE
#define CARP_SOKOL_RWOPS_IMPL
#include "Carp_RWops.h"
#endif
#endif
