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
	// async为true表示异步执行，所有回调会在支线程被调用
	// 如果为false，表示所有回调会在调用Run所在的线程被执行
	void Run(bool async, std::function<void(time_t)> timer_func)
	{
		if (!m_is_exit) return;
		m_is_exit = false;
		m_timer_func = timer_func;

		if (async)
			m_thread = new std::thread(&CarpSchedule::RunImpl, this);
		else
			RunImpl();
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

	void Timer(int delay_ms)
	{
		if (m_is_exit) return;
		m_io_service.post(std::bind(&CarpSchedule::TimerImpl, this, delay_ms));
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
	void RunImpl()
	{
		m_loop_timer = CarpAsioTimerPtr(new CarpAsioTimer(m_io_service, std::chrono::seconds(0xEFFFFFFF)));
		m_loop_timer->async_wait(std::bind(&CarpSchedule::LoopUpdate, this, std::placeholders::_1));

		asio::error_code ec;
		m_io_service.run(ec);

		m_loop_timer = CarpAsioTimerPtr();
		m_timer = CarpAsioTimerPtr();
		m_timer_func = std::function<void(time_t)>();
	}
	
	void LoopUpdate(const asio::error_code& ec)
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (m_timer_func) m_timer_func(time);

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

	void TimerUpdate(const asio::error_code& ec) const
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (m_timer_func) m_timer_func(time);
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
	std::function<void(time_t)> m_timer_func;

private:
	std::thread* m_thread;
};

#endif