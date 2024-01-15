#include "config.h"


System_Status_Table System_Status;


void Initialize()
{
    System_Init();
    Camera_Init();
}


void System_Init()
{
    System_Status.Camera.Activate_Flag = true;
}
