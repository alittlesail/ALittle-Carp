#ifndef CARP_THREAD_CONSUMER_INCLUDED
#define CARP_THREAD_CONSUMER_INCLUDED (1)

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

template <typename T>
class CarpThreadConsumer
{
public:
    CarpThreadConsumer() { }

    virtual ~CarpThreadConsumer() { Stop(); }

protected:
    // 启动
    void Start()
    {
        if (m_thread != nullptr) return;

        // 设置为继续执行
        m_run = true;
        // 创建线程，并开始执行
        m_thread = new std::thread(&CarpThreadConsumer::Run, this);
    }

    bool IsStart() const
    {
        return m_thread != nullptr;
    }

    // 添加
    void Add(T& info)
    {
        if (m_thread == nullptr) return;

        // 把日志添加到列表
        std::unique_lock<std::mutex> lock(m_mutex);
        m_list.emplace_back(std::move(info));
        m_cv.notify_one();
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
        while (!m_list.empty())
        {
            Flush(m_list.front());
            m_list.pop_front();
        }
    }

private:
    // 支线程函数
    int Run()
    {
        // 定义临时列表
        std::list<T> temp_list;
        while (m_run)
        {
            // 获取执行队列
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (m_run && m_list.empty())
                    m_cv.wait(lock);
                temp_list.swap(m_list);
            }

            // 开始执行
            while (!temp_list.empty())
            {
                Flush(temp_list.front());
                temp_list.pop_front();
            }
        }

        return 0;
    }

protected:
    // 执行日志
    virtual void Flush(T& info) = 0;

private:
    std::mutex m_mutex;                 // 互斥锁
    std::condition_variable m_cv;       // 条件变量

private:
    std::list<T> m_list;	// 等待日志的列表

private:
    volatile bool m_run = false;	// 支线程是否正在执行
    std::thread* m_thread = nullptr;	// 线程对象
};

#endif
