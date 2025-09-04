#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>

//阻塞队列，使操作变为线程安全
template <typename T>
class BlockQueue
{
public:
    explicit BlockQueue(int MaxCapacity = 1000) : _capacity(MaxCapacity)
    {
        assert(MaxCapacity > 0);
        _isClose = false;
    }

    ~BlockQueue()
    {
        Close();
    }

    void clear()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        while (!_q.empty()) {
            _q.pop();
        }
        _condProducer.notify_all();
        _condConsumer.notify_all();
    }

    void clear(std::function<void(T&)> cleaner)
    {
        std::lock_guard<std::mutex> locker(_mtx);
        while (!_q.empty()) {
            T tmp = _q.front();
            cleaner(tmp);
            _q.pop();
        }
        _condProducer.notify_all();
        _condConsumer.notify_all();
    }
    bool empty()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _q.empty();
    }
    bool full()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _q.size() >= _capacity;
    }
    void Close()
    {
        {
            std::lock_guard<std::mutex> locker(_mtx);
            while (!_q.empty()) _q.pop();
            _isClose = true;
        }
        _condProducer.notify_all();
        _condConsumer.notify_all();
    }
    int size()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _q.size();
    }
    int capacity()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _capacity;
    }

    T front()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _q.front();
    }				//返回头部数据
    void push_back(const T& item)
    {
        std::unique_lock<std::mutex> locker(_mtx);
        while (_q.size() >= _capacity) {
            _condProducer.wait(locker);		//等待消费者线程消费
        }
        _q.push(item);
        _condConsumer.notify_one();
    }
    bool pop(T& item)
    {
        std::unique_lock<std::mutex> locker(_mtx);
        while (_q.empty()) {
            _condConsumer.wait(locker);		//等待生产者线程生产
            if (_isClose)	return false;
        }
        item = _q.front();
        _q.pop();
        _condProducer.notify_one();
        return true;
    }

private:
    std::queue<T> _q;
    int _capacity;
    bool _isClose;
    std::mutex _mtx;
    std::condition_variable _condConsumer;
    std::condition_variable _condProducer;
};

#endif // BLOCKQUEUE_H
