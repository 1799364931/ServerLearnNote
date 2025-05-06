
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


  
  std::vector<iovec> iovecs(5);
  for (auto &it : iovecs) {
    it.iov_base = new char[10];
    it.iov_len = 10;
  }

  readv(serverfd, iovecs.data(), static_cast<int>(iovecs.size()));

  for (auto &it : iovecs) {
    printf("%s", (char *)it.iov_base);

    delete[] (char *)it.iov_base;
  }
  close(serverfd);
}