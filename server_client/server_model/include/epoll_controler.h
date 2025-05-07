#ifndef EPOLL_CONTROLER_H
#define EPOLL_CONTROLER_H
#include <cstdio>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>

class EpollControler {

    std::mutex latch_;

    std::unordered_map<int, int> fd_events_map_;

  public:
    int epoll_fd_;
    int timeout_;

    

    EpollControler(int timeout = -1) {
        epoll_fd_ = epoll_create(10);
        timeout_ = timeout;
    }

    auto AddEvent(int fd, int EPOLL_EVENT) -> void {
        epoll_event event;
        event.events = EPOLL_EVENT;
        event.data.fd = fd;
        std::lock_guard<std::mutex> lock(latch_);
        fd_events_map_[fd] = EPOLL_EVENT;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    }

    auto AppendEvent(int fd,int EPOLL_EVENT) -> void{
        std::lock_guard<std::mutex> lock(latch_);
        if(fd_events_map_.find(fd) == fd_events_map_.end()){
            fprintf(stderr, "fd not exist in epoll\n");
            return;
        }
        epoll_event event;
        event.events = EPOLL_EVENT | fd_events_map_[fd];
        event.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd,&event);
    }

    auto DelEvent(int fd) -> void {
        std::lock_guard<std::mutex> lock(latch_);
        fd_events_map_.erase(fd);
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    }

    auto WaitEvent(epoll_event *events, int events_len) -> int {
        return epoll_wait(epoll_fd_, events, events_len, timeout_);
    }
};

#endif