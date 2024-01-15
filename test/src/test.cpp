#include "../inc/test.h"


int main(int argc, char **argv)
{
    Test_Camera();
}


void Test_Camera()
{
    Camera_Init();
    Frame_Data_Table frame;
    for (int index = 0; index < TEST_FRAME_COUNT; ++index) {
        frame = Camera_Get_Frame();
        // 构造RGB文件名
        char filename[256];
        snprintf(filename, sizeof(filename), "frame_%04d.jpeg", index);
        // 打开文件以保存RGB数据
        FILE *file = fopen(filename, "wb");
        if (!file) {
            perror("Failed to open file");
        }
        // 写入JPEG数据到文件
        size_t bytes_written = fwrite(frame.frame_data, 1, frame.frame_size, file);
        if (bytes_written != frame.frame_size) {
            perror("Failed to write frame data");
        }
        // 关闭文件
        fclose(file);
    }
}
