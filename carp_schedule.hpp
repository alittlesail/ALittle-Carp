#ifndef CARP_SCHEDULE_INCLUDED
#define CARP_SCHEDULE_INCLUDED

#include <asio.hpp>
#include <thread>

typedef asio::basic_waitable_timer<std::chrono::system_clock> CarpAsioTimer;
typedef std::shared_ptr<CarpAsioTimer> CarpAsioTimerPtr;

class CarpSchedule
{
public:
	CarpSchedule() {}
	virtual ~CarpSchedule() { Exit(); }

public:
	// async为true表示异步执行，所有回调会在支线程被调用
	// 如果为false，表示所有回调会在调用Run所在的线程被执行
	void Run(bool async)
	{
		if (!m_is_exit) return;
		m_is_exit = false;

		if (async)
			m_thread = new std::thread(&CarpSchedule::RunFull, this);
		else
			RunFull();
	}

	// 同步run一个，所有回调会在调用Run所在的线程被执行
	void RunOne()
	{
		asio::error_code ec;
		if (m_io_service.run_one(ec) == 0)
		{
			m_io_service.restart();
			m_io_service.run_one(ec);
		}
	}

	// 同步pool一个，所有回调会在调用Run所在的线程被执行
	void PollOne()
	{
		asio::error_code ec;
		if (m_io_service.poll_one(ec) == 0)
		{
			m_io_service.restart();
			m_io_service.poll_one(ec);
		}
	}

	void TimerOnce(int delay_ms, const std::function<void(time_t)>& timer_func)
	{
		if (m_thread)
			m_io_service.post(std::bind(&CarpSchedule::TimerOnceImpl, this, delay_ms, timer_func));
		else
			TimerOnceImpl(delay_ms, timer_func);
	}

	void TimerLoop(int interval_ms, const std::function<void(time_t)>& timer_func)
	{
		if (m_thread)
			m_io_service.post(std::bind(&CarpSchedule::TimerLoopImpl, this, interval_ms, timer_func));
		else
			TimerLoopImpl(interval_ms, timer_func);
	}

	void Exit()
	{
		if (m_is_exit) return;

		m_is_exit = true;
		m_io_service.stop();

		if (m_thread)
		{
			m_thread->join();
			delete m_thread;
			m_thread = nullptr;
		}
	}

private:
	void RunFull()
	{
		m_keep_run = std::make_shared<CarpAsioTimer>(m_io_service, std::chrono::seconds(0xEFFFFFFF));
		m_keep_run->async_wait(std::bind(&CarpSchedule::LoopUpdate, this, std::placeholders::_1));

		asio::error_code ec;
		m_io_service.run(ec);
		m_io_service.restart();

		m_keep_run = CarpAsioTimerPtr();
		m_timer_once = CarpAsioTimerPtr();
		m_timer_loop = CarpAsioTimerPtr();
	}
	
	void LoopUpdate(const asio::error_code& ec)
	{
		if (!m_keep_run || m_is_exit) return;
		m_keep_run->expires_after(std::chrono::seconds(0xEFFFFFFF));
		m_keep_run->async_wait(std::bind(&CarpSchedule::LoopUpdate, this, std::placeholders::_1));
	}

	void TimerLoopImpl(int interval_ms, const std::function<void(time_t)>& timer_func)
	{
		if (!m_timer_loop)
			m_timer_loop = std::make_shared<CarpAsioTimer>(m_io_service, std::chrono::milliseconds(interval_ms));
		else
			m_timer_loop->expires_after(std::chrono::milliseconds(interval_ms));
		
		m_timer_loop->async_wait(std::bind(&CarpSchedule::TimerLoopUpdate, this, std::placeholders::_1, interval_ms, timer_func));
	}

	void TimerLoopUpdate(const asio::error_code& ec, int interval_ms, const std::function<void(time_t)>& timer_func) const
	{
		const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		timer_func(time);

		if (!m_timer_loop || m_is_exit) return;
		m_timer_loop->expires_after(std::chrono::milliseconds(interval_ms));
		m_timer_loop->async_wait(std::bind(&CarpSchedule::TimerLoopUpdate, this, std::placeholders::_1, interval_ms, timer_func));
	}

	void TimerOnceImpl(int delay_ms, const std::function<void(time_t)>& timer_func)
	{
		if (!m_timer_once)
			m_timer_once = std::make_shared<CarpAsioTimer>(m_io_service, std::chrono::milliseconds(delay_ms));
		else
			m_timer_once->expires_after(std::chrono::milliseconds(delay_ms));

		m_timer_once->async_wait(std::bind(&CarpSchedule::TimerOnceUpdate, this, std::placeholders::_1, timer_func));
	}

	void TimerOnceUpdate(const asio::error_code& ec, const std::function<void(time_t)>& timer_func) const
	{
		const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (timer_func) timer_func(time);
	}

public:
	asio::io_service& GetIOService() { return m_io_service; }
	void Execute(std::function<void()> func) { m_io_service.post(func); }
	bool IsExit() const { return m_is_exit; }

private:
	bool m_is_exit = true;

private:
	asio::io_service m_io_service;
	CarpAsioTimerPtr m_keep_run;
	CarpAsioTimerPtr m_timer_once;
	CarpAsioTimerPtr m_timer_loop;

private:
	std::thread* m_thread = nullptr;
};

extern CarpSchedule s_carp_schedule;

#endif

#ifdef CARP_SCHEDULE_IMPL
#ifndef CARP_SCHEDULE_IMPL_INCLUDE
#define CARP_SCHEDULE_IMPL_INCLUDE
CarpSchedule s_carp_schedule;
#endif
#endif