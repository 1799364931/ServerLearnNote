

#include <list>
#include <map>
#include <sys/epoll.h>
#include <vector>
class WebServer{
    private:
        int serverfd_ ;
        int port_;
        int epollfd_;
        
        std::map<int, int> client_map_;
        std::vector<int> free_nums_;

    public:
        WebServer(int port,int max_client_cnt);

        void InitSocket();

        void InitEpoll();

        void DealListen();

        void DealDown();

        void AddEpollEvent(int fd,int wait_events);
};