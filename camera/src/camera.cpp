#include "config.h"


Camera_Data_Table Camera_Data;


void Camera_Init()
{
    unsigned int i;

    // 打开摄像头设备
    Camera_Data.fd = open(Camera_Data.dev_name, O_RDWR | O_NONBLOCK, 0);
    if (Camera_Data.fd == -1) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    // 查询摄像头设备信息
    if (ioctl(Camera_Data.fd, VIDIOC_QUERYCAP, &Camera_Data.cap) == -1) {
        perror("Cannot query device");
        exit(EXIT_FAILURE);
    }
    printf("Driver: %s\n", Camera_Data.cap.driver);
    printf("Card: %s\n", Camera_Data.cap.card);
    printf("Bus: %s\n", Camera_Data.cap.bus_info);
    printf("Version: %d.%d\n", (Camera_Data.cap.version >> 16) != 0, (Camera_Data.cap.version >> 24) != 0);
    printf("Capabilities: %08x\n", Camera_Data.cap.capabilities);

    Camera_Data.width = CAMERA_INIT_WIDTH;
    Camera_Data.height = CAMERA_INIT_HEIGHT;

    // 设置视频捕获格式
    memset(&Camera_Data.fmt, 0, sizeof(Camera_Data.fmt));
    Camera_Data.fmt.fmt.pix.width = Camera_Data.width; // 设置宽度
    Camera_Data.fmt.fmt.pix.height = Camera_Data.height; // 设置高度
    Camera_Data.fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Camera_Data.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; // Set JPEG format if supported
//    Camera_Data.fmt.Camera_Data.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24; // 设置像素格式
    if (ioctl(Camera_Data.fd, VIDIOC_S_FMT, &Camera_Data.fmt) == -1) {
        perror("Cannot set format");
        exit(EXIT_FAILURE);
    }

    // 请求视频缓冲区
    memset(&Camera_Data.req, 0, sizeof(Camera_Data.req));
    Camera_Data.req.count = CAMERA_BUFFER_SIZE;
    Camera_Data.req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Camera_Data.req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(Camera_Data.fd, VIDIOC_REQBUFS, &Camera_Data.req) == -1) {
        perror("Cannot request Camera_Data.buffers");
        exit(EXIT_FAILURE);
    }

    // 映射视频缓冲区
    for (i = 0; i < Camera_Data.req.count; i++) {
        memset(&Camera_Data.buf, 0, sizeof(Camera_Data.buf));
        Camera_Data.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Camera_Data.buf.memory = V4L2_MEMORY_MMAP;
        Camera_Data.buf.index = i;
        if (ioctl(Camera_Data.fd, VIDIOC_QUERYBUF, &Camera_Data.buf) == -1) {
            perror("Cannot query buffer");
            exit(EXIT_FAILURE);
        }
        Camera_Data.buffers[i] = mmap(NULL, Camera_Data.buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, Camera_Data.fd, Camera_Data.buf.m.offset);
        if (Camera_Data.buffers[i] == MAP_FAILED) {
            perror("Cannot mmap buffer");
            exit(EXIT_FAILURE);
        }
    }

    // 开始视频捕获
    for (i = 0; i < Camera_Data.req.count; i++) {
        memset(&Camera_Data.buf, 0, sizeof(Camera_Data.buf));
        Camera_Data.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Camera_Data.buf.memory = V4L2_MEMORY_MMAP;
        Camera_Data.buf.index = i;
        if (ioctl(Camera_Data.fd, VIDIOC_QBUF, &Camera_Data.buf) == -1) {
            perror("Cannot queue buffer");
            exit(EXIT_FAILURE);
        }
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(Camera_Data.fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Cannot start streaming");
        exit(EXIT_FAILURE);
    }

    // 初始化时间戳
    if (clock_gettime(CLOCK_REALTIME, &Camera_Data.ts_next) == -1) {
        perror("Cannot get current time");
        exit(EXIT_FAILURE);
    }
}


// 若中途调整视频窗口大小，则进行重初始化
void Camera_Re_Init()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(Camera_Data.fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("Cannot stop streaming");
        exit(EXIT_FAILURE);
    }

    Camera_Data.fmt.fmt.pix.width = Camera_Data.width; // 设置宽度
    Camera_Data.fmt.fmt.pix.height = Camera_Data.height; // 设置高度

    if (ioctl(Camera_Data.fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Cannot start streaming");
        exit(EXIT_FAILURE);
    }
}


void Camera_End()
{
    // 停止视频捕获并释放资源
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(Camera_Data.fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("Cannot stop streaming");
        exit(EXIT_FAILURE);
    }
    for (unsigned int i = 0; i < Camera_Data.req.count; i++) {
        munmap(Camera_Data.buffers[i], Camera_Data.buf.length);
    }
    close(Camera_Data.fd);
}


Frame_Data_Table Camera_Get_Frame()
{
// 读取视频帧
    struct timeval tv;
    fd_set fds;
    int r;

    // 在实际代码中，这里应该是一个循环，等待帧的可用信号（例如使用select()或poll()）
    // 然后使用mmap来获取帧数据
    FD_ZERO(&fds);
    FD_SET(Camera_Data.fd, &fds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(Camera_Data.fd + 1, &fds, NULL, NULL, &tv);
    if (r == -1) {
        perror("Cannot select");
        exit(EXIT_FAILURE);
    }

    memset(&Camera_Data.buf, 0, sizeof(Camera_Data.buf));
    Camera_Data.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Camera_Data.buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(Camera_Data.fd, VIDIOC_DQBUF, &Camera_Data.buf) == -1) {
        perror("Cannot dequeue buffer");
        exit(EXIT_FAILURE);
    }

    // 假设buffers[i]包含了一帧RGB数据
    Frame_Data_Table frame;
    frame.frame_data = Camera_Data.buffers[Camera_Data.buf.index]; // buffers是之前mmap得到的缓冲区数组
    frame.frame_size = Camera_Data.fmt.fmt.pix.sizeimage; // 你需要知道帧的大小

    // ... 可能需要处理其他V4L2相关的步骤，比如重新队列缓冲区 ...
    if (ioctl(Camera_Data.fd, VIDIOC_QBUF, &Camera_Data.buf) == -1) {
        perror("Cannot requeue buffer");
        exit(EXIT_FAILURE);
    }

    return frame;
}


void Update_Frame_Timer()
{
    // 更新下一帧的时间点
    Camera_Data.ts_next.tv_nsec += FRAME_INTERVAL_NS;
    if (Camera_Data.ts_next.tv_nsec >= NANOSEC_PER_SEC) {
        Camera_Data.ts_next.tv_sec++;
        Camera_Data.ts_next.tv_nsec -= NANOSEC_PER_SEC;
    }
    // 考虑处理时延：如果处理时间超过了帧间隔，则立即继续
    // 否则，在继续之前等待剩余的时间
    struct timespec ts_diff;
    ts_diff.tv_sec = Camera_Data.ts_next.tv_sec - Camera_Data.ts_now.tv_sec;
    ts_diff.tv_nsec = Camera_Data.ts_next.tv_nsec - Camera_Data.ts_now.tv_nsec;
    if (ts_diff.tv_nsec < 0) {
        ts_diff.tv_sec--;
        ts_diff.tv_nsec += NANOSEC_PER_SEC;
    }
    if (ts_diff.tv_sec > 0 || ts_diff.tv_nsec > 0) {
        nanosleep(&ts_diff, NULL); // 等待直到下一帧的预设时间点
    }
}


void Wait_Frame_Timer()
{
    // 在这里处理视频帧数据，例如保存为文件或进行图像处理
    while (clock_gettime(CLOCK_REALTIME, &Camera_Data.ts_now) == 0 && Camera_Data.ts_now.tv_sec < Camera_Data.ts_next.tv_sec ||
           (Camera_Data.ts_now.tv_sec == Camera_Data.ts_next.tv_sec && Camera_Data.ts_now.tv_nsec < Camera_Data.ts_next.tv_nsec)) {
        // 等待直到下一帧应该被捕获的时间点
    }
}
