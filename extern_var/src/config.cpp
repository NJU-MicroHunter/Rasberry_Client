#include "config.h"


System_Status_Table System_Status;


void Initialize()
{
    Log_Init();
    System_Init();
    Camera_Init();
    Net_Init();
}


void System_Init()
{
    System_Status.Camera.Activate_Flag = true;
}
