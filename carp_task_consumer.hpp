#ifndef CARP_TASK_CONSUMER_INCLUDED
#define CARP_TASK_CONSUMER_INCLUDED

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
#ifndef __EMSCRIPTEN__
		for (int i = static_cast<int>(m_threads.size()); i < count; ++i)
		{
			auto* thread = new CarpTaskThread();
			thread->Start();
			m_threads.push_back(thread);
		}
#endif
	}

	int GetThreadCount() const { return static_cast<int>(m_threads.size()); }

	void AddTask(CarpTask* task)
	{
		if (m_threads.empty())
		{
#ifdef __EMSCRIPTEN__
			task->Execute();
#else
			task->Abandon();
#endif
			return;
		}

		m_index += 1;
		m_index %= m_threads.size();
		m_threads[m_index]->Add(task);
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
};

extern CarpTaskConsumer s_carp_task_consumer;

#endif

#ifdef CARP_TASK_CONSUMER_IMPL
#ifndef CARP_TASK_CONSUMER_IMPL_INCLUDE
#define CARP_TASK_CONSUMER_IMPL_INCLUDE
CarpTaskConsumer s_carp_task_consumer;
#endif
#endif
