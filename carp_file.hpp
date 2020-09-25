#ifndef CARP_FILE_INCLUDED
#define CARP_FILE_INCLUDED (1)

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

#ifdef _WIN32
#define CARP_STAT_STRUCT struct _stati64
#define CARP_STAT_FUNC _wstati64
#else
#define CARP_STAT_STRUCT struct stat
#define CARP_STAT_FUNC stat
#endif

class CarpFile
{
public:
	// 创建文件夹
	static void CreateFolder(const std::string& path)
	{
#ifdef _WIN32
		int result = _wmkdir(UTF82Unicode(path).c_str());
#else
		mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
			S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	}
	// 删除文件夹
	static void DeleteFolder(const std::string& path)
	{
#ifdef _WIN32
		_wrmdir(UTF82Unicode(path).c_str());
#else
		rmdir(path.c_str());
#endif
	}

	// 递归创建文件夹
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

	// 判断文件是否存在
	static bool IsFileExist(const std::string& path)
	{
		CARP_STAT_STRUCT buffer;
#ifdef _WIN32
		if (CARP_STAT_FUNC(UTF82Unicode(path).c_str(), &buffer)) return false;
#else
		if (CARP_STAT_FUNC(path.c_str(), &buffer)) return false;
#endif
		return (buffer.st_mode & S_IFREG) != 0;
	}

	// 判断文件夹是否存在
	static bool IsDirExist(const std::string& path)
	{
		CARP_STAT_STRUCT buffer;
#ifdef _WIN32
		if (CARP_STAT_FUNC(UTF82Unicode(path).c_str(), &buffer)) return false;
#else
		if (CARP_STAT_FUNC(path.c_str(), &buffer)) return false;
#endif
		return (buffer.st_mode & S_IFDIR) != 0;
	}

	struct PathAttribute
	{
		bool directory = false;	// 是否是路径
		time_t modify_time = 0;	// 最后修改时间
		time_t create_time = 0;	// 创建时间
		size_t size = 0;		// 文件大小
	};
	
	// 获取文件属性
	static bool GetPathAttribute(const std::string& path, PathAttribute& attr)
	{
		CARP_STAT_STRUCT buffer;
#ifdef _WIN32
		if (CARP_STAT_FUNC(UTF82Unicode(path).c_str(), &buffer)) return false;
#else
		if (CARP_STAT_FUNC(path.c_str(), &buffer)) return false;
#endif

		attr.directory = (buffer.st_mode & S_IFDIR) != 0;
		attr.modify_time = buffer.st_mtime;
		attr.create_time = buffer.st_ctime;
		attr.size = buffer.st_size;

		return true;
	}

	// 获取文件夹下的所有文件
	static void GetNameListInFolder(const std::string& path, std::vector<std::string>& file_list, std::vector<std::string>& dir_list)
	{
#ifdef _WIN32
		std::wstring wpath = UTF82Unicode(path);
		//文件句柄
		std::intptr_t   hFile = 0;
		//文件信息
		struct _wfinddata_t fileinfo;
		std::wstring p;
		if ((hFile = _wfindfirst(p.assign(wpath).append(L"\\*").c_str(), &fileinfo)) != -1)
		{
			do
			{
				if (fileinfo.attrib & _A_SUBDIR)
				{
					if (wcscmp(fileinfo.name, L".") != 0 && wcscmp(fileinfo.name, L"..") != 0)
						dir_list.push_back(Unicode2UTF8(fileinfo.name));
				}
				else
				{
					file_list.push_back(Unicode2UTF8(fileinfo.name));
				}
			} while (_wfindnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
#else
		DIR* dir;
		struct    dirent* ptr;
		dir = opendir(path.c_str()); ///open the dir
		if (dir != NULL)
		{
			while ((ptr = readdir(dir)) != NULL) ///read the list of this dir
			{
				if (ptr->d_type == DT_DIR)
				{
					if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
						dir_list.push_back(ptr->d_name);
				}
				else
				{
					file_list.push_back(ptr->d_name);
				}
			}
			closedir(dir);
		}
#endif
	}

	// 从路径中获取文件扩展名
	static std::string GetFileExtByPath(const std::string& file_path)
	{
		std::string::size_type pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return "";

		return file_path.substr(pos + 1);
	}
	// 改变扩展名
	static std::string ChangeFileExtByPath(const std::string& file_path, const std::string& ext)
	{
		std::string::size_type pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return file_path + "." + ext;

		return file_path.substr(0, pos) + "." + ext;
	}

	// 从路径中获取文件名
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

	// 从路径中获取文件名
	static std::string GetFilePathByPath(const std::string& file_path)
	{
		size_t index = 0;
		for (size_t i = file_path.size(); i > 0; --i)
		{
			if (file_path[i - 1] == '/' || file_path[i - 1] == '\\')
			{
				index = i - 1;
				break;
			}
		}
		return file_path.substr(0, index);
	}

	// 从路径中获取文件名(不带后缀)
	static std::string GetJustFileNameByPath(const std::string& file_path)
	{
		std::string result = GetFileNameByPath(file_path);

		std::string::size_type pos = result.find_last_of('.');
		if (pos == std::string::npos) return result;

		return result.substr(0, pos);
	}

	// 为文件结尾添加路径分隔符
	static std::string TryAddFileSeparator(const std::string& file_path)
	{
		if (file_path.empty()) return file_path;
		if (file_path.back() == '/' || file_path.back() == '\\') return file_path;
		if (file_path.find('\\') != std::string::npos) return file_path + "\\";
		return file_path + "/";
	}

	// 加载文件
	static bool LoadStdFile(const std::string& file_path, std::vector<char>& out)
	{
#ifdef _WIN32
		FILE* file = 0;
		_wfopen_s(&file, UTF82Unicode(file_path).c_str(), L"rb");
#else
		FILE* file = fopen(file_path.c_str(), "rb");
#endif
		if (file == nullptr) return false;

		char buffer[1024];
		while (true)
		{
			size_t read_size = fread(buffer, 1, sizeof(buffer), file);
			if (read_size == 0) break;
			for (size_t i = 0; i < read_size; ++i)
				out.push_back(buffer[i]);
		}
		fclose(file);

		return true;
	}
	// 写入文件
	static bool WriteToStdFile(const std::string& file_path, const std::vector<char>& out)
	{
#ifdef _WIN32
		FILE* file = 0;
		_wfopen_s(&file, UTF82Unicode(file_path).c_str(), L"wb");
#else
		FILE* file = fopen(file_path.c_str(), "wb");
#endif
		if (file == nullptr) return false;

		if (out.size() > 0) fwrite(out.data(), 1, out.size(), file);

		fclose(file);
		return true;
	}

#ifdef WIN32
	static std::wstring UTF82Unicode(const std::string& utf8)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
		std::wstring result;
		result.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, (wchar_t*)result.c_str(), len);
		return result;
	}

	static std::string Unicode2UTF8(const std::wstring& unicode)
	{
		int len = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
		std::string result;
		result.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, (char*)result.c_str(), len, NULL, NULL);
		return result;
	}
#endif
};

#endif