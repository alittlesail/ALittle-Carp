#ifndef CARP_PROCESS_INCLUDED
#define CARP_PROCESS_INCLUDED

#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#endif

class CarpProcess
{
public:
    // 创建进程
    static bool CreateCarpProcess(const std::string& file_path, const std::string& param, const char* work_path)
    {
#ifdef _WIN32
        std::wstring w_cmd = UTF82Unicode(file_path) + L" " + UTF82Unicode(param);
        std::wstring w_work_path;
        if (work_path != nullptr) w_work_path = UTF82Unicode(work_path);

        LPCWSTR cur_dir = NULL;
        if (!w_work_path.empty()) cur_dir = (LPCWSTR)w_work_path.c_str();

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        BOOL result = CreateProcessW(NULL
            , (LPWSTR)w_cmd.c_str()
            , NULL
            , NULL
            , false
            , CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS
            , NULL
            , cur_dir, &si, &pi);
        return result == TRUE;
#else
        return false;
#endif
    }

	// 根据可执行文件的路径获取进程ID
    static std::vector<unsigned long> GetProcessIDByPath(const std::string& path)
    {
		std::vector<unsigned long> result;
#ifdef _WIN32
        const std::wstring wpath = UTF82Unicode(path);

		HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapShot == INVALID_HANDLE_VALUE) return result;

		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);

        // 获取第一个
		::Process32First(hSnapShot, &pe32);
		do
		{
            // 创建进程快照
            HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            if (hModule != INVALID_HANDLE_VALUE)
            {
                // 把第一个模块信息给 minfo
                MODULEENTRY32W minfo = { 0 };
                minfo.dwSize = sizeof(MODULEENTRY32W);//填充进程入口对象大小 
                Module32FirstW(hModule, &minfo);

                if (wpath == minfo.szExePath)//查找指定进程名的PID 
                    result.push_back(pe32.th32ProcessID);

                ::CloseHandle(hModule);
            }

		} while (::Process32Next(hSnapShot, &pe32));
		::CloseHandle(hSnapShot);
#else
#endif
		return result;
    }

	// 根据进程ID杀死进程
	static bool KillProcessByID(unsigned long pid)
    {
#ifdef _WIN32
		HANDLE hGUI = OpenProcess(PROCESS_ALL_ACCESS, true, pid);
		if (NULL == hGUI) return false;

		TerminateProcess(hGUI, 0);
        CloseHandle(hGUI);
		return true;
#else
		return false;
#endif
    }

    // 发送虚拟按键
    static bool SendVirtualKey(unsigned long pid, const char* cmd)
    {
#ifdef _WIN32
        if (cmd == NULL) return false;

        HWND hWnd = GetWindowHWndByPID(pid);
        if (hWnd == NULL) return false;

        while (*cmd != 0)
        {
            const auto sc = OemKeyScan(*cmd);
            const auto vkey = MapVirtualKey(sc & 0xffff, 1);

            if (::PostMessage(hWnd, WM_KEYDOWN, vkey, 1) == 0)
                return false;
            ++cmd;
        }
        return true;
#else
        return false;
#endif
    }

#ifdef _WIN32
    static long long FileTimeToUTC(const FILETIME& file_time) {
        LARGE_INTEGER li;
        li.LowPart = file_time.dwLowDateTime;
        li.HighPart = file_time.dwHighDateTime;
        return li.QuadPart;
    }
#endif

    // 获取当前CPU使用率
    static int GetCPUUsage(int process_id, long long& last_time, long long& last_system_time)
    {
#ifdef _WIN32
        static SYSTEM_INFO info = {0};
        if (info.dwNumberOfProcessors <= 0) GetSystemInfo(&info);

        HANDLE process = NULL;
        if (process_id == 0)
            process = GetCurrentProcess();
        else
            process = OpenProcess(PROCESS_ALL_ACCESS, true, process_id);
        if (NULL == process) return -1;

        // cpu数量
        FILETIME now;
        GetSystemTimeAsFileTime(&now);

        FILETIME creation_time;
        FILETIME exit_time;
        FILETIME kernel_time;
        FILETIME user_time;
        if (!GetProcessTimes(process, &creation_time, &exit_time,
            &kernel_time, &user_time)) {
            if (process_id != 0) CloseHandle(process);
            // We don't assert here because in some cases (such as in the Task Manager)
            // we may call this function on a process that has just exited but we have
            // not yet received the notification.
            return -1;
        }
        if (process_id != 0) CloseHandle(process);

        const long long system_time = (FileTimeToUTC(kernel_time) + FileTimeToUTC(user_time)) / info.dwNumberOfProcessors;
        const long long time = FileTimeToUTC(now);

        //上一次的时间
        if (last_system_time == 0 || last_time == 0)
        {
            // First call, just set the last values.
            last_system_time = system_time;
            last_time = time;
            return -1;
        }

        const long long system_time_delta = system_time - last_system_time;
        const long long time_delta = time - last_time;
        if (time_delta == 0) return -1;

        // We add timeDelta / 2 so the result is rounded.
        const int cpu = static_cast<int>((system_time_delta * 100 + time_delta / 2) / time_delta);
        last_system_time = system_time;
        last_time = time;

        return cpu;
#else
        return -1;
#endif // _WIN32
    }

    // 获取当前内存使用率
    static bool GetMemoryUsage(int process_id, size_t& mem, size_t& vmem)
    {
        mem = 0;
        vmem = 0;

#ifdef _WIN32
        HANDLE process = NULL;
        if (process_id == 0)
            process = GetCurrentProcess();
        else
            process = OpenProcess(PROCESS_ALL_ACCESS, true, process_id);
        if (NULL == process) return false;

        PROCESS_MEMORY_COUNTERS pmc;
        if (!GetProcessMemoryInfo(process, &pmc, sizeof(pmc)))
        {
            if (process_id != 0) CloseHandle(process);
            return false;
        }
        mem = pmc.WorkingSetSize;
        vmem = pmc.PagefileUsage;
        if (process_id != 0) CloseHandle(process);
        return true;
#else
        return false;
#endif
    }

    // 获取吞吐量
    static bool GetIOUsage(int process_id, size_t& read, size_t& write)
    {
        read = 0;
        write = 0;

#ifdef _WIN32
        HANDLE process = NULL;
        if (process_id == 0)
            process = GetCurrentProcess();
        else
            process = OpenProcess(PROCESS_ALL_ACCESS, true, process_id);
        if (NULL == process) return false;

        IO_COUNTERS io_counter;
        if (!GetProcessIoCounters(process, &io_counter))
        {
            if (process_id != 0) CloseHandle(process);
            return false;
        }

        read = io_counter.ReadTransferCount;
        write = io_counter.WriteTransferCount;
        if (process_id != 0) CloseHandle(process);
        return true;
#else
        return false;
#endif
    }

private:
#ifdef _WIN32
    ///< 枚举窗口参数
    typedef struct
    {
        HWND    hWndWindow;     // 窗口句柄
        DWORD   dwProcessID;    // 进程ID
    } EnumWindowsArg;

    ///< 枚举窗口回调函数
    static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
    {
        EnumWindowsArg* pArg = (EnumWindowsArg*)lParam;
        DWORD  dwProcessID = 0;
        // 通过窗口句柄取得进程ID
        ::GetWindowThreadProcessId(hWnd, &dwProcessID);
        if (dwProcessID == pArg->dwProcessID)
        {
            pArg->hWndWindow = hWnd;
            // 找到了返回FALSE
            return FALSE;
        }
        // 没找到，继续找，返回TRUE
        return TRUE;
    }

    ///< 通过进程ID获取窗口句柄
    static HWND GetWindowHWndByPID(DWORD dwProcessID)
    {
        EnumWindowsArg ewa;
        ewa.dwProcessID = dwProcessID;
        ewa.hWndWindow = NULL;
        EnumWindows(EnumWindowsProc, (LPARAM)&ewa);
        return ewa.hWndWindow;
    }

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
        if (len >= 1) result.resize(len - 1);
        WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, const_cast<char*>(result.c_str()), len, nullptr, nullptr);
        return result;
    }
#endif
};


#endif