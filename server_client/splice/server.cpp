#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <vector>
#include <fcntl.h>
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


        //零拷贝回射服务器 原样返回数据

        //利用 splice + pipe 使数据从io缓冲区直接发送到网卡缓冲区 无需经过拷贝
        int pipe_io[2];
        ret = pipe(pipe_io);
        assert(ret >= 0);
        //bind pipe 
        ret = splice(clientfd, NULL, pipe_io[1], NULL, 100, 0);
        assert(ret >= 0);
        splice(pipe_io[0], NULL, clientfd, NULL, 100, 0);
        assert(ret >= 0);
        while(true);
        
    }
    close(sockfd);    
}