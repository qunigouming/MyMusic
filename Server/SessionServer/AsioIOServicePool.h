#pragma once
#include "Singleton.h"
#include <vector>
#include <boost\asio.hpp>
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;
public:
	boost::asio::io_context& GetIOService();
	~AsioIOServicePool();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
	std::vector<boost::asio::io_context> _ioServices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOService = 0;
};

