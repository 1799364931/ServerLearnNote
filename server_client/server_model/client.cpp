#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const char* SERVER_IP = "127.0.0.1"; // 服务器地址
const int SERVER_PORT = 8080;        // 服务器端口

void clientTask(int thread_id) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Thread " << thread_id << ": Socket creation failed\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Thread " << thread_id << ": Connection failed\n";
        close(sockfd);
        return;
    }

    // 发送线程 ID 信息
    std::string message = "Thread ID: " + std::to_string(thread_id) +"\n";
    send(sockfd, message.c_str(), message.size(), 0);

    // 接收服务器回显信息
    char buffer[1024] = {0};
    recv(sockfd, buffer, sizeof(buffer), 0);
    std::cout << "Thread " << thread_id << " received: " << buffer << std::endl;

    close(sockfd);
}

int main() {
    const int CLIENT_COUNT = 5; // 设定并发客户端数量
    std::vector<std::thread> clients;

    for (int i = 0; i < CLIENT_COUNT; ++i) {
        clients.emplace_back(clientTask, i);
        //usleep(100);
    }

    for (auto& client : clients) {
        client.join();
    }

    return 0;
}
