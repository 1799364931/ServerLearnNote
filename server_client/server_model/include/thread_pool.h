#ifndef THREAD_POOL_H
#define THREAD_POLL_H
#include <algorithm>
#include <any>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <numeric>
#include <optional>
#include <queue>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

template <class Func> class ThreadPool {
    constexpr static int thread_num_ = 2;
    std::mutex latch_queue_;
    std::atomic<int> empty_num_;
    std::condition_variable cv_;
    std::condition_variable cv_push_;
    std::vector<std::thread> threads_;
    std::queue<Func> tasks;

  public:
    ThreadPool() {
        // 条件变量应该用在这里
        empty_num_.store(thread_num_);
        threads_.resize(thread_num_);
        for (auto &thread_iter : threads_) {
            thread_iter = std::thread([&]() {
                // 可以增加一个stop 标志来停止
                while (true) {
                    std::unique_lock<std::mutex> lock(latch_queue_);
                    cv_.wait(lock, [&] { return !tasks.empty(); });
                    auto task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                    empty_num_.fetch_add(1);
                    cv_push_.notify_one();
                }
            });
            // thread_iter.detach();
        }
    };

    ~ThreadPool() {
        for (auto &thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    auto AddTask(Func task) -> void {

        std::unique_lock<std::mutex> lock(latch_queue_);
        cv_push_.wait(lock, [&] { return empty_num_ != 0; });
        empty_num_.fetch_sub(1);
        tasks.push(task);
        lock.unlock();
        cv_.notify_one();
    }
};

#endif