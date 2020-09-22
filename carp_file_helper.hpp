#ifndef CARP_FILEHELPER_INCLUDED
#define CARP_FILEHELPER_INCLUDED (1)

#include <string>
#include <vector>

#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <sys/locking.h>
#ifdef __BORLANDC__
#include <utime.h>
#else
#include <sys/utime.h>
#endif
#include <fcntl.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>
#endif

class FileHelper
{
public:
	// �����ļ���
	static void CreateFolder(const std::string& path)
	{
#ifdef _WIN32
		int result = _mkdir(path.c_str());
#else
		mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
			S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	}

	// �ݹ鴴���ļ���
	static void CreateDeepFolder(const std::string& path)
	{
		if (path.empty()) return;

		std::string sub_path;
		for (size_t i = 0; i < path.size(); ++i)
		{
			if (path[i] == '/' || path[i] == '\\')
				CreateFolder(sub_path);
			sub_path.push_back(path[i]);
		}
		CreateFolder(sub_path);
	}

	// �ж��ļ��Ƿ����
	static bool IsFileExist(const std::string& path)
	{
#ifdef _WIN32
#define STAT_STRUCT struct _stati64
#define STAT_FUNC _stati64
#else
#define STAT_STRUCT struct stat
#define STAT_FUNC stat
#endif
		STAT_STRUCT buffer;
		if (STAT_FUNC(path.c_str(), &buffer)) return false;
		return (buffer.st_mode & S_IFREG) != 0;
	}

	// ��·���л�ȡ�ļ���չ��
	static std::string GetFileExtByPath(const std::string& file_path)
	{
		std::string::size_type pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return "";

		return file_path.substr(pos + 1);
	}

	// ��·���л�ȡ�ļ���
	static std::string GetFileNameByPath(const std::string& file_path)
	{
		size_t index = 0;
		for (size_t i = file_path.size(); i > 0; --i)
		{
			if (file_path[i - 1] == '/' || file_path[i - 1] == '\\')
			{
				index = i;
				break;
			}
		}
		return file_path.substr(index);
	}

	// ��·���л�ȡ�ļ���(������׺)
	static std::string GetJustFileNameByPath(const std::string& file_path)
	{
		std::string result = GetFileNameByPath(file_path);

		std::string::size_type pos = result.find_last_of('.');
		if (pos == std::string::npos) return result;

		return result.substr(0, pos);
	}

	// Ϊ�ļ���β���·���ָ���
	static std::string TryAddFileSeparator(const std::string& file_path)
	{
		if (file_path.empty()) return file_path;
		if (file_path.back() == '/' || file_path.back() == '\\') return file_path;
		if (file_path.find('\\') != std::string::npos) return file_path + "\\";
		return file_path + "/";
	}

	// �����ļ�
	static bool LoadStdFile(const std::string& file_path, std::vector<char>& out)
	{
#ifdef _WIN32
		FILE* file = 0;
		fopen_s(&file, file_path.c_str(), "rb");
#else
		FILE* file = fopen(file_path.c_str(), "rb");
#endif
		if (file == nullptr) return false;

		char buffer[1024];
		while (true)
		{
			const size_t read_size = fread(buffer, 1, sizeof(buffer), file);
			if (read_size == 0) break;
			for (size_t i = 0; i < read_size; ++i)
				out.push_back(buffer[i]);
		}
		fclose(file);

		return true;
	}
};

#endif