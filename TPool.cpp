#include <iostream>
#include <chrono>
#include <thread>
#include <exception>
#include "TPool.hpp"

int TQueue::pop(std::function<void()> &task) {
    std::unique_lock<std::mutex> safe(_lock);

    if (!_queue.empty()) {
        task = _queue.back();
        _queue.pop_back();
        return (0);
    }
    return (-1);
}

int TQueue::push(std::function<void()> &task) {
    std::unique_lock<std::mutex> safe(_lock);
    _queue.emplace_front(std::move(task));
    return (0);
}

size_t TQueue::size() const {
    std::unique_lock<std::mutex> safe(_lock);
    return (_queue.size());
}

void TPool::destroy() {
    _end = true;
    _cnd.notify_all();
    for (unsigned int i = 0; i < _max; ++i)
        if (_workers[i].joinable())
            _workers[i].join();
}

int TPool::addTask(std::function<void()> &task) {
    int ret = 0;
    {
        std::unique_lock<std::mutex> safe(_lock);
        ret = _queue.push(task);
    }
    if (!ret) {
        _cnd.notify_one();
        return (0);
    }
    return (1);
}

const unsigned int &TPool::threadsWorking() const {
    std::unique_lock<std::mutex> safe(_lock);
    return (_workingOn);
}

bool TPool::isBusy() const {
    return (_queue.size() > 0 || threadsWorking() > 0);
}

void TPool::infWorker() {
    std::function<void()> task = nullptr;
    while (!_end) {
        {
            std::unique_lock<std::mutex> safe(_lock);
            if (!_queue.size()) {
                std::this_thread::yield();
                _cnd.wait(safe);
            }
            if (_end && !_queue.size())
                return;
            _queue.pop(task);
            _workingOn++;
        }
        if (task) {
            task();
            task = nullptr;
        }
        {
            std::unique_lock<std::mutex> safe(_lock);
            _workingOn--;
        }
    }
}

