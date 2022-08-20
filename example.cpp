//g++ -std=c++11 example.cpp TPool.cpp

#include <chrono>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <iostream>
#include "TPool.hpp"

#define DEFAULT_NB_TASKS 10000

double fact(uint16_t n) {
    double f = 1.;
    for (int i = 1; i < n; ++i)
        f *= i;
    return f;
}

int main(int ac, char **av) {
    std::chrono::steady_clock::time_point begin;
    int nb = (ac < 2 ? DEFAULT_NB_TASKS : atoi(av[1]));
    std::cout << "tasks=" << nb << std::endl;

    //random data
    std::vector<uint16_t> numbers(nb);
    int fd = open("/dev/urandom", O_RDONLY);
    for (int i = 0; i < nb; ++i)
        read(fd, &numbers.data()[i], sizeof(uint16_t));
    close(fd);

    //process w/ pool
    TPool pool;
    pool.launch();
    begin = std::chrono::steady_clock::now();
    for (auto &&n : numbers) {
        std::function<void ()> f = [&n]() { fact(n); };
        pool.addTask(f);
    }
    while (pool.isBusy()) std::this_thread::yield();
    std::cout << "compute time w/ pool = " << std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;

    //process w/o pool
    begin = std::chrono::steady_clock::now();
    for (auto &&n : numbers)
        fact(n);
    std::cout << "compute time w/o pool = " << std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;
}
