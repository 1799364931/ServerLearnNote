#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <unistd.h>

#include "include/reactor_server.h"

int main() {
    #ifdef REACTOR
    ReactorServer server("0.0.0.0", 8080);
    server.ServerRun();
    #else

    #endif
}