#include "AsioIOServicePool.h"
#include <iostream>

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) _nextIOService = 0;
	return service;
}

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioServicePool destruct!!!" << std::endl;
	Stop();
}

void AsioIOServicePool::Stop()
{
	for (auto& work : _works) {
		work->get_io_context().stop();
		work.reset();
	}
	for (auto& t : _threads) {
		t.join();
	}
}

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(size), _works(size)
{
	// 创建io_service线程
	for (std::size_t i = 0; i < size; ++i) {
		_works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
	}

	for (std::size_t i = 0; i < size; ++i) {
		_threads.emplace_back([this, i]() {
			_ioServices[i].run();
		});
	}
}
