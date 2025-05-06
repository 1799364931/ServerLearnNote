
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
int main(){
    auto serverfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sock_addr;
    sock_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sock_addr.sin_port = htons(8080);
    sock_addr.sin_family = AF_INET;
    
    auto ret =  connect(serverfd, (sockaddr*)&sock_addr, sizeof(sock_addr));
    assert(ret >= 0);

    // close(STDIN_FILENO);
    // dup(serverfd);
    
    // while(true){
    //     char buffer[2];
    //     buffer[0] = getchar();
    //     if(buffer[0] == '#'){
    //         break;
    //     }

    //     send(serverfd, buffer, 2, 0);
    // }
    char buffer[100];
    recv(serverfd, buffer, 100 , 0);
    
    printf("len %ld ",strlen(buffer));
    for(int i = 0 ;i < strlen(buffer);i++){
        printf("%c",buffer[i]);
    }
    close(serverfd);
}