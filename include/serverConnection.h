#include <sys/ipc.h>
#include <sys/shm.h>
#include "./cJSON.h"
#define TOKEN "#^lxn6`S@Z9CGD"
#define SHARED_MEMORY_SIZE 1024
void serverConnection();

void initWs();
cJSON *parseSharedMemory(char *sharedMemory);
