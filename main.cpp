#include "config.h"


int main(int argc, char **argv)
{
    Initialize();

    std::thread netThread(networkModule);

    static Frame_Data_Table frame;
//    Wait_Frame_Timer();
    frame = Camera_Get_Frame();
    Update_Frame_Timer();

    while (true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

}





