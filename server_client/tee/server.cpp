#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
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
        //利用tee 将收到的内容输出到终端 / 写入file
        
        //tee 的作用是在两个pipe复制数据 即 pipe1 的读端数据 复制到 pipe2的写端 此时缓冲区数据被复制 但不会被读取

        //create pipe
        int pipe_stdout[2];
        int pipe_recv[2];
        ret = pipe(pipe_stdout);
        assert(ret>=0);
        ret = pipe(pipe_recv);
        assert(ret>=0);

        auto fildfd = open("log.txt", O_RDONLY|O_WRONLY|O_CREAT);
        //splice recv and clientfd
        
        splice(clientfd, NULL, pipe_recv[1], NULL, 100, 0);
        tee(pipe_recv[0], pipe_stdout[1], 100, 0);
        splice(pipe_stdout[0], NULL, STDOUT_FILENO, NULL, 100, 0);
        splice(pipe_recv[0],NULL,fildfd,NULL,100,0);

        close(fildfd);
    }

    close(sockfd);    
}