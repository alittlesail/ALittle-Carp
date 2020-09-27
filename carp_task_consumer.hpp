#ifndef CARP_TASK_CONSUMER_INCLUDED
#define CARP_TASK_CONSUMER_INCLUDED (1)

#include <vector>
#include <functional>

#include "carp_thread_consumer.hpp"

class CarpTask
{
public:
	/**
	 * execute
	 */
	virtual void Execute() = 0;
	/**
	 * abandon
	 */
	virtual void Abandon() = 0;
	/**
	 * destruction
	 */
	virtual ~CarpTask() { }
};

class CarpTaskThread : public CarpThreadConsumer<CarpTask*>
{
public:
	void Execute(CarpTask*& info) override { info->Execute(); }
	void Abandon(CarpTask*& info) override { info->Abandon(); }
};

class CarpTaskConsumer
{
public:
	void SetThreadCount(int count)
	{
		for (int i = (int)m_threads.size(); i < count; ++i)
		{
			CarpTaskThread* thread = new CarpTaskThread();
			thread->Start();
			m_threads.push_back(thread);
		}
	}

	int GetThreadCount() const { return (int)m_threads.size(); }

	void AddTask(CarpTask* task)
	{
		if (m_threads.empty())
		{
			task->Abandon();
			return;
		}

		m_index += 1;
		m_index %= m_threads.size();
		m_threads[m_index]->Add(task);
	}

	void PushEvent(const std::function<void()>& event)
	{
		m_mutex.lock();
		m_event_list.push_back(event);
		m_mutex.unlock();
	}

	void HandleEvent()
	{
		static std::vector<std::function<void()>> event_list;
		m_mutex.lock();
		event_list.swap(m_event_list);
		m_mutex.unlock();

		for (auto& event : event_list)
			event();
	}

public:
	void Shutdown()
	{
		for (auto* thread : m_threads)
			delete thread;
		m_threads.clear();
	}

private:
	size_t m_index = 0;
	std::vector<CarpTaskThread*> m_threads;

private:
	std::mutex m_mutex;
	std::vector<std::function<void()>> m_event_list;
};

extern CarpTaskConsumer s_carp_task_consumer;

#endif

#ifdef CARP_TASK_CONSUMER_IMPL
CarpTaskConsumer s_carp_task_consumer;
#endif
