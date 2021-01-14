#ifndef CARP_PROCESS_INCLUDED
#define CARP_PROCESS_INCLUDED

#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#endif

class CarpProcess
{
public:
    // ��������
    static bool CreateCarpProcess(const std::string& cmd_line, const char* work_path)
    {
#ifdef _WIN32
        std::wstring w_work_path;
        if (work_path != nullptr) w_work_path = UTF82Unicode(work_path);
        std::wstring w_cmd_line = UTF82Unicode(cmd_line);

        LPCWSTR cur_dir = NULL;
        if (!w_work_path.empty()) cur_dir = (LPCWSTR)w_work_path.c_str();

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        BOOL result = CreateProcessW(NULL, (LPWSTR)w_cmd_line.c_str(), NULL, NULL, false, 0, NULL, cur_dir, &si, &pi);
        return result == TRUE;
#else
        return false;
#endif
    }

	// ���ݿ�ִ���ļ���·����ȡ����ID
    static std::vector<unsigned long> GetProcessIDByPath(const std::string& path)
    {
		std::vector<unsigned long> result;
#ifdef _WIN32
        const std::wstring wpath = UTF82Unicode(path);

		HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapShot == INVALID_HANDLE_VALUE) return result;

		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);

        // ��ȡ��һ��
		::Process32First(hSnapShot, &pe32);
		do
		{
            // �������̿���
            HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            if (hModule != INVALID_HANDLE_VALUE)
            {
                // �ѵ�һ��ģ����Ϣ�� minfo
                MODULEENTRY32W minfo = { 0 };
                minfo.dwSize = sizeof(MODULEENTRY32W);//��������ڶ����С 
                Module32FirstW(hModule, &minfo);

                if (wpath == minfo.szExePath)//����ָ����������PID 
                    result.push_back(pe32.th32ProcessID);

                ::CloseHandle(hModule);
            }

		} while (::Process32Next(hSnapShot, &pe32));
		::CloseHandle(hSnapShot);
#else
#endif
		return result;
    }

	// ���ݽ���IDɱ������
	static bool KillProcessByID(unsigned long pid)
    {
#ifdef _WIN32
		HANDLE hGUI = OpenProcess(PROCESS_ALL_ACCESS, true, pid);
		if (NULL == hGUI) return false;

		TerminateProcess(hGUI, 0);
		return true;
#else
		return false;
#endif
    }

    // �������ⰴ��
    static bool SendVirtualKey(unsigned long pid, const char* cmd)
    {
#ifdef _WIN32
        HWND hWnd = GetWindowHWndByPID(pid);
        if (hWnd == NULL) return false;
        if (cmd == NULL) return false;

        while (*cmd != 0)
        {
            const auto sc = OemKeyScan(*cmd);
            const auto vkey = MapVirtualKey(sc & 0xffff, 1);

            ::PostMessage(hWnd, WM_KEYDOWN, vkey, 1);
            ++cmd;
        }
        return true;
#else
        return false;
#endif
    }

private:
#ifdef _WIN32
    ///< ö�ٴ��ڲ���
    typedef struct
    {
        HWND    hWndWindow;     // ���ھ��
        DWORD   dwProcessID;    // ����ID
    } EnumWindowsArg;

    ///< ö�ٴ��ڻص�����
    static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
    {
        EnumWindowsArg* pArg = (EnumWindowsArg*)lParam;
        DWORD  dwProcessID = 0;
        // ͨ�����ھ��ȡ�ý���ID
        ::GetWindowThreadProcessId(hWnd, &dwProcessID);
        if (dwProcessID == pArg->dwProcessID)
        {
            pArg->hWndWindow = hWnd;
            // �ҵ��˷���FALSE
            return FALSE;
        }
        // û�ҵ��������ң�����TRUE
        return TRUE;
    }

    ///< ͨ������ID��ȡ���ھ��
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