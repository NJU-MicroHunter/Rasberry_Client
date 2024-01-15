#ifndef RASBERRY_CLIENT_CONFIG_H
#define RASBERRY_CLIENT_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "camera.h"
#include "net_module.h"


typedef struct System_Status_Table{
    // Global Status

    // Camera Status
    struct {
        int8_t Activate_Flag : 1;
    }Camera;

    // Network Status

}System_Status_Table;

extern System_Status_Table System_Status;


void Initialize();
void System_Init();

#endif //RASBERRY_CLIENT_CONFIG_H
