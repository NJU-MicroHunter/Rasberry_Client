#ifndef RASBERRY_CLIENT_CAMERA_H
#define RASBERRY_CLIENT_CAMERA_H

#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#define TEST_FRAME_COUNT 3
#define CAMERA_BUFFER_SIZE 4
#define FRAME_MAX_RATE_HZ 10
#define NANOSEC_PER_SEC 1000000000ULL
#define FRAME_INTERVAL_NS (NANOSEC_PER_SEC / FRAME_MAX_RATE_HZ)
#define CAMERA_INIT_WIDTH 640
#define CAMERA_INIT_HEIGHT 480

typedef struct Camera_Data_Table{
    int fd; // 文件描述符
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void *buffers[CAMERA_BUFFER_SIZE];
    char *dev_name = "/dev/video0"; // 摄像头设备文件
    struct timespec ts_now, ts_next;
    int width, height;
}Camera_Data_Table;

typedef struct Frame_Data_Table{
    void *frame_data;
    unsigned int frame_size;
}Frame_Data_Table;

extern Camera_Data_Table Camera_Data;

void Camera_Init();
void Camera_Re_Init();
Frame_Data_Table Camera_Get_Frame();
void Update_Frame_Timer();
void Wait_Frame_Timer();


#endif //RASBERRY_CLIENT_CAMERA_H
