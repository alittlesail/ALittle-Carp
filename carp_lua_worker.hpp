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

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

class CarpLuaWorker : public CarpScript
{
public:
    CarpLuaWorker(const char* core_path, const char* std_path, const char* lua_path)
    {
        // 保存路径
        m_core_path = core_path;
        m_std_path = std_path;
        m_lua_path = lua_path;
        // 设置为继续执行
        m_run = true;
        // 创建线程，并开始执行
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

    // 主线程传给支线程
    bool PostToWorker(const char* info)
    {
        if (m_thread == nullptr) return false;

        // 把日志添加到列表
        std::unique_lock<std::mutex> lock(m_mutex);
        m_push_list.emplace_back(info);
        m_cv.notify_one();

        return true;
    }

    // 支线程向主线发送
    bool PostToMain(const char* info)
    {
        std::unique_lock<std::mutex> lock(m_pull_mutex);
        m_pull_list.emplace_back(info);
        return true;
    }

    // 主线程向支线程拉取
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

    // 关闭
    void Stop()
    {
        if (m_thread == nullptr) return;

        // 解开锁
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_run = false;
            m_cv.notify_all();
        }
        // 等待线程正常结束
        m_thread->join();
        // 删除线程对象
        delete m_thread;
        m_thread = nullptr;

        // 开始执行
        while (!m_push_list.empty())
        {
            Execute(m_push_list.front());
            m_push_list.pop_front();
        }
    }

private:
    // 支线程函数
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

        luabridge::getGlobalNamespace(m_L)
            .beginNamespace("carp")
            .beginClass<CarpLuaWorker>("CarpLuaWorker")
            .addFunction("Post", &CarpLuaWorker::PostToMain)
            .endClass()
            .endNamespace();

        luabridge::setGlobal(m_L, this, "carp_CarpScript");

        // 加载核心
        Require((m_core_path + "ALittle").c_str());
        Invoke("RequireCore", m_core_path.c_str());

        // 加载标准库
        Require((m_std_path + "ALittle").c_str());
        Invoke("RequireStd", m_std_path.c_str());

        // 加载lua文件
        Require(m_lua_path.c_str());

        // 定义临时列表
        std::list<std::string> temp_list;
        while (m_run)
        {
            // 获取执行队列
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (m_run && m_push_list.empty())
                    m_cv.wait(lock);
                temp_list.swap(m_push_list);
            }

            // 开始执行
            while (!temp_list.empty())
            {
                Execute(temp_list.front());
                temp_list.pop_front();
            }
        }

        Release();
        return 0;
    }

protected:
    // 执行日志
    void Execute(const std::string& info)
    {
        Invoke("__ALITTLEAPI_WorkerMessage", info.c_str());
    }

private:
    std::mutex m_mutex;                 // 互斥锁
    std::condition_variable m_cv;       // 条件变量
    std::list<std::string> m_push_list;	// 等待日志的列表

private:
    std::mutex m_pull_mutex;            // 互斥锁
    std::list<std::string> m_pull_list; // 拉取列表
    std::list<std::string> m_handle_list;   // 处理列表
    std::string m_temp;                 // 临时字符串

private:
    std::string m_core_path;        // 核心库路径
    std::string m_std_path;         // 标准库路径
    std::string m_lua_path;     // 入口lua文件

private:
    volatile bool m_run = false;	// 支线程是否正在执行
    std::thread* m_thread = nullptr;	// 线程对象
};

#endif
