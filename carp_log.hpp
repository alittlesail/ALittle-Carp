#ifndef CARP_LOG_INCLUDED
#define CARP_LOG_INCLUDED (1)

#include "carp_thread_consumer.hpp"
#include "carp_time_helper.hpp"
#include "carp_string_helper.hpp"

#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>

enum CarpLogColor
{
	CARP_LOG_COLOR_ERROR = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | FOREGROUND_RED,
	CARP_LOG_COLOR_SYSTEM = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_BLUE,
	CARP_LOG_COLOR_DATABASE = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
	CARP_LOG_COLOR_EVENT = FOREGROUND_RED | FOREGROUND_GREEN,
	CARP_LOG_COLOR_WARN = FOREGROUND_RED | FOREGROUND_GREEN,
	CARP_LOG_COLOR_INFO = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};

#endif // _WIN32

enum CarpLogLevel
{
	CARP_LOG_LEVEL_INFO = 0,
	CARP_LOG_LEVEL_WARN = 1,
	CARP_LOG_LEVEL_ERROR = 2,
	CARP_LOG_LEVEL_SYSTEM = 3,
	CARP_LOG_LEVEL_DATABASE = 4,
	CARP_LOG_LEVEL_EVENT = 5,
};

// 日志信息结构体
struct CarpLogInfo
{
	std::string content;		// 日志内容
	short level = 0;				// 级别
};

class CarpLog : public CarpThreadConsumer<CarpLogInfo>
{
public:
	// 设置日志文件名前缀
	void Setup(const std::string& path, const std::string& name)
	{
		// 保存文件名
		m_file_name = name;
		m_file_path = path;

#ifdef _WIN32
		// 获取控制台句柄
		m_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // _WIN32

		// 启动
		Start();
	}

private:
	// 日志文件前缀
	std::string m_file_name;
	std::string m_file_path;

public:
	// 日志
	// content 表示日志的内容
	void Log(const char* content, short level)
	{
		// 如果还未启动直接返回
		if (!IsStart())
		{
			printf("%s\n", content);
			return;
		}

		// 先从内存池中获取对象，如果没有就创建一个
		CarpLogInfo log;
		// 保存颜色
		log.level = level;
		// 设置当前时间
		CarpTimeHelper::FormatTime(CarpTimeHelper::GetCurTime(), 0, '-', &(log.content), ':');
		// 添加一个空格
		log.content.push_back(' ');
		// 添加日志内容
		log.content.append(content);
		// 添加换行
		log.content.push_back('\n');
		// 把日志添加到列表
		Add(log);
	}

	// 关闭日志系统
	void Shutdown()
	{
		// 关闭
		Stop();

		// 关闭文件
		if (m_file)
		{
			fclose(m_file);
			m_file = nullptr;
		}
	}

protected:
	// 执行日志
	virtual void Flush(CarpLogInfo& info) override
	{
		// 获取现在的时间
		time_t cur_time = time(0);
		// 计算是不是第二天了
		if (cur_time - m_cur_day >= 60 * 60 * 24)
		{
			if (m_file != nullptr)
			{
				fclose(m_file);
				m_file = nullptr;
			}

			std::string YMD, HMS;
			CarpTimeHelper::FormatTime(cur_time, &YMD, '-', &HMS, '-');

			// 创建一个新的文件对象
			std::string file_path;
			file_path.append(m_file_path).append(m_file_name).append("_").append(YMD).append("_").append(HMS).append(".log");
#ifdef _WIN32
			_wfopen_s(&m_file, CarpStringHelper::UTF82Unicode(file_path).c_str(), L"a");
#else
			m_file = fopen(file_path.c_str(), "a");
#endif

			if (m_file == nullptr)
				printf("log file open failed:%s", file_path.c_str());

			// 把最新时间的0点保存起来
			m_cur_day = CarpTimeHelper::CalcTodayBeginTime(cur_time);
		}

		if (m_file == nullptr)
			return;

		// 设置控制台颜色
#ifdef _WIN32
		if (info.level == CARP_LOG_LEVEL_INFO)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_INFO);
		else if (info.level == CARP_LOG_LEVEL_WARN)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_WARN);
		else if (info.level == CARP_LOG_LEVEL_ERROR)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_ERROR);
		else if (info.level == CARP_LOG_LEVEL_SYSTEM)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_SYSTEM);
		else if (info.level == CARP_LOG_LEVEL_DATABASE)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_DATABASE);
		else if (info.level == CARP_LOG_LEVEL_EVENT)
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_EVENT);
		else
			SetConsoleTextAttribute(m_out, CARP_LOG_COLOR_INFO);
		wprintf(CarpStringHelper::UTF82Unicode(info.content).c_str());
		SetConsoleTextAttribute(m_out, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#elif __ANDROID__
		if (info.color == CARP_LOG_LEVEL_ERROR)
			__android_log_write(ANDROID_LOG_ERROR, "SDL", info.content.c_str());
		else if (info.color == CARP_LOG_LEVEL_WARN)
			__android_log_write(ANDROID_LOG_WARN, "SDL", info.content.c_str());
		else
			__android_log_write(ANDROID_LOG_INFO, "SDL", info.content.c_str());
#else
		printf("%s", info.content.c_str());
#endif
		// 写入到文件
		std::fwrite(info.content.data(), 1, info.content.size(), m_file);
	}

private:
	FILE* m_file = nullptr;			// 日志文件对象
	time_t m_cur_day = 0;		// 当天0点的时间

#ifdef _WIN32
	HANDLE m_out = NULL;		// 控制台句柄
#endif // _WIN32
};

extern CarpLog& CarpLogInstance();
#define s_carp_log CarpLogInstance()

// 属于严重的代码错误，属于非常意外的，没想到会在这里出错的
#define CARP_ERROR(text) do{ std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << ":" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); }while(0)
// 属于系统级别的错误，这种错误是预知的，已经在系统流程规划之内，也是无关紧要的日志
#define CARP_SYSTEM(text) do{ std::ostringstream astream; astream << "SYSTEM:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_SYSTEM); }while(0)
// 属于数据库错误，属于严重错误
#define CARP_DATABASE(text) do{ std::ostringstream astream; astream << "DATABASE:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_DATABASE); }while(0)
// 事件性
#define CARP_EVENT(text) do{ std::ostringstream astream; astream << "EVENT:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_EVENT); }while(0)
// 警告，比如负载很高了，压力很大了
#define CARP_WARN(text) do{ std::ostringstream astream; astream << "WARN:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_WARN); }while(0)
// 单纯的调试打印
#define CARP_INFO(text) do{ std::ostringstream astream; astream << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_INFO); }while(0)
// 属于严重的代码错误，属于非常意外的，没想到会在这里出错的，提供给脚本系统使用
#define CARP_SCRIPT_ERROR(text) do{ std::ostringstream astream; astream << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); }while(0)

#endif

#ifdef CARP_LOG_IMPL
CarpLog& CarpLogInstance() { static CarpLog instance; return instance; };
#endif

