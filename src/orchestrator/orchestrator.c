#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../include/hardwareManager.h"
#include "../../include/orchestrator.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"

int idMqHardwareManager = 1;
int idMqServerConnection = 2;
int mqHardwareManager;
int mqServerConnection;


int hardwareManager() 
{
    printf("HardwareManager started\n");
    initHardware();
    sequenceJouerUnPoint(); 
    return 0;
}

int serverConnection()
{
    printf("ServerConnection started\n");
    initWs();
    return 0;
}

int createSubprocesses(void (*callback)(void), char* processName){
    printf("Creating process %s\n", processName);
    pid_t pid = fork();
    if(pid < 0){
        printf("Error while forking %s\n", processName);
        return 1;
    }else if(pid == 0){
        callback();
        exit(0);
    }
}

void initSubProcesses(){
    createSubprocesses(&hardwareManager, "HardwareManager");
    createSubprocesses(&serverConnection, "ServerConnection");
}

int orchestrator()
{
    printf("Orchestartor starting....\n");
    initSubProcesses();
    openMQ(idMqHardwareManager);
    openMQ(idMqServerConnection);
    printf("Orchestartor started\n");
    return 0;
}