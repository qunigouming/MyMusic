#include "AsioIOServicePool.h"
#include "LogManager.h"

AsioIOServicePool::~AsioIOServicePool()
{
	LOG(INFO) << "AsioIOServicePool::~AsioIOServicePool()";
	Stop();
}

boost::asio::io_service& AsioIOServicePool::GetIOService()
{
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) _nextIOService = 0;
	return service;
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

AsioIOServicePool::AsioIOServicePool(std::size_t poolSize) : _ioServices(poolSize), _works(poolSize)
{
	for (std::size_t i = 0; i < poolSize; ++i) {
		_works[i] = std::make_unique<Work>(_ioServices[i]);
	}
	for (std::size_t i = 0; i < poolSize; ++i) {
		_threads.emplace_back([this, i]() {
			_ioServices[i].run();
		});
	}
}
