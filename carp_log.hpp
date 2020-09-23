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

// ��־��Ϣ�ṹ��
struct CarpLogInfo
{
	std::string content;		// ��־����
	short level = 0;				// ����
};

class CarpLog : public CarpThreadConsumer<CarpLogInfo>
{
public:
	// ������־�ļ���ǰ׺
	void Setup(const std::string& path, const std::string& name)
	{
		// �����ļ���
		m_file_name = name;
		m_file_path = path;

#ifdef _WIN32
		// ��ȡ����̨���
		m_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // _WIN32

		// ����
		Start();
	}

private:
	// ��־�ļ�ǰ׺
	std::string m_file_name;
	std::string m_file_path;

public:
	// ��־
	// content ��ʾ��־������
	void Log(const char* content, short level)
	{
		// �����δ����ֱ�ӷ���
		if (!IsStart())
		{
			printf("%s\n", content);
			return;
		}

		// �ȴ��ڴ���л�ȡ�������û�оʹ���һ��
		CarpLogInfo log;
		// ������ɫ
		log.level = level;
		// ���õ�ǰʱ��
		CarpTimeHelper::FormatTime(CarpTimeHelper::GetCurTime(), 0, '-', &(log.content), ':');
		// ���һ���ո�
		log.content.push_back(' ');
		// �����־����
		log.content.append(content);
		// ��ӻ���
		log.content.push_back('\n');
		// ����־��ӵ��б�
		Add(log);
	}

	// �ر���־ϵͳ
	void Shutdown()
	{
		// �ر�
		Stop();

		// �ر��ļ�
		if (m_file)
		{
			fclose(m_file);
			m_file = nullptr;
		}
	}

protected:
	// ִ����־
	virtual void Flush(CarpLogInfo& info) override
	{
		// ��ȡ���ڵ�ʱ��
		time_t cur_time = time(0);
		// �����ǲ��ǵڶ�����
		if (cur_time - m_cur_day >= 60 * 60 * 24)
		{
			if (m_file != nullptr)
			{
				fclose(m_file);
				m_file = nullptr;
			}

			std::string YMD, HMS;
			CarpTimeHelper::FormatTime(cur_time, &YMD, '-', &HMS, '-');

			// ����һ���µ��ļ�����
			std::string file_path;
			file_path.append(m_file_path).append(m_file_name).append("_").append(YMD).append("_").append(HMS).append(".log");
#ifdef _WIN32
			_wfopen_s(&m_file, CarpStringHelper::UTF82Unicode(file_path).c_str(), L"a");
#else
			m_file = fopen(file_path.c_str(), "a");
#endif

			if (m_file == nullptr)
				printf("log file open failed:%s", file_path.c_str());

			// ������ʱ���0�㱣������
			m_cur_day = CarpTimeHelper::CalcTodayBeginTime(cur_time);
		}

		if (m_file == nullptr)
			return;

		// ���ÿ���̨��ɫ
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
		// д�뵽�ļ�
		std::fwrite(info.content.data(), 1, info.content.size(), m_file);
	}

private:
	FILE* m_file = nullptr;			// ��־�ļ�����
	time_t m_cur_day = 0;		// ����0���ʱ��

#ifdef _WIN32
	HANDLE m_out = NULL;		// ����̨���
#endif // _WIN32
};

extern CarpLog& CarpLogInstance();
#define s_carp_log CarpLogInstance()

// �������صĴ���������ڷǳ�����ģ�û�뵽������������
#define CARP_ERROR(text) do{ std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << ":" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); }while(0)
// ����ϵͳ����Ĵ������ִ�����Ԥ֪�ģ��Ѿ���ϵͳ���̹滮֮�ڣ�Ҳ���޹ؽ�Ҫ����־
#define CARP_SYSTEM(text) do{ std::ostringstream astream; astream << "SYSTEM:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_SYSTEM); }while(0)
// �������ݿ�����������ش���
#define CARP_DATABASE(text) do{ std::ostringstream astream; astream << "DATABASE:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_DATABASE); }while(0)
// �¼���
#define CARP_EVENT(text) do{ std::ostringstream astream; astream << "EVENT:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_EVENT); }while(0)
// ���棬���縺�غܸ��ˣ�ѹ���ܴ���
#define CARP_WARN(text) do{ std::ostringstream astream; astream << "WARN:" << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_WARN); }while(0)
// �����ĵ��Դ�ӡ
#define CARP_INFO(text) do{ std::ostringstream astream; astream << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_INFO); }while(0)
// �������صĴ���������ڷǳ�����ģ�û�뵽�����������ģ��ṩ���ű�ϵͳʹ��
#define CARP_SCRIPT_ERROR(text) do{ std::ostringstream astream; astream << text; s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); }while(0)

#endif

#ifdef CARP_LOG_IMPL
CarpLog& CarpLogInstance() { static CarpLog instance; return instance; };
#endif

