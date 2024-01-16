#include "log_module.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fstream>
#include <iostream>
using namespace std;


Log_Data_Table Log_Data;


void Write_Log(char * msg)
{
    // 获取时间戳
    // 定义用于存储时间的结构体
    struct timespec ts;
    // 获取自系统启动以来的时间
    if (clock_gettime(CLOCK_BOOTTIME, &ts) != 0) {
        perror("clock_gettime");
    }
    // 格式化时间戳为字符串
    char timestamp[50];
    snprintf(timestamp, sizeof(timestamp), "[%5lu.%06lu]    ", ts.tv_sec, ts.tv_nsec / 1000);

    // 创建输出文件流
    ofstream logfile(Log_Data.log_dir, std::ios::app); // 使用std::ios::app以追加方式写入文件
    if (!logfile.is_open()) {
        cerr << "Failed to open log file " << Log_Data.log_dir << endl;
    }
    else{
        // 将时间写入文件
        logfile << timestamp << msg << endl ;
        // 关闭文件流
        logfile.close();
    }
}


void Log_Init()
{
    strcpy(Log_Data.log_dir, LOG_DIR_INIT);
    // 获取当前时间戳
    time_t current_time = time(NULL);
    // 转换时间戳为本地时间
    struct tm *local_time = localtime(&current_time);
    // 格式化时间字符串
    char time_string[50];
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", local_time);
    // 创建输出文件流
    ofstream logfile(Log_Data.log_dir, std::ios::app); // 使用std::ios::app以追加方式写入文件
    if (!logfile.is_open()) {
        cerr << "Failed to open log file" << Log_Data.log_dir << endl;
    }
    else{
        // 将时间写入文件
        logfile << endl << "[" << time_string << "] " << endl ;
        // 关闭文件流
        logfile.close();
    }
}
