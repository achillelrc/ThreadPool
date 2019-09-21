#ifndef TPOOL_HPP
#define TPOOL_HPP

#include <condition_variable>
#include <iostream>
#include <utility>
#include <thread>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <functional>

class TQueue {
public:
    TQueue() = default;
    ~TQueue() = default;
    int push(std::function<void()> &);
    int pop(std::function<void()> &);
    size_t size() const;

private:
    mutable std::mutex _lock;
    std::deque<std::function<void()> > _queue;
};

class TPool {
private:
    void infWorker();
public:
    TPool(const unsigned int &max) : _max(max), _end(false), _queue(), _workingOn(0) {
        _workers = std::vector<std::thread>(max);
    }

    TPool() : _max(std::thread::hardware_concurrency()), _end(false), _queue(), _workingOn(0) {
        _workers = std::vector<std::thread>(_max);
    }

    ~TPool() {
        destroy();
    }

    const unsigned int &getMax() const {
        return (_max);
    }

    inline void launch() {
        for (auto i = _workers.begin(); i != _workers.end(); ++i)
            (*i) = std::thread([=] { infWorker(); });
    }

    void destroy();
    int addTask(std::function<void()> &task);
    const unsigned int &threadsWorking() const;
    bool isBusy() const;
private:
    bool _end;
    unsigned int _max;
    unsigned int _workingOn;
    mutable std::mutex _lock;
    std::condition_variable _cnd;
    std::vector<std::thread> _workers;
    TQueue _queue;
};

#endif //TPOOL_HPP

