#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdlib.h>
int main(){
    //socket init
    auto sockfd = socket(AF_INET,SOCK_STREAM,0);

    //setsocketopt
    auto optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    //sockaddr
    sockaddr_in sock_addr;
    sock_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sock_addr.sin_port = htons(8080);
    sock_addr.sin_family = AF_INET;

    //bind
    auto ret = bind(sockfd, (sockaddr*)&sock_addr, sizeof(sock_addr));
    assert(ret != -1);
    
    //listen 
    ret = listen(sockfd, 5);
    assert(ret != -1);

    //accept
    sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    auto clientfd = accept(sockfd, (sockaddr*)&clientaddr, &clientaddr_len);

    if(clientfd < 0){
        fprintf(stderr, "%d \n",errno);
    }

    else{

        //iovec 的作用是分块读取 分块写入？
        std::vector<iovec> iovecs(5);
        for(auto &it:iovecs){
            it.iov_base = new char[]{"abcd\n"};

            //iov_len 要初始化
            it.iov_len = 10;
        }
        writev(clientfd, iovecs.data(), static_cast<int>(iovecs.size()));
        for(auto &it:iovecs){
            delete [] (char*)it.iov_base;
        }

    }
    close(sockfd);    
}