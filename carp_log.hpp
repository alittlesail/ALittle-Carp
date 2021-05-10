#ifndef CARP_LOG_INCLUDED
#define CARP_LOG_INCLUDED

#include "carp_thread_consumer.hpp"
#include "carp_time.hpp"

#include <string>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#endif

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
	CARP_LOG_LEVEL_INFO		= 0,
	CARP_LOG_LEVEL_WARN		= 1,
	CARP_LOG_LEVEL_ERROR	= 2,
	CARP_LOG_LEVEL_SYSTEM	= 3,
	CARP_LOG_LEVEL_DATABASE	= 4,
	CARP_LOG_LEVEL_EVENT	= 5,
};

// ��־��Ϣ�ṹ��
struct CarpLogInfo
{
	std::string content;		// ��־����
	short level = 0;				// ����
};

class CarpLog : public CarpThreadConsumer<CarpLogInfo, 10000>
{
public:
	// ������־�ļ���ǰ׺
	void Setup(const std::string& path, const std::string& name, bool print)
	{
		// �����ļ���
		m_file_name = name;
		m_file_path = path;
		m_print = print;

#ifdef _WIN32
		// ��ȡ����̨���
		if (m_print) m_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#ifndef __EMSCRIPTEN__
		// ����
		Start();
#endif
	}

private:
	// ��־�ļ�ǰ׺
	std::string m_file_name;
	std::string m_file_path;
	bool m_print = true;

#ifdef _WIN32
	static std::string UTF82ANSI(const std::string& utf8)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
		std::vector<WCHAR> wszGBK;
		wszGBK.resize(len + 1, 0);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8.c_str(), -1, wszGBK.data(), static_cast<int>(len));

		len = WideCharToMultiByte(CP_ACP, 0, wszGBK.data(), -1, NULL, 0, NULL, NULL);
		std::vector<char> szGBK;
		szGBK.resize(len + 1, 0);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK.data(), -1, szGBK.data(), static_cast<int>(len), NULL, NULL);
		return szGBK.data();
	}
#endif
	
public:
	// ��־
	// content ��ʾ��־������
	void Log(const char* content, short level)
	{
		// �����δ����ֱ�ӷ���
		if (!IsStart())
		{
			if (m_print)
			{
#ifdef _WIN32
				printf("%s\n", UTF82ANSI(content).c_str());
#else
				printf("%s\n", content);
#endif		
			}
			return;
		}

		// �ȴ��ڴ���л�ȡ�������û�оʹ���һ��
		CarpLogInfo log;
		// ������ɫ
		log.level = level;
		// ���õ�ǰʱ��
		CarpTime::FormatTime(CarpTime::GetCurTime(), nullptr, '-', &(log.content), ':');
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
	virtual void Execute(CarpLogInfo& info) override
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
			CarpTime::FormatTime(cur_time, &YMD, '-', &HMS, '-');

			// ����һ���µ��ļ�����
			std::string file_path;
			file_path.append(m_file_path).append(m_file_name).append("_").append(YMD).append("_").append(HMS).append(".log");
#ifdef _WIN32
			fopen_s(&m_file, UTF82ANSI(file_path).c_str(), "a");
#else
			m_file = fopen(file_path.c_str(), "a");
#endif

			if (m_file == nullptr)
				printf("log file open failed:%s\n", file_path.c_str());

			// ������ʱ���0�㱣������
			m_cur_day = CarpTime::CalcTodayBeginTime(cur_time);
		}

		if (m_file == nullptr) return;
		// д�뵽�ļ�
		std::fwrite(info.content.data(), 1, info.content.size(), m_file);

		// ��ӡ������̨
		if (m_print)
		{
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
			printf("%s", UTF82ANSI(info.content).c_str());
			SetConsoleTextAttribute(m_out, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#elif __ANDROID__
			if (info.level == CARP_LOG_LEVEL_ERROR)
				__android_log_write(ANDROID_LOG_ERROR, "SDL", info.content.c_str());
			else if (info.level == CARP_LOG_LEVEL_WARN)
				__android_log_write(ANDROID_LOG_WARN, "SDL", info.content.c_str());
			else
				__android_log_write(ANDROID_LOG_INFO, "SDL", info.content.c_str());
#else
			printf("%s", info.content.c_str());
#endif
		}
	}

	void Flush() override
	{
		if (m_file) std::fflush(m_file);
	}

private:
	FILE* m_file = nullptr;			// ��־�ļ�����
	time_t m_cur_day = 0;		// ����0���ʱ��

#ifdef _WIN32
	HANDLE m_out = nullptr;		// ����̨���
#endif // _WIN32
};

extern CarpLog s_carp_log;

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

#define VA_CARP_LOG_CHECK_EXPAND(...)      __VA_ARGS__

#define VA_CARP_LOG_CHECK_FILTER_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_N,...) _N
#define VA_CARP_LOG_CHECK_NUMBER_()        52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
#define VA_CARP_LOG_CHECK_HELPER(...)      VA_CARP_LOG_CHECK_EXPAND(VA_CARP_LOG_CHECK_FILTER_(__VA_ARGS__))
#define VA_CARP_LOG_CHECK_COUNT(...)       VA_CARP_LOG_CHECK_HELPER(__VA_ARGS__, VA_CARP_LOG_CHECK_NUMBER_())

#define VA_CARP_LOG_CHECK_CAT(X, Y)        X##Y
#define VA_CARP_LOG_CHECK_JOIN(X, Y)       VA_CARP_LOG_CHECK_CAT(X, Y)
#define VA_CARP_LOG_CHECK_PROXY(F, ...)    VA_CARP_LOG_CHECK_EXPAND(F(__VA_ARGS__))

#define CARP_LOG_CHECK_MACRO_1(c) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c; \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return; \
    } \
} while(0)

#define CARP_LOG_CHECK_MACRO_2(c, p0) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return; \
    } \
} while(0)

#define CARP_LOG_CHECK_MACRO_3(c, p0, p1) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
		<< ", " << #p1 << ":" << (p1); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return; \
    } \
} while(0)

#define CARP_LOG_CHECK_MACRO_4(c, p0, p1, p2) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
		<< ", " << #p1 << ":" << (p1); \
		<< ", " << #p2 << ":" << (p2); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return; \
    } \
} while(0)

// ������԰�����ȥ����
#define CARP_CHECK(...)        VA_CARP_LOG_CHECK_PROXY(VA_CARP_LOG_CHECK_JOIN(CARP_LOG_CHECK_MACRO_, VA_CARP_LOG_CHECK_COUNT(__VA_ARGS__)), __VA_ARGS__)


#define CARP_LOG_CHECK_RETURN_MACRO_2(c, r) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return r; \
    } \
} while(0)

#define CARP_LOG_CHECK_RETURN_MACRO_3(c, r, p0) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return r; \
    } \
} while(0)

#define CARP_LOG_CHECK_RETURN_MACRO_4(c, r, p0, p1) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
		<< ", " << #p1 << ":" << (p1); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return r; \
    } \
} while(0)

#define CARP_LOG_CHECK_RETURN_MACRO_5(c, r, p0, p1, p2) \
do { \
    if (!(c)) { \
        std::ostringstream astream; astream << "ERROR:" << __FILE__ << ":"<< __FUNCTION__ << "() " << __LINE__ << " CHECK failed:" << #c \
        << ", " << #p0 << ":" << (p0); \
		<< ", " << #p1 << ":" << (p1); \
		<< ", " << #p2 << ":" << (p2); \
        s_carp_log.Log(astream.str().c_str(), CARP_LOG_LEVEL_ERROR); return r; \
    } \
} while(0)

// ������԰�����ȥ����
#define CARP_CHECK_RETURN(...)        VA_CARP_LOG_CHECK_PROXY(VA_CARP_LOG_CHECK_JOIN(CARP_LOG_CHECK_RETURN_MACRO_, VA_CARP_LOG_CHECK_COUNT(__VA_ARGS__)), __VA_ARGS__)


#endif

#ifdef CARP_LOG_IMPL
#ifndef CARP_LOG_IMPL_INCLUDE
#define CARP_LOG_IMPL_INCLUDE
CarpLog s_carp_log;
#endif
#endif

