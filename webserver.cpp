#include "webserver.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

WebServer::WebServer(int port,int max_client_cnt):port_(port){
    free_nums_.resize(max_client_cnt);
    for(int i = 0 ; i<free_nums_.size() ; i++){
        free_nums_[i] = i;
    }
    InitEpoll();
    InitSocket();
}

void WebServer::InitEpoll(){
    epollfd_ = epoll_create(0);
    assert(epollfd_ < 0);
}

void WebServer::InitSocket(){
    serverfd_ = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    auto ret = bind(serverfd_, (sockaddr*)&serv_addr,sizeof(serv_addr));
    assert(ret > 0);
    ret = listen(serverfd_, SOMAXCONN);
    assert(ret > 0);
}

void WebServer::AddEpollEvent(int fd,int wait_events){
    struct epoll_event ev;
    ev.events = wait_events;
    ev.data.fd = fd;
    epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
}

void WebServer::DealListen(){
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    auto clientfd = accept(serverfd_,(sockaddr*)&client_addr,&client_addr_len);
    assert(clientfd > 0);
    if(free_nums_.empty()){
        perror("reach the max client connect");
        return ;
    }
    client_map_[clientfd] = free_nums_.back();
    free_nums_.pop_back();
}

void WebServer::DealDown(){

}