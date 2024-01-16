#ifndef HEART_DETECTION_SERVER_NET_MODULE_H
#define HEART_DETECTION_SERVER_NET_MODULE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::seconds
#include <atomic> // for std::atomic_bool
#include "net_protocol.h"
using namespace std;


#define NET_SERVER_INIT_IPV4 "172.26.85.221"
#define NET_SERVER_INIT_PORT 8080
#define NET_TIMEOUT_INIT_US 500000
#define NET_MAX_FAIL_TIMES 3
#define NET_RECV_BUFFER_MAX_SIZE 100000000    //100 MB


typedef struct Net_Data_Table{
    int socket_fd;
    sockaddr_in server_addr;
    char server_ipv4[16];
    int server_port;
    atomic_bool Start_Connect_Flag;
    atomic_bool Already_Connect_Flag;
    struct timeval timeout;
    int connect_failed_times;
}Net_Data_Table;

extern Net_Data_Table Net_Data;

struct Net_Request_Block;
typedef struct Net_Request_Block Net_Request_Block;
typedef struct Net_Request_Block * PNet_Request_Block;

typedef struct Net_Request_Block{
    uint32_t size_byte;
    uint8_t index;
    void *data;
}Net_Request_Block;

typedef struct Net_Request_Queue{
    Net_Request_Block block[NET_PROTOCOL_MAX];
    int size[NET_PROTOCOL_NUM];
    int total_size;
    int max_num[NET_PROTOCOL_NUM];
    int start_num[NET_PROTOCOL_NUM + 1];
    int head_num[NET_PROTOCOL_NUM];
    int tail_num[NET_PROTOCOL_NUM];
    atomic_bool Busy_Flag;
}Net_Request_Queue;

typedef struct Net_Head_Block{
    uint8_t index;
    uint32_t size_byte;
}Net_Head_Block;

extern Net_Request_Queue Net_Request_Data;

void networkModule();
void Net_Init();
bool Net_Add_Queue(int index, void * data);

#endif //HEART_DETECTION_SERVER_NET_MODULE_H
