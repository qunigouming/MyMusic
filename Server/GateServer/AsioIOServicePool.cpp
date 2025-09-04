#include "AsioIOServicePool.h"

AsioIOServicePool::~AsioIOServicePool()
{
	Stop();
	std::cout << "AsioIOServicePool are destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size())	_nextIOService = 0;
	return service;
}

void AsioIOServicePool::Stop()
{
	for (auto& work : _works) {
		work->get_io_context().stop();
		work.reset();
	}

	for (auto& t : _threads)
		t.join();
}

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(size), _works(size)
{
	for (std::size_t i = 0; i < size; ++i) {
		//创建work管理io_content
		_works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
		//创建线程运行io_content
		_threads.emplace_back([this, i] {
			_ioServices[i].run();
		});
	}
}
