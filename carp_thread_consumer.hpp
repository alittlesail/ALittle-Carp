#ifndef CARP_THREAD_CONSUMER_INCLUDED
#define CARP_THREAD_CONSUMER_INCLUDED

#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

// T ��־�ṹ���ͣ�WAIT_FOR_FLUSH_MS ������0ʱ����ʾʹ�ö�ʱִ��Flush
template <typename T, int WAIT_FOR_FLUSH_MS = 0>
class CarpThreadConsumer
{
public:
    CarpThreadConsumer() { }

    virtual ~CarpThreadConsumer() { Stop(); }

public:
    // ����
    void Start()
    {
        if (m_thread != nullptr) return;

        // ����Ϊ����ִ��
        m_run = true;
        // �����̣߳�����ʼִ��
        m_thread = new std::thread(&CarpThreadConsumer::Run, this);
    }

    bool IsStart() const
    {
        return m_thread != nullptr;
    }

    // ���
    void Add(T& info)
    {
        if (m_thread == nullptr) return;

        // ����־��ӵ��б�
        std::unique_lock<std::mutex> lock(m_mutex);
        m_list.emplace_back(std::move(info));
        m_cv.notify_one();
    }

    // �ر�
    void Stop()
    {
        if (m_thread == nullptr) return;

        // �⿪��
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_run = false;
            m_cv.notify_all();
        }
        // �ȴ��߳���������
        m_thread->join();
        // ɾ���̶߳���
        delete m_thread;
        m_thread = nullptr;

        // ��ʼִ��
        while (!m_list.empty())
        {
            Abandon(m_list.front());
            m_list.pop_front();
        }
    }

private:
    // ֧�̺߳���
    int Run()
    {
        // ������ʱ�б�
        std::list<T> temp_list;
        while (m_run)
        {
            // �ж��Ƿ�ˢ��
            bool flush = false;

            // ��ȡִ�ж���
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (m_run && m_list.empty())
                {
                    if (WAIT_FOR_FLUSH_MS > 0)
                    {
                        if (m_cv.wait_for(lock, std::chrono::milliseconds(WAIT_FOR_FLUSH_MS)) == std::cv_status::timeout)
                        {
                            flush = true;
                            break;
                        }
                    }   
                    else
                        m_cv.wait(lock);
                }
                temp_list.swap(m_list);
            }

            // ��ʼִ��
            while (!temp_list.empty())
            {
                Execute(temp_list.front());
                temp_list.pop_front();
            }

            if (flush) Flush();
        }

        return 0;
    }

protected:
    // ִ����־
    virtual void Execute(T& info) = 0;
    virtual void Abandon(T& info) { Execute(info); }
    virtual void Flush() {}

private:
    std::mutex m_mutex;                 // ������
    std::condition_variable m_cv;       // ��������

private:
    std::list<T> m_list;	// �ȴ���־���б�

private:
    volatile bool m_run = false;	// ֧�߳��Ƿ�����ִ��
    std::thread* m_thread = nullptr;	// �̶߳���
};

#endif
