#include <sys/epoll.h>

class EpollControler {

  public:
    int epoll_fd_;
    int timeout_ = -1;

    EpollControler(int timeout = -1) {
        epoll_fd_ = epoll_create(10);
        timeout_ = timeout;
    }

    auto AddEvent(int fd, int EPOLL_EVENT) -> void {
        epoll_event event;
        event.events = EPOLL_EVENT;
        event.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    }

    auto WaitEvent(epoll_event *events, int events_len) -> int {
        return epoll_wait(epoll_fd_, events, events_len, timeout_);
    }
};