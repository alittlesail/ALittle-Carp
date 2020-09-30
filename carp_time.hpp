#ifndef CARP_TIME_INCLUDED
#define CARP_TIME_INCLUDED

#include <chrono>
#include <ctime>
#include <string>

const int CARP_TIME_ONE_MINUTE_SECONDS = 60;						// ÿ���Ӷ�Ӧ������
const int CARP_TIME_ONE_HOUR_SECONDS = 60 * CARP_TIME_ONE_MINUTE_SECONDS;	// ÿСʱ��Ӧ������
const int CARP_TIME_ONE_DAY_SECONDS = 24 * CARP_TIME_ONE_HOUR_SECONDS;		// һ�������
const int CARP_TIME_ONE_WEEK_DAY = 7;								// һ����Ҫ������
const int CARP_TIME_ONE_WEEK_SECONDS = 7 * CARP_TIME_ONE_DAY_SECONDS;		// һ����Ҫ������

class CarpTime
{
public:
	static tm GetTimeStruct(time_t cur_time)
	{
		tm* ptr_time;
#ifdef _WIN32
		tm st_time;
		ptr_time = &st_time;
		localtime_s(ptr_time, &cur_time);
#else
		ptr_time = localtime(&cur_time);
#endif
		return *ptr_time;
	}
	
	// ��ȡ��ǰʱ�䣬��λ����
	static long long GetCurMSTime()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	// ��ȡ��ǰʱ�䣬��λ��
	static time_t GetCurTime()
	{
		return time(nullptr);
	}
	// ��ȡ����0���ʱ�䣬��λ��
	static time_t GetTodayBeginTime()
	{
		return CalcTodayBeginTime(GetCurTime());
	}
	static time_t CalcTodayBeginTime(time_t cur_time)
	{
		const tm stime = GetTimeStruct(cur_time);

		tm st_time_tmp;
		st_time_tmp.tm_year = stime.tm_year;
		st_time_tmp.tm_mon = stime.tm_mon;
		st_time_tmp.tm_mday = stime.tm_mday;
		st_time_tmp.tm_hour = 0;
		st_time_tmp.tm_min = 0;
		st_time_tmp.tm_sec = 0;

		return mktime(&st_time_tmp);
	}
	// ��ȡ�ڶ���0���ʱ�䣬��λ��
	static time_t GetNextTodayBeginTime()
	{
		return CalcTodayBeginTime(GetCurTime()) + CARP_TIME_ONE_DAY_SECONDS;
	}

	// ��ȡ�����0��ʱ�䣬��λ��
	static time_t GetMonthBeginTime()
	{
		return CalcMonthBeginTime(GetCurTime());
	}
	static time_t CalcMonthBeginTime(time_t cur_time)
	{
		const tm stime = GetTimeStruct(cur_time);

		tm st_time_tmp;
		st_time_tmp.tm_year = stime.tm_year;
		st_time_tmp.tm_mon = stime.tm_mon;
		st_time_tmp.tm_mday = 1;
		st_time_tmp.tm_hour = 0;
		st_time_tmp.tm_min = 0;
		st_time_tmp.tm_sec = 0;

		return mktime(&st_time_tmp);
	}
	// ��ȡ�¸���0��ʱ�䣬��λ��
	static time_t GetNextMonthBeginTime()
	{
		return CalcNextMonthBeginTime(GetCurTime());
	}
	static time_t CalcNextMonthBeginTime(time_t cur_time)
	{
		const tm stime = GetTimeStruct(cur_time);

		tm st_time_tmp;
		st_time_tmp.tm_year = stime.tm_year;
		st_time_tmp.tm_mon = stime.tm_mon + 1;
		if (st_time_tmp.tm_mon >= 12)
		{
			st_time_tmp.tm_year += 1;
			st_time_tmp.tm_mon = 0;
		}
		st_time_tmp.tm_mday = 1;
		st_time_tmp.tm_hour = 0;
		st_time_tmp.tm_min = 0;
		st_time_tmp.tm_sec = 0;

		return mktime(&st_time_tmp);
	}

	/* ��ʽ��ʱ��
	 * @param time: the time to format
	 * @return YMD: YYYY-MM-DD
	 * @param ymd_split �������и��ַ�
	 * @return HMS: HH:MM:SS
	 * @param hms_split ʱ�����и��ַ�
	 */
	static void FormatTime(time_t cur_time, std::string* YMD, char ymd_split, std::string* HMS, char hms_split)
	{
		const tm stime = GetTimeStruct(cur_time);

		if (HMS)
		{
			char hms[16];
			if (hms_split != 0)
			{
#ifdef _WIN32
				sprintf_s(hms, 16, "%02d%c%02d%c%02d", stime.tm_hour, hms_split, stime.tm_min, hms_split, stime.tm_sec);
#else
				sprintf(hms, "%02d%c%02d%c%02d", stime.tm_hour, hms_split, stime.tm_min, hms_split, stime.tm_sec);
#endif
			}
			else
			{
#ifdef _WIN32
				sprintf_s(hms, 16, "%02d%02d%02d", stime.tm_hour, stime.tm_min, stime.tm_sec);
#else
				sprintf(hms, "%02d%02d%02d", stime.tm_hour, stime.tm_min, stime.tm_sec);
#endif
			}
			*HMS = hms;
		}

		if (YMD)
		{
			char date[16];
			if (ymd_split != 0)
			{
#ifdef _WIN32
				sprintf_s(date, 16, "%4d%c%02d%c%02d", (stime.tm_year + 1900), ymd_split, stime.tm_mon + 1, ymd_split, stime.tm_mday);
#else
				sprintf(date, "%4d%c%02d%c%02d", (stime.tm_year + 1900), ymd_split, stime.tm_mon + 1, ymd_split, stime.tm_mday);
#endif
			}
			else
			{
#ifdef _WIN32
				sprintf_s(date, 16, "%4d%02d%02d", (stime.tm_year + 1900), stime.tm_mon + 1, stime.tm_mday);
#else
				sprintf(date, "%4d%02d%02d", (stime.tm_year + 1900), stime.tm_mon + 1, stime.tm_mday);
#endif
			}
			*YMD = date;
		}
	}
	
	static std::string FormatTime(time_t time)
	{
		std::string YMD, HMS;
		FormatTime(time, &YMD, '-', &HMS, ':');
		return YMD + " " + HMS;
	}
};


#endif