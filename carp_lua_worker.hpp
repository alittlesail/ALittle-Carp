#ifndef CARP_LUA_WORKER_INCLUDED
#define CARP_LUA_WORKER_INCLUDED

extern "C" {
#include "lua.h"
#include "lualib/ajson/lua_ajson.h"
#include "lualib/cjson/lua_cjson.h"
#include "lualib/sqlite3/lsqlite3.h"
}

#include "Carp/carp_script.hpp"
#include "Carp/carp_message_bind.hpp"
#include "Carp/carp_crypto_bind.hpp"
#include "Carp/carp_log_bind.hpp"
#include "Carp/carp_csv_bind.hpp"
#include "Carp/carp_timer_bind.hpp"
#include "Carp/carp_file_bind.hpp"
#include "Carp/carp_string_bind.hpp"
#include "Carp/carp_rwops_bind.hpp"
#include "Carp/carp_surface_bind.hpp"
#include "Carp/carp_process_bind.hpp"

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

class CarpLuaWorker : public CarpScript
{
public:
    CarpLuaWorker(const char* core_path, const char* std_path, const char* lua_path)
    {
        // ����·��
        m_core_path = core_path;
        m_std_path = std_path;
        m_lua_path = lua_path;
        // ����Ϊ����ִ��
        m_run = true;
        // �����̣߳�����ʼִ��
        m_thread = new std::thread(&CarpLuaWorker::Run, this);
    }

    virtual ~CarpLuaWorker()
    {
        Stop();
    }

    static void Bind(lua_State* l_state)
    {
        luabridge::getGlobalNamespace(l_state)
            .beginNamespace("carp")
            .beginClass<CarpLuaWorker>("CarpLuaWorker")
            .addConstructor<void(*)(const char*, const char*, const char*)>()
            .addFunction("Post", &CarpLuaWorker::PostToWorker)
            .addFunction("Pull", &CarpLuaWorker::Pull)
            .addFunction("IsStopped", &CarpLuaWorker::IsStopped)
            .addFunction("Stop", &CarpLuaWorker::Stop)
            .endClass()
            .endNamespace();
    }

public:
    bool IsStopped() const
    {
        return m_thread == nullptr;
    }

    // ���̴߳���֧�߳�
    bool PostToWorker(const char* info)
    {
        if (m_thread == nullptr) return false;

        // ����־��ӵ��б�
        std::unique_lock<std::mutex> lock(m_push_mutex);
        m_push_list.emplace_back(info);

        return true;
    }

    // ֧�߳������߷���
    bool PostToMain(const char* info)
    {
        std::unique_lock<std::mutex> lock(m_pull_mutex);
        m_pull_list.emplace_back(info);
        return true;
    }

    // ���߳���֧�߳���ȡ
    const char* Pull()
    {
        if (m_handle_list.empty())
        {
            std::unique_lock<std::mutex> lock(m_pull_mutex);
            m_handle_list.swap(m_pull_list);
        }

        if (m_handle_list.empty()) return nullptr;

        m_temp = std::move(m_handle_list.front());
        m_handle_list.pop_front();
        return m_temp.c_str();
    }

    // �ر�
    void Stop()
    {
        if (m_thread == nullptr) return;

        // �⿪��
        {
            std::unique_lock<std::mutex> lock(m_push_mutex);
            m_run = false;
        }
        // �ȴ��߳���������
        m_thread->join();
        // ɾ���̶߳���
        delete m_thread;
        m_thread = nullptr;

        // ��ʼִ��
        while (!m_push_list.empty())
        {
            Execute(m_push_list.front());
            m_push_list.pop_front();
        }
    }

private:
    // ֧�̺߳���
    int Run()
    {
        Init();

        luaopen_cjson(m_L); lua_settop(m_L, 0);
        luaopen_ajson(m_L); lua_settop(m_L, 0);
        luaopen_sqlite3(m_L); lua_settop(m_L, 0);

        CarpMessageBind::Bind(m_L);
        CarpCryptoBind::Bind(m_L);
        CarpLogBind::Bind(m_L);
        CarpCsvBind::Bind(m_L);
        CarpTimerBind::Bind(m_L);
        CarpFileBind::Bind(m_L);
        CarpStringBind::Bind(m_L);
        CarpRWopsBind::Bind(m_L);
        CarpSurfaceBind::Bind(m_L);
        CarpProcessBind::Bind(m_L);

        luabridge::getGlobalNamespace(m_L)
            .beginNamespace("carp")
            .beginClass<CarpLuaWorker>("CarpLuaWorker")
            .addFunction("Post", &CarpLuaWorker::PostToMain)
            .endClass()
            .endNamespace();

        luabridge::setGlobal(m_L, this, "carp_CarpLuaWorker");

        // ���غ���
        Require((m_core_path + "ALittle").c_str());
        Invoke("RequireCore", m_core_path.c_str());

        // ���ر�׼��
        Require((m_std_path + "ALittle").c_str());
        Invoke("RequireStd", m_std_path.c_str());

        // ����lua�ļ�
        Require(m_lua_path.c_str());

        // ��ȡ��һ���¼�
        auto last_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        // ������ʱ�б�
        std::list<std::string> temp_list;
        while (m_run)
        {
            // ��ȡִ�ж���
            {
                std::unique_lock<std::mutex> lock(m_push_mutex);
                temp_list.swap(m_push_list);
            }

            // ��ʼִ��
            while (!temp_list.empty())
            {
                Execute(temp_list.front());
                temp_list.pop_front();
            }

            auto cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            Invoke("__ALITTLEAPI_WorkerUpdate", cur_time - last_time);
            last_time = cur_time;

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        Release();
        return 0;
    }

protected:
    // ִ����־
    void Execute(const std::string& info)
    {
        Invoke("__ALITTLEAPI_WorkerMessage", info.c_str());
    }

private:
    std::mutex m_push_mutex;                 // ������
    std::list<std::string> m_push_list;	// �ȴ���־���б�

private:
    std::mutex m_pull_mutex;            // ������
    std::list<std::string> m_pull_list; // ��ȡ�б�
    std::list<std::string> m_handle_list;   // �����б�
    std::string m_temp;                 // ��ʱ�ַ���

private:
    std::string m_core_path;        // ���Ŀ�·��
    std::string m_std_path;         // ��׼��·��
    std::string m_lua_path;     // ���lua�ļ�

private:
    volatile bool m_run = false;	// ֧�߳��Ƿ�����ִ��
    std::thread* m_thread = nullptr;	// �̶߳���
};

#endif
