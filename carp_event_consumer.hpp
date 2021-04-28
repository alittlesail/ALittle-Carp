#ifndef CARP_EVENT_CONSUMER_INCLUDED
#define CARP_EVENT_CONSUMER_INCLUDED

#include <vector>
#include <functional>
#include <mutex>

class CarpEventConsumer
{
public:
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
		event_list.resize(0);
	}

private:
	std::mutex m_mutex;
	std::vector<std::function<void()>> m_event_list;
};

extern CarpEventConsumer s_carp_event_consumer;

#endif

#ifdef CARP_EVENT_CONSUMER_IMPL
#ifndef CARP_EVENT_CONSUMER_IMPL_INCLUDE
#define CARP_EVENT_CONSUMER_IMPL_INCLUDE
CarpEventConsumer s_carp_events_consumer;
#endif
#endif
