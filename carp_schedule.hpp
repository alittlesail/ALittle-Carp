#ifndef CARP_SCHEDULE_INCLUDED
#define CARP_SCHEDULE_INCLUDED (1)

#include <asio.hpp>

typedef asio::basic_waitable_timer<std::chrono::system_clock> CarpAsioTimer;
typedef std::shared_ptr<CarpAsioTimer> CarpAsioTimerPtr;

class CarpSchedule
{
public:
	CarpSchedule() : m_is_exit(false) {}
	~CarpSchedule() {}

public:
	void Start(int delay_ms, std::function<void(time_t)> update_func)
	{
		if (!m_is_exit) return;
		if (delay_ms <= 0) delay_ms = 0xEFFFFFFF;
		m_update_func = update_func;
		m_timer = CarpAsioTimerPtr(new CarpAsioTimer(m_io_service, std::chrono::milliseconds(delay_ms)));
		m_timer->async_wait(std::bind(&CarpSchedule::Update, this, std::placeholders::_1, delay_ms));
	}

	void Run()
	{
		asio::error_code ec;
		m_io_service.run(ec);
	}

	void Exit()
	{
		if (m_is_exit) return;
		m_is_exit = true;
		m_timer = CarpAsioTimerPtr();
		m_update_func = std::function<void(time_t)>();
		m_io_service.stop();
	}

private:
	void Update(const asio::error_code& ec, int delay_ms)
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (m_update_func) m_update_func(time);

		if (!m_timer) return;
		m_timer->expires_at(std::chrono::system_clock::now() + std::chrono::seconds(delay_ms));
		m_timer->async_wait(std::bind(&CarpSchedule::Update, this, std::placeholders::_1, delay_ms));
	}

public:
	asio::io_service& GetIOService() { return m_io_service; }
	void Execute(std::function<void()> func) { m_io_service.post(func); }
	bool IsExit() const { return m_is_exit; }

private:
	bool m_is_exit;

private:
	asio::io_service m_io_service;
	CarpAsioTimerPtr m_timer;
	std::function<void(time_t)> m_update_func;
};

#endif