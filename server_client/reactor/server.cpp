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
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

class Server {
  int sockfd_;

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

public:
  Server(const char *ip, short port) { InitSocket(ip, port); }

  auto InitSocket(const char *ip, short port) -> void {
    sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    int optval = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  }

  auto ServerRun() -> void {
    // reactor 模式
    // 主线程负责进行io注册调用线程
  }

  auto TestPool() -> void {
    ThreadPool<std::function<void()>> tp;
    for (int i = 0; i < 100; i++) {
      // 这里 是捕获变量导致的多线程错误
      tp.AddTask([i]() -> void { std::cout << i << "\n"; });
      // usleep(100);
    }
  }
};

int main() {
  Server server("0.0.0.0", 8080);
  server.TestPool();
}