#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "epoll_controler.h"
#include "thread_pool.h"

class Server {
    int sockfd_;

    int ret_;

    ThreadPool<std::function<void()>> thread_pool_;
    EpollControler epoll_ctler_;

    std::vector<int> clientfds_;

    std::mutex stdout_mtx_;

    constexpr static int LISTEN_MAX = 10;
    // 日志信息
    auto InitLogFile() -> void {
        openlog("ReactorServer", LOG_PERROR | LOG_CONS | LOG_PID, LOG_USER);
        syslog(LOG_INFO,
               "FILE:%s | LINE:%d | FUNCTION:%s LOG SYSTEM OPEN SUCCESS\n",
               __FILE__, __LINE__, __FUNCTION__);
    }

    auto InitSocket(const char *ip, short port) -> void {
        sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in addr;
        addr.sin_addr.s_addr = inet_addr(ip);
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        int optval = 1;
        setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
        ret_ = bind(sockfd_, (sockaddr *)&addr, sizeof(addr));
        assert(ret_ >= 0);
        listen(sockfd_, LISTEN_MAX);
    }

  public:
    Server(const char *ip, short port) {
        InitLogFile();
        InitSocket(ip, port);

        // 增加一个accept事件的监听
        epoll_ctler_.AddEvent(sockfd_, EPOLLIN);
    }
    ~Server() {
        close(sockfd_);
        closelog();
    }

    auto ServerRun() -> void {
        // reactor 模式
        // 主线程负责进行io注册调用线程
        while (true) {
            std::vector<epoll_event> events(10);
            auto events_num = epoll_ctler_.WaitEvent(events.data(), 10);
            for (int i = 0; i < events_num; i++) {
                if (events[i].data.fd == sockfd_ &&
                    events[i].events & EPOLLIN) {
                    AcceptClient();
                    continue;
                }
                if (events[i].events & EPOLLIN) {
                    // 有数据读
                    auto readfd = events[i].data.fd;
                    thread_pool_.AddTask([&] {
                        // 读取数据并输出到屏幕中
                        int pipe_stdout[2];
                        auto ret = pipe(pipe_stdout);
                        if (ret < 0) {
                            syslog(LOG_ERR, "client fd : %d open pipe fail\n",
                                   readfd);
                            return;
                        }
                        splice(readfd, NULL, pipe_stdout[1], NULL, 100, 0);
                        std::lock_guard<std::mutex> lock(stdout_mtx_);
                        splice(pipe_stdout[0], NULL, STDOUT_FILENO, NULL, 100,
                               0);
                        epoll_ctler_.AddEvent(readfd, EPOLLOUT);
                    });
                }
                if (events[i].events & EPOLLOUT) {
                    // 有数据写？
                    auto writefd = events[i].data.fd;
                    thread_pool_.AddTask(
                        [&] { send(writefd, "echo success", 20, 0); });
                }
                if (events[i].events & EPOLLHUP){
                    //主机掉线了
                    syslog(LOG_ERR, "client fd : %d hang up\n",
                                events[i].data.fd);
                }
            }
        }
    }

    // 增加一个client
    auto AcceptClient() -> void {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        auto clientfd =
            accept(sockfd_, (sockaddr *)&client_addr, &client_addr_len);
        if (clientfd < 0) {
            syslog(LOG_ERR, "Connect fail\n");
        } else {
            syslog(LOG_INFO, "Connect success , client fd : %d\n", clientfd);
            clientfds_.push_back(clientfd);
            epoll_ctler_.AddEvent(clientfd, EPOLLIN);
        }
        
    }
};

int main() {
    Server server("0.0.0.0", 8080);
    server.ServerRun();
}