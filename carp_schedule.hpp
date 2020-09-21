#ifndef CARP_SCHEDULE_INCLUDED
#define CARP_SCHEDULE_INCLUDED (1)

#include <asio.hpp>
#include <thread>

typedef asio::basic_waitable_timer<std::chrono::system_clock> CarpAsioTimer;
typedef std::shared_ptr<CarpAsioTimer> CarpAsioTimerPtr;

class CarpSchedule
{
public:
	CarpSchedule() : m_is_exit(true), m_thread(0) {}
	~CarpSchedule() { Exit(); }

public:
	void Start(std::function<void(time_t)> update_func)
	{
		if (!m_is_exit) return;
		m_is_exit = false;
		m_update_func = update_func;
		m_thread = new std::thread(&CarpSchedule::Run, this);
	}

	void Timer(int delay_ms)
	{
		if (m_is_exit) return;
		m_io_service.post(std::bind(&CarpSchedule::TimerImpl, this, delay_ms));
	}

	void Run()
	{
		m_loop_timer = CarpAsioTimerPtr(new CarpAsioTimer(m_io_service, std::chrono::seconds(0xEFFFFFFF)));
		m_loop_timer->async_wait(std::bind(&CarpSchedule::LoopUpdate, this, std::placeholders::_1));

		asio::error_code ec;
		m_io_service.run(ec);

		m_loop_timer = CarpAsioTimerPtr();
		m_timer = CarpAsioTimerPtr();
		m_update_func = std::function<void(time_t)>();
	}

	void Exit()
	{
		if (m_is_exit) return;

		m_is_exit = true;
		m_io_service.stop();

		m_thread->join();
		delete m_thread;

	}

private:
	void LoopUpdate(const asio::error_code& ec)
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (m_update_func) m_update_func(time);

		if (!m_loop_timer || m_is_exit) return;
		m_loop_timer->expires_at(std::chrono::system_clock::now() + std::chrono::seconds(0xEFFFFFFF));
		m_loop_timer->async_wait(std::bind(&CarpSchedule::LoopUpdate, this, std::placeholders::_1));
	}

	void TimerImpl(int delay_ms)
	{
		if (!m_timer)
			m_timer = CarpAsioTimerPtr(new CarpAsioTimer(m_io_service, std::chrono::milliseconds(delay_ms)));
		else
			m_timer->expires_after(std::chrono::milliseconds(delay_ms));

		m_timer->async_wait(std::bind(&CarpSchedule::TimerUpdate, this, std::placeholders::_1));
	}

	void TimerUpdate(const asio::error_code& ec)
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (m_update_func) m_update_func(time);
	}

public:
	asio::io_service& GetIOService() { return m_io_service; }
	void Execute(std::function<void()> func) { m_io_service.post(func); }
	bool IsExit() const { return m_is_exit; }

private:
	bool m_is_exit;

private:
	asio::io_service m_io_service;
	CarpAsioTimerPtr m_loop_timer;
	CarpAsioTimerPtr m_timer;
	std::function<void(time_t)> m_update_func;

private:
	std::thread* m_thread;
};

extern CarpSchedule& CarpScheduleInstance();

#ifdef CARP_SCHEDULE_IMPL
CarpSchedule& CarpScheduleInstance()
{
	static CarpSchedule instance;
	return instance;
}
#endif


#endif