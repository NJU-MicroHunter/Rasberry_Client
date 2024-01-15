#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


#define TEST_FRAME_COUNT 3
#define CAMERA_BUFFER_SIZE 4
#define FRAME_MAX_RATE_HZ 10
#define NANOSEC_PER_SEC 1000000000ULL
#define FRAME_INTERVAL_NS (NANOSEC_PER_SEC / FRAME_MAX_RATE_HZ)


int main(int argc, char **argv) {
    int fd; // 文件描述符
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void *buffers[CAMERA_BUFFER_SIZE];
    unsigned int i;
    char *dev_name = "/dev/video0"; // 摄像头设备文件
    // 计时功能
    struct timespec ts_now, ts_next;

    // 打开摄像头设备
    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    // 查询摄像头设备信息
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Cannot query device");
        exit(EXIT_FAILURE);
    }
    printf("Driver: %s\n", cap.driver);
    printf("Card: %s\n", cap.card);
    printf("Bus: %s\n", cap.bus_info);
    printf("Version: %d.%d\n", (cap.version>>16)&&0xff, (cap.version>>24)&&0xff);
    printf("Capabilities: %08x\n", cap.capabilities);

    // 设置视频捕获格式
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640; // 设置宽度
    fmt.fmt.pix.height = 480; // 设置高度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; // Set JPEG format if supported
//    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24; // 设置像素格式
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Cannot set format");
        exit(EXIT_FAILURE);
    }

    // 请求视频缓冲区
    memset(&req, 0, sizeof(req));
    req.count = CAMERA_BUFFER_SIZE;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Cannot request buffers");
        exit(EXIT_FAILURE);
    }

    // 映射视频缓冲区
    for (i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("Cannot query buffer");
            exit(EXIT_FAILURE);
        }
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i] == MAP_FAILED) {
            perror("Cannot mmap buffer");
            exit(EXIT_FAILURE);
        }
    }

    // 开始视频捕获
    for (i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("Cannot queue buffer");
            exit(EXIT_FAILURE);
        }
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Cannot start streaming");
        exit(EXIT_FAILURE);
    }

    // 读取视频帧
    struct timeval tv;
    fd_set fds;
    int r;

    // 初始化时间戳
    if (clock_gettime(CLOCK_REALTIME, &ts_next) == -1) {
        perror("Cannot get current time");
        exit(EXIT_FAILURE);
    }

    // 在这里处理视频帧数据，例如保存为文件或进行图像处理
    for (int frame = 0; frame < TEST_FRAME_COUNT; ++frame) {

        while (clock_gettime(CLOCK_REALTIME, &ts_now) == 0 && ts_now.tv_sec < ts_next.tv_sec ||
               (ts_now.tv_sec == ts_next.tv_sec && ts_now.tv_nsec < ts_next.tv_nsec)) {
            // 等待直到下一帧应该被捕获的时间点
        }

        // 在实际代码中，这里应该是一个循环，等待帧的可用信号（例如使用select()或poll()）
        // 然后使用mmap来获取帧数据
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r == -1) {
            perror("Cannot select");
            exit(EXIT_FAILURE);
        }

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("Cannot dequeue buffer");
            exit(EXIT_FAILURE);
        }

        // 假设buffers[i]包含了一帧RGB数据
        void *frame_data = buffers[buf.index]; // buffers是之前mmap得到的缓冲区数组
        unsigned int frame_size = fmt.fmt.pix.sizeimage; // 你需要知道帧的大小
        // 构造RGB文件名
        char filename[256];
        snprintf(filename, sizeof(filename), "frame_%04d.jpeg", frame);
        // 打开文件以保存RGB数据
        FILE *file = fopen(filename, "wb");
        if (!file) {
            perror("Failed to open file");
            break;
        }
        // 写入JPEG数据到文件
        size_t bytes_written = fwrite(frame_data, 1, frame_size, file);
        if (bytes_written != frame_size) {
            perror("Failed to write frame data");
        }

        // 关闭文件
        fclose(file);

        // ... 可能需要处理其他V4L2相关的步骤，比如重新队列缓冲区 ...
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("Cannot requeue buffer");
            exit(EXIT_FAILURE);
        }

        // 更新下一帧的时间点
        ts_next.tv_nsec += FRAME_INTERVAL_NS;
        if (ts_next.tv_nsec >= NANOSEC_PER_SEC) {
            ts_next.tv_sec++;
            ts_next.tv_nsec -= NANOSEC_PER_SEC;
        }
        // 考虑处理时延：如果处理时间超过了帧间隔，则立即继续
        // 否则，在继续之前等待剩余的时间
        struct timespec ts_diff;
        ts_diff.tv_sec = ts_next.tv_sec - ts_now.tv_sec;
        ts_diff.tv_nsec = ts_next.tv_nsec - ts_now.tv_nsec;
        if (ts_diff.tv_nsec < 0) {
            ts_diff.tv_sec--;
            ts_diff.tv_nsec += NANOSEC_PER_SEC;
        }
        if (ts_diff.tv_sec > 0 || ts_diff.tv_nsec > 0) {
            nanosleep(&ts_diff, NULL); // 等待直到下一帧的预设时间点
        }
    }

    // 停止视频捕获并释放资源
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("Cannot stop streaming");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < req.count; i++) {
        munmap(buffers[i], buf.length);
    }
    close(fd);

    return 0;
}
