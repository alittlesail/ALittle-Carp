#ifndef CARP_FILE_INCLUDED
#define CARP_FILE_INCLUDED

#include <string>
#include <vector>

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
	// 获取当前路径
	static std::string GetCurrentPath()
	{
		std::string result;
#ifdef _WIN32
		wchar_t buffer[1024];
		wchar_t* w_path = _wgetcwd(buffer, 1024);
		if (w_path != nullptr) result = Unicode2UTF8(w_path);
#else
		char buffer[1024];
		char* path = getcwd(buffer, 1024);
		if (path != nullptr) result = path;
#endif
		return result;
	}
	// 设置当前路径
	static bool SetCurrentPath(const std::string& path)
	{
#ifdef _WIN32
		return _wchdir(UTF82Unicode(path).c_str()) == 0;
#else
		return chdir(path.c_str()) == 0;
#endif
	}
	
	// 创建文件夹
	static void CreateFolder(const std::string& path)
	{
#ifdef _WIN32
		_wmkdir(UTF82Unicode(path).c_str());
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
		//文件句柄
		std::intptr_t h_file;
		//文件信息
		struct _wfinddata_t file_info{};
		std::wstring w_path = UTF82Unicode(path);
		w_path += L"\\*";
		if ((h_file = _wfindfirst(w_path.c_str(), &file_info)) != -1)
		{
			do
			{
				if (file_info.attrib & _A_SUBDIR)
				{
					if (wcscmp(file_info.name, L".") != 0 && wcscmp(file_info.name, L"..") != 0)
						dir_list.push_back(Unicode2UTF8(file_info.name));
				}
				else
				{
					file_list.push_back(Unicode2UTF8(file_info.name));
				}
			} while (_wfindnext(h_file, &file_info) == 0);
			_findclose(h_file);
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
		const auto pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return "";

		return file_path.substr(pos + 1);
	}
	// 改变扩展名
	static std::string ChangeFileExtByPath(const std::string& file_path, const std::string& ext)
	{
		const auto pos = file_path.find_last_of('.');
		if (pos == std::string::npos) return file_path + "." + ext;

		return file_path.substr(0, pos) + "." + ext;
	}

	// 从路径中获取文件名
	static std::string GetFileNameByPath(const std::string& file_path)
	{
		size_t index = 0;
		for (auto i = file_path.size(); i > 0; --i)
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
		for (auto i = file_path.size(); i > 0; --i)
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

		const auto pos = result.find_last_of('.');
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
			const auto read_size = fread(buffer, 1, sizeof(buffer), file);
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
	// 获取所有盘符
	static void GetDriveStrings(std::vector<std::string>& result)
	{
#ifdef _WIN32
		DWORD dwLen = GetLogicalDriveStrings(0, NULL);	//获取系统字符串长度.
		std::vector<char> vecDriver;				//构建一个相应长度的数组.
		vecDriver.resize(dwLen, 0);
		GetLogicalDriveStringsA(dwLen, vecDriver.data());		//获取盘符字符串.
		char* pDriver = vecDriver.data();
		while (*pDriver != '\0')
		{
			result.push_back(pDriver);
			pDriver += strlen(pDriver) + 1;			//定位到下一个字符串.加一是为了跳过'\0'字符串.
		}
#else
#endif
	}

#ifdef WIN32
	static std::wstring UTF82Unicode(const std::string& utf8)
	{
		const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		std::wstring result;
		if (len >= 1) result.resize(len - 1);
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, const_cast<wchar_t*>(result.c_str()), len);
		return result;
	}

	static std::string Unicode2UTF8(const std::wstring& unicode)
	{
		const int len = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string result;
		if (len >= 1) result.resize(len -1);
		WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, const_cast<char*>(result.c_str()), len, nullptr, nullptr);
		return result;
	}
#endif
};

#endif