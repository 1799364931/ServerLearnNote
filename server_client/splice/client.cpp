
#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>
int main() {
  auto serverfd = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in sock_addr;
  sock_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  sock_addr.sin_port = htons(8080);
  sock_addr.sin_family = AF_INET;

  auto ret = connect(serverfd, (sockaddr *)&sock_addr, sizeof(sock_addr));
  assert(ret >= 0);

  char buffer[100];
  send(serverfd, "hhhhhh\n", 100,0);
  recv(serverfd,buffer,100,0);
  printf("%s ",buffer);
  close(serverfd);
}