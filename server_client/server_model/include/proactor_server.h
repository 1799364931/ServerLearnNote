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
#include <liburing/io_uring.h>
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
#include <sys/io.h>
#include <liburing.h>

#include "epoll_controler.h"
#include "thread_pool.h"
#include "io_uring_controler.h"

class ProactorServer {
    int sockfd_;

    int ret_;

    ThreadPool<std::function<void()>> thread_pool_;

    EpollControler epoll_ctler_;

    std::vector<int> clientfds_;

    std::mutex stdout_mtx_;

    IOUringControler io_uring_ctler;

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
    ProactorServer(const char *ip, short port) {
        
        InitLogFile();
        InitSocket(ip, port);
        // 增加一个accept事件的监听
        epoll_ctler_.AddEvent(sockfd_, EPOLLIN);
        // 初始化ring
    }
    ~ProactorServer() {
        close(sockfd_);
        closelog();
    }

    auto ServerRun() -> void {
        syslog(LOG_INFO, "PROACTOR SERVER START RUNNING....\n");
        //成功触发后注册写完成事件
        while (true) {
            
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
            epoll_ctler_.AddEvent(clientfd,
                                  EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP |EPOLLET);
        }
    }
};