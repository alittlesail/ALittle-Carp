#ifndef CARP_RWOPS_HPP_INCLUDED
#define CARP_RWOPS_HPP_INCLUDED

#ifdef HAS_SDL
#include <SDL.h>
#endif
#include <string>
#include <vector>

#include "carp_crypto.hpp"

#ifndef HAS_SDL
#define SDL_RWclose fclose
#define SDL_RWread(file, buffer, e_size, e_count) fread(buffer, e_size, e_count, file)
#define SDL_RWwrite(file, buffer, e_size, e_count) fwrite(buffer, e_size, e_count, file)
#endif

class CarpRWops
{
public:
	// 获取基本路径
	static std::string BaseFilePath()
	{
		std::string base_path;
#ifdef __ANDROID__
		base_path.append(SDL_AndroidGetInternalStoragePath()).append("/");
		return base_path;
#elif __IPHONEOS__
		@autoreleasepool
		{
			NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString* path = [paths objectAtIndex : 0];
			base_path.append([path UTF8String]).append("/");
		}
		return base_path;
#elif _WIN32
		return base_path;
#else
		return base_path;
#endif
	}

	// 获取外部路径
	static std::string ExternalFilePath()
	{
#ifdef __ANDROID__
		std::string external_path;
		external_path.append(SDL_AndroidGetExternalStoragePath()).append("/");
		return external_path;
#elif __IPHONEOS__
		return BaseFilePath();
#elif _WIN32
		return BaseFilePath();
#else
		return BaseFilePath();
#endif
	}

	// 计算文件的md5
	static std::string FileMd5(const char* file_path)
	{
		auto* file = OpenFile(file_path, "rb");
		if (file == nullptr) return "";

		CarpCrypto::MD5_HASH digest;
		CarpCrypto::Md5Context context;

		CarpCrypto::Md5Initialise(&context);

		unsigned char buffer[1024];
		while (true)
		{
			const size_t read_size = SDL_RWread(file, buffer, 1, sizeof(buffer));
			if (read_size == 0) break;
			CarpCrypto::Md5Update(&context, buffer, static_cast<int>(read_size));
		}
		SDL_RWclose(file);

		CarpCrypto::Md5Finalise(&context, &digest);
		return CarpCrypto::Md4HashToString(&digest);
	}

	// 打开文件
#ifdef HAS_SDL
	static SDL_RWops* OpenFile(const std::string& path, const char* mode)
	{
		return SDL_RWFromFile(path.c_str(), mode);
	}
#else
	static FILE* OpenFile(const std::string& path, const char* mode)
	{
		FILE* file = nullptr;
#ifdef _WIN32
#pragma warning(push)  
#pragma warning(disable:4996)
		file = fopen(path.c_str(), mode);
#pragma warning(pop)
#else
		file = fopen(path.c_str(), mode);
#endif
		return file;
	}
#endif

	// 复制文件
	static bool CpFile(const char* src_path, const char* dest_path)
	{
		if (src_path == nullptr || dest_path == nullptr) return false;

		// open src file
		auto* src_file = OpenFile(src_path, "rb");
		if (src_file == nullptr) return false;

		// open dest file
		auto* dest_file = OpenFile(dest_path, "wb");
		if (dest_file == nullptr)
		{
			SDL_RWclose(src_file);
			return false;
		}

		// define buffer, read and write
		char buff[1024];
		size_t read_size = 0;
		do
		{
			read_size = SDL_RWread(src_file, buff, 1, sizeof(buff));
			if (read_size == 0) break;
			SDL_RWwrite(dest_file, buff, 1, read_size);
		} while (true);

		// close all file
		SDL_RWclose(src_file);
		SDL_RWclose(dest_file);

		return true;
	}

	// 加载文件
	static bool LoadFile(const std::string& path, std::vector<char>& memory)
	{
		// open src file
		auto* file = OpenFile(path, "rb");
		if (file == nullptr) return false;

		// get file size
#ifdef HAS_SDL
		const int size = static_cast<int>(SDL_RWsize(file));
#else
		fseek(file, 0, SEEK_END);
		const int size = static_cast<int>(ftell(file));
		fseek(file, 0, SEEK_SET);
#endif
		memory.resize(size, 0);

		// read from memory
		if (size > 0) SDL_RWread(file, &(memory[0]), size, 1);
		// close file
		SDL_RWclose(file);

		return true;
	}

	// 保存到文件
	static bool SaveFile(const char* target_path, const char* content, int size)
	{
		if (content == nullptr) return false;

		auto* file = OpenFile(target_path, "wb");
		if (file == nullptr) return false;

		if (size <= 0) size = static_cast<int>(strlen(content));
		if (size > 0) SDL_RWwrite(file, content, 1, size);
		SDL_RWclose(file);

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
	bool Load()
	{
		ClearMemory();

		// open file from local directory first
		auto* src_file = CarpRWops::OpenFile(m_path, "rb");
		// check exist or not
		if (!src_file) return false;

		// get file size
#ifdef HAS_SDL
		m_size = static_cast<unsigned int>(SDL_RWsize(src_file));
#else
		fseek(src_file, 0, SEEK_END);
		m_size = static_cast<unsigned int>(ftell(src_file));
		fseek(src_file, 0, SEEK_SET);
#endif

		// malloc memory
		m_memory = static_cast<char*>(malloc(m_size + 1));
		// write to memory
		SDL_RWread(src_file, static_cast<char*>(m_memory), m_size, 1);
		// adjust to string
		m_memory[m_size] = 0;
		// close file
		SDL_RWclose(src_file);

		return true;
	}

	/**
	* Decrypt
	*/
	void Decrypt(const char* key) const
	{
		if (!m_memory) return;
		CarpCrypto::XXTeaDecodeMemory(m_memory, m_size, key);
	}

	/**
	* Crypt
	*/
	void Encrypt(const char* key) const
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
	void Save(const char* file_path) const
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
			m_memory = nullptr;
		}

		if (m_read_rwops)
		{
			SDL_RWclose(m_read_rwops);
			m_read_rwops = nullptr;
		}

		m_cur_offset = 0;
	}

public:
	const char* GetContent() const { return m_memory; }
	const char* GetPath() const { return m_path.c_str(); }
	int GetSize() const { return m_size; }

public:
	unsigned char ReadUChar(int offset) const
	{
		if (offset + static_cast<int>(sizeof(unsigned char)) > m_size) return 0;
		return *reinterpret_cast<unsigned char*>(m_memory + offset);
	}
	char ReadChar(int offset) const
	{
		if (offset + static_cast<int>(sizeof(char)) > m_size) return 0;
		return *(m_memory + offset);
	}
	unsigned short ReadUShort(int offset) const
	{
		if (offset + static_cast<int>(sizeof(unsigned short)) > m_size) return 0;
		return *reinterpret_cast<unsigned short*>(m_memory + offset);
	}
	int ReadShort(int offset) const
	{
		if (offset + static_cast<int>(sizeof(short)) > m_size) return 0;
		return *reinterpret_cast<short*>(m_memory + offset);
	}
	unsigned int ReadUInt(int offset) const
	{
		if (offset + static_cast<int>(sizeof(unsigned int)) > m_size) return 0;
		return *reinterpret_cast<unsigned int*>(m_memory + offset);
	}
	int ReadInt(int offset) const
	{
		if (offset + static_cast<int>(sizeof(int)) > m_size) return 0;
		return *reinterpret_cast<int*>(m_memory + offset);
	}
	float ReadFloat(int offset) const
	{
		if (offset + static_cast<int>(sizeof(float)) > m_size) return 0;
		return *reinterpret_cast<float*>(m_memory + offset);
	}
	double ReadDouble(int offset) const
	{
		if (offset + static_cast<int>(sizeof(double)) > m_size) return 0;
		return *reinterpret_cast<double*>(m_memory + offset);
	}

private:
	std::string m_path;		// file path

private:
	char* m_memory = nullptr;			// content
	int m_size = 0;				// file_size

private:
	int m_cur_offset = 0;
#ifdef HAS_SDL
	SDL_RWops* m_read_rwops = nullptr;
#else
	FILE* m_read_rwops = nullptr;
#endif
};

#endif
