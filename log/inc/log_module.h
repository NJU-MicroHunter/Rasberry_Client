#ifndef RASBERRY_CLIENT_LOG_MODULE_H
#define RASBERRY_CLIENT_LOG_MODULE_H

#define LOG_DIR_INIT "./client.log"

typedef struct Log_Data_Table{
    char log_dir[100];
}Log_Data_Table;

extern Log_Data_Table Log_Data;

void Write_Log();
void Log_Init();

#endif //RASBERRY_CLIENT_LOG_MODULE_H
