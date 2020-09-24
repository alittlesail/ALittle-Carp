
#ifdef _WIN32

#ifndef CARP_CONSOLE_INCLUDED
#define CARP_CONSOLE_INCLUDED (1)

#include <functional>
#include <string>
#include <thread>

#include <Windows.h>
#include "Carp/carp_string.hpp"
#include "Carp/carp_log.hpp"

class CarpConsole
{
public:
    CarpConsole() {}
    ~CarpConsole()
    {
        Shutdown();
    }

public:
    void Setup(const std::string& title, std::function<void(const std::string&, const std::string&)> func
        , std::function<void()> exit, std::function<void()> list)
    {
        if (m_thread != nullptr) return;

        m_run = true;
        m_thread = new std::thread(&CarpConsole::Run, this);
        m_title = CarpString::UTF82Unicode(title);
        m_handle = func;
        m_exit = exit;
        m_list = list;
    }
    void Shutdown()
    {
        if (m_thread == nullptr) return;

        m_run = false;
        HANDLE std_in = GetStdHandle(STD_INPUT_HANDLE);
        if (std_in != INVALID_HANDLE_VALUE && std_in != NULL) {
            INPUT_RECORD record;
            record.EventType = KEY_EVENT;
            record.Event.KeyEvent.bKeyDown = true;
            record.Event.KeyEvent.uChar.AsciiChar = 0;
            DWORD count;
            WriteConsoleInput(std_in, &record, 1, &count);
        }

        m_thread->join();
        delete m_thread;
        m_thread = 0;
    }

private:
    int Run()
    {
        // Get the standard input handle. 
        HANDLE std_in = GetStdHandle(STD_INPUT_HANDLE);
        if (std_in == INVALID_HANDLE_VALUE) return 0;

        if (std_in == NULL)
        {
            CARP_ERROR("GetStdHandle(STD_INPUT_HANDLE) failed!");
            return 0;
        }

        INPUT_RECORD irInBuf[128];
        DWORD cNumRead;
        std::wstring cmd;

        while (m_run)
        {
            if (!ReadConsoleInput(
                std_in,      // input buffer handle 
                irInBuf,     // buffer to read into 
                128,         // size of read buffer 
                &cNumRead)) // number of records read
            {
                CARP_ERROR("ReadConsoleInput failed!");
                return 0;
            }

            for (DWORD i = 0; i < cNumRead; ++i)
            {
                if (irInBuf[i].EventType != KEY_EVENT) continue;

                KEY_EVENT_RECORD& ker = irInBuf[i].Event.KeyEvent;
                if (!ker.bKeyDown) continue;
                if (ker.uChar.AsciiChar == 0) continue;

                if (ker.uChar.AsciiChar == 8)
                {
                    if (cmd.empty()) continue;
                    if (cmd.back() > 0)
                        cmd.pop_back();
                    else
                    {
                        cmd.pop_back();
                        if (!cmd.empty()) cmd.pop_back();
                    }
                }
                else if (ker.uChar.AsciiChar == '\r'
                    || ker.uChar.AsciiChar == '\n')
                {
                    if (cmd.empty()) continue;

                    std::string cmd_utf8 = CarpString::Unicode2UTF8(cmd);
                    HandleCmd(cmd_utf8);
                    cmd.clear();
                }
                else
                {
                    cmd.push_back(ker.uChar.UnicodeChar);
                }

                if (cmd.size())
                    ::SetConsoleTitleW((m_title + L":" + cmd).c_str());
                else
                    ::SetConsoleTitleW(m_title.c_str());
            }
        }

        return 0;
    }
    void HandleCmd(std::string& cmd)
    {
        CarpString::TrimLeft(cmd);
        CarpString::TrimRight(cmd);
        if (cmd.empty()) return;

        std::string upper_cmd = cmd;
        CarpString::UpperString(upper_cmd);
        if (m_exit && upper_cmd == "EXIT")
        {
            m_exit();
            return;
        }

        if (m_list && upper_cmd == "HELP")
        {
            m_list();
            return;
        }

        std::string::size_type module_pos = cmd.find(' ');
        if (module_pos == 0)
        {
            CARP_WARN(u8"û������ģ����");
            return;
        }
        if (module_pos == std::string::npos)
        {
            CARP_WARN(u8"û�������κ�ָ�ָ���ʽΪ: ģ���� ָ���� ָ�����");
            return;
        }
        std::string module_name = cmd.substr(0, module_pos);
        std::string cmd_content = cmd.substr(module_pos + 1);
        CarpString::TrimLeft(cmd_content);
        CarpString::TrimRight(cmd_content);

        if (m_handle) m_handle(module_name, cmd_content);
    }

private:
    std::thread* m_thread = nullptr;
    volatile bool m_run = false;

private:
    std::wstring m_title;
    std::function<void()> m_exit;
    std::function<void()> m_list;
    std::function<void(const std::string&, const std::string&)> m_handle;
};

extern CarpConsole& CarpConsoleInstance();
#define s_carp_console CarpConsoleInstance()

#endif

#ifdef CARP_CONSOLE_IMPL
CarpConsole& CarpConsoleInstance() { static CarpConsole instance; return instance; }
#endif

#endif
