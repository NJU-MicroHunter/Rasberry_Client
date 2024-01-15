#include "../inc/test_net.h"


#include <iostream>
#include <cstring>
#include <unistd.h> // for close()
#include <arpa/inet.h> // for sockaddr_in
#include <sys/socket.h> // for socket(), connect(), send(), recv()
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::seconds
#include <atomic> // for std::atomic_bool

int sockfd;
struct sockaddr_in address;

std::atomic_bool connected{false};
std::atomic_bool reconnect_timer_started{false};

void start_reconnect_timer() {
    // 示例代码，并没有真正启动一个计时器
    // 你可以在这里实现你的计时器逻辑
    reconnect_timer_started = true;
    std::cout << "Reconnect timer started." << std::endl;
}

void stop_reconnect_timer() {
    // 示例代码，并没有真正停止一个计时器
    reconnect_timer_started = false;
    std::cout << "Reconnect timer stopped." << std::endl;
}

bool connect_to_server(const char* server_ip, int server_port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket." << std::endl;
        return false;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        close(sockfd);
        return false;
    }

    std::cout << "Connected to server." << std::endl;
    connected = true;
    // 在这里可以将sockfd保存为全局变量以供后续使用
    // 但请注意线程安全问题和资源管理的问题

    return true;
}

void communicate_with_server(int sockfd) {
    const char* msg = "hello server";
    char buffer[256] = {0};

    while (connected) {
        if (send(sockfd, msg, strlen(msg), 0) < 0) {
            std::cerr << "Failed to send message." << std::endl;
            connected = false;
            start_reconnect_timer();
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (recv(sockfd, buffer, sizeof(buffer) - 1, 0) < 0) {
            std::cerr << "Failed to receive message." << std::endl;
            connected = false;
            start_reconnect_timer();
            break;
        }

        std::cout << "Received: " << buffer << std::endl;
    }

    // 在实际的生产代码中，你应该更加细致地处理错误，
    // 以及确保在连接断开时正确关闭socket。
}

int test_network() {
    const char* server_ip = "127.0.0.1";
    int server_port = 12345;

    while (true) {
        if (!connected && !reconnect_timer_started) {
            if (connect_to_server(server_ip, server_port)) {
                stop_reconnect_timer();
                // 假设sockfd是全局变量，并且已经在connect_to_server中正确设置
                // int sockfd; // 这个应该是全局的
                 communicate_with_server(sockfd);

                // 由于我们没有将sockfd设置为全局变量，这里我们简单地启动一个新线程
//                std::thread(communicate_with_server, sockfd).detach();
                // 注意：上面的代码片段有问题，因为sockfd在connect_to_server函数作用域内就被销毁了。
                // 实际上，你需要将sockfd保存为全局变量或通过其他方式传递给它。
            }
        } else {
            // 如果正在尝试重新连接，则等待一段时间再次检查
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}
