#include "config.h"


Net_Data_Table Net_Data;
Net_Request_Queue Net_Request_Data;


void Net_Init()
{
    strcpy(Net_Data.server_ipv4, NET_SERVER_INIT_IPV4);
    Net_Data.server_port = NET_SERVER_INIT_PORT;
    Net_Data.server_addr.sin_family = AF_INET;
    Net_Data.server_addr.sin_addr.s_addr = inet_addr(Net_Data.server_ipv4);
    Net_Data.server_addr.sin_port = htons(Net_Data.server_port);

    Net_Data.Start_Connect_Flag = true;
    Net_Data.Already_Connect_Flag = false;

    // 申请socket号码
    Net_Data.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (Net_Data.socket_fd < 0) {
        std::cerr << "Error opening socket." << std::endl;
        Net_Data.Start_Connect_Flag = false;
    }

    // 设置发送超时时间
    Net_Data.timeout.tv_sec = 0;     // 0秒
    Net_Data.timeout.tv_usec = NET_TIMEOUT_INIT_US;    // 500ms
    if (setsockopt(Net_Data.socket_fd, SOL_SOCKET, SO_SNDTIMEO, &(Net_Data.timeout), sizeof(Net_Data.timeout)) < 0) {
        perror("Error setting send timeout");
        Net_Data.Start_Connect_Flag = false;
    }
    Net_Data.timeout.tv_usec = 1000;    // 1ms
    if (setsockopt(Net_Data.socket_fd, SOL_SOCKET, SO_RCVTIMEO, &(Net_Data.timeout), sizeof(Net_Data.timeout)) < 0) {
        perror("Error setting recv timeout");
        Net_Data.Start_Connect_Flag = false;
    }

    // 初始化发送队列
    unsigned char i, j;
    Net_Request_Data.Busy_Flag = true;

    j = 0;
    Net_Request_Data.start_num[j] = 0;
    Net_Request_Data.max_num[j] = NET_EMPTY_MAX;
    for (i = Net_Request_Data.start_num[j]; i < Net_Request_Data.start_num[j+1]; i++){
        Net_Request_Data.block[i].index = j;
        Net_Request_Data.block[i].data = NULL;
        Net_Request_Data.block[i].size_byte = 0;
    }

    j = 1;
    Net_Request_Data.start_num[j] = Net_Request_Data.start_num[j-1] + NET_EMPTY_MAX;
    Net_Request_Data.max_num[j] = NET_HELLO_MAX;
    for (i = Net_Request_Data.start_num[j]; i < Net_Request_Data.start_num[j+1]; i++){
        Net_Request_Data.block[i].index = j;
        char tmp_data[] = "Hello Server From Client";
        Net_Request_Data.block[i].data = tmp_data;
        Net_Request_Data.block[i].size_byte = sizeof(tmp_data);
    }

    j = 2;
    Net_Request_Data.start_num[j] = Net_Request_Data.start_num[j-1] + NET_HELLO_MAX;
    Net_Request_Data.max_num[j] = NET_IMAGE_MAX;
    for (i = Net_Request_Data.start_num[j]; i < Net_Request_Data.start_num[j+1]; i++){
        Net_Request_Data.block[i].index = j;
        Net_Request_Data.block[i].data = NULL;
        Net_Request_Data.block[i].size_byte = 0;
    }

    j = 3;
    Net_Request_Data.total_size = 0;
    Net_Request_Data.start_num[j] = Net_Request_Data.start_num[j - 1] + NET_IMAGE_MAX;

    Net_Request_Data.Busy_Flag = false;
}


void connect_to_server() {
    while(connect(Net_Data.socket_fd, (struct sockaddr*)&(Net_Data.server_addr), sizeof(Net_Data.server_addr)) < 0) {
        Net_Data.connect_failed_times++;
        cerr << "Connection failed " << Net_Data.connect_failed_times << std::endl;
        if (Net_Data.connect_failed_times > NET_MAX_FAIL_TIMES){
            Net_Data.connect_failed_times = 0;
            Net_Data.Start_Connect_Flag = false;
            return;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
    cout << "Connected to server: " << std::endl;
    cout << "IP / Port: " << Net_Data.server_ipv4 << " : " << Net_Data.server_port << std::endl;
    Net_Data.connect_failed_times = 0;
    Net_Data.Already_Connect_Flag = true;
}


inline bool Net_Wait_Queue_Flag(int wait_us)
{
    int ctr;
    while (Net_Request_Data.Busy_Flag){
        ctr++;
        std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
        if (ctr >= NET_MAX_FAIL_TIMES){
            std::cerr << "Failed to Get Net Lock" << std::endl;
            return false;
        }
    }
    return true;
}


bool Net_Add_Queue(int index, void * data, uint32_t size_byte)
{
    if (! Net_Wait_Queue_Flag(1000)){
        return false;
    }

    Net_Request_Data.Busy_Flag = true;
    if (index >= NET_PROTOCOL_NUM){
        std::cerr << "Undefined Net Protocol! Index: " << index << std::endl;
        Net_Request_Data.Busy_Flag = false;
        return false;
    }
    if (Net_Request_Data.size[index] > Net_Request_Data.max_num[index]){
        std::cerr << "No Net Cache Space for " << index << std::endl;
        Net_Request_Data.Busy_Flag = false;
        return false;
    }

    // 循环队列，当前的head总为这一次的节点
    Net_Request_Data.total_size++;
    Net_Request_Data.size[index]++;
    Net_Request_Data.block[Net_Request_Data.head_num[index]].index = index;
    Net_Request_Data.block[Net_Request_Data.head_num[index]].data = data;
    Net_Request_Data.block[Net_Request_Data.head_num[index]].size_byte = size_byte;
    // 上提head节点
    Net_Request_Data.head_num[index]++;
    if (Net_Request_Data.head_num[index] == Net_Request_Data.start_num[index+1]){
        Net_Request_Data.head_num[index] = Net_Request_Data.start_num[index];
    }
    Net_Request_Data.Busy_Flag = false;
    return true;
}


bool Net_Pop_Queue(int index, void ** data, uint32_t * size_byte)
{
    if (! Net_Wait_Queue_Flag(1000)){
        return false;
    }

    Net_Request_Data.Busy_Flag = true;
    if (index >= NET_PROTOCOL_NUM){
        std::cerr << "Undefined Net Protocol! Index: " << index << std::endl;
        Net_Request_Data.Busy_Flag = false;
        return false;
    }
    if (Net_Request_Data.size[index] == 0){
        std::cerr << "No Any Net Cache for " << index << std::endl;
        Net_Request_Data.Busy_Flag = false;
        return false;
    }

    // 循环队列，当前的tail总为这一次的节点
    Net_Request_Data.total_size--;
    Net_Request_Data.size[index]--;
    *size_byte = Net_Request_Data.block[Net_Request_Data.tail_num[index]].size_byte;
    *data = Net_Request_Data.block[Net_Request_Data.tail_num[index]].data;
    // 上提tail节点
    Net_Request_Data.tail_num[index]++;
    if (Net_Request_Data.tail_num[index] == Net_Request_Data.start_num[index+1]){
        Net_Request_Data.tail_num[index] = Net_Request_Data.start_num[index];
    }
    Net_Request_Data.Busy_Flag = false;
    return true;
}


inline void connect_clear()
{
    Net_Data.Already_Connect_Flag = false;
    if(Net_Wait_Queue_Flag(1000)){
        cerr << "Cannot Get Net Lock When Clear Connect!!!" << endl;
    }
    Net_Request_Data.Busy_Flag = true;
    // 清空网络队列
    Net_Request_Data.total_size = 0;
    for (unsigned char i = 0; i < NET_PROTOCOL_NUM; i++){
        Net_Request_Data.head_num[i] = Net_Request_Data.start_num[i];
        Net_Request_Data.tail_num[i] = Net_Request_Data.start_num[i];
        Net_Request_Data.size[i] = 0;
    }
    Net_Request_Data.Busy_Flag = false;
}


void communicate_with_server() {
    uint16_t i, ctr = 0;
    static Net_Head_Block head;

    // 发送
    if (Net_Request_Data.total_size >0){
        uint32_t size_byte; void * data;
        for (i = 0; i < NET_PROTOCOL_NUM; i++){
            while (Net_Request_Data.size[i] > 0){
                if(Net_Pop_Queue(i, &data, &size_byte)){
                    head.index = i; head.size_byte = size_byte;
                    while(send(Net_Data.socket_fd, &head, sizeof(Net_Head_Block), 0) < 0) {
                        ctr++;
                        if (ctr > NET_MAX_FAIL_TIMES){
                            std::cerr << "Failed to send head for index " << i << std::endl;
                            connect_clear();
                            return;
                        }
                        std::this_thread::sleep_for(std::chrono::microseconds(1000));
                    }
                    while(send(Net_Data.socket_fd, data, size_byte, 0) < 0) {
                        ctr++;
                        if (ctr > NET_MAX_FAIL_TIMES){
                            std::cerr << "Failed to send message for index " << i << std::endl;
                            connect_clear();
                            return;
                        }
                        std::this_thread::sleep_for(std::chrono::microseconds(1000));
                    }
                }
            }
        }
    }

    static uint8_t recv_buffer[NET_RECV_BUFFER_MAX_SIZE];
    uint32_t recv_size = 0;
    static uint8_t index = 0;

    // 接收
    if ((recv_size=recv(Net_Data.socket_fd, recv_buffer, NET_RECV_BUFFER_MAX_SIZE, 0)) < 0) {
        return;
    }
    if (recv_size == sizeof(Net_Head_Block)){
        head = *(Net_Head_Block *)(recv_buffer);
    }
    else{
        switch (index) {
            case NET_EMPTY_NUM:
                break;
            case NET_HELLO_NUM:
                cout << recv_buffer << endl;
                break;
            case NET_IMAGE_NUM:
                break;
            default:
                break;
        }
        index = 0;
    }
}


void networkModule()
{
    while (true){
        if (Net_Data.Start_Connect_Flag == false){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        else if (Net_Data.Already_Connect_Flag == false){
            connect_to_server();
        }
        else{
            communicate_with_server();
        }
    }
}
