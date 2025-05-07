#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <sys/socket.h>

// 用于进行IO操作的提交
class Request {
  public:
    enum class TYPE { ACCEPT, READ, WRITE };

    TYPE request_type_;
    int fd;
    sockaddr sockaddr_;
    char *write_buffer_;

    int buffer_length_;

    socklen_t addr_len_;
};

class IOUringControler {
    io_uring ring_;
    constexpr static int QUEUE_SIZE = 32;

  public:
    IOUringControler() { io_uring_queue_init(QUEUE_SIZE, &ring_, 0); }
    ~IOUringControler() { io_uring_queue_exit(&ring_); }

    auto Complete(io_uring_cqe *cqe) -> void { io_uring_cqe_seen(&ring_, cqe); }

    // 注册事件
    //
    auto Accept(int sockfd, Request *r) -> void {
        auto sqe = io_uring_get_sqe(&ring_);
        r->fd = sockfd;
        r->addr_len_ = sizeof(sockaddr_in);
        r->request_type_ = Request::TYPE::ACCEPT;
        io_uring_prep_accept(sqe, r->fd, (sockaddr *)&r->sockaddr_,
                             &r->addr_len_, 0);
        io_uring_submit(&ring_);
        io_uring_sqe_set_data(sqe, r);
    }

    auto Write(int clientfd, Request *r) -> void {
        auto sqe = io_uring_get_sqe(&ring_);
        r->request_type_ = Request::TYPE::WRITE;
        r->fd = clientfd;
        io_uring_prep_write(sqe, r->fd, r->write_buffer_, r->buffer_length_, 0);
        io_uring_sqe_set_data(sqe, r);
    }

    auto Read(int clientfd, Request *r) -> void {
        auto sqe = io_uring_get_sqe(&ring_);
        r->request_type_ = Request::TYPE::READ;
        r->fd = clientfd;
        io_uring_prep_read(sqe, r->fd, r->write_buffer_, r->buffer_length_, 0);
        io_uring_sqe_set_data(sqe, r);
    }
};