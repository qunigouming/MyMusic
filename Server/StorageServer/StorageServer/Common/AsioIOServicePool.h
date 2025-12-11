#ifndef __ASIO_IO_SERVICE_POOL_H__
#define __ASIO_IO_SERVICE_POOL_H__
#include <vector>

#include <boost/asio.hpp>

#include "Singleton.h"


class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend class Singleton<AsioIOServicePool>;
    using Work = boost::asio::io_service::work;
    using WorkPtr = std::unique_ptr<Work>;
public:
    ~AsioIOServicePool();

    boost::asio::io_service& GetIOService();
    void Stop();

private:
    AsioIOServicePool(std::size_t poolSize = std::thread::hardware_concurrency());

private:
    std::vector<boost::asio::io_context> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService = 0;
};

#endif