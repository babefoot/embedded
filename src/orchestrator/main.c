#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../../include/hardwareManager.h"


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
    printf("Orchestartor started\n");
    return 0;
}


int main(int argc, char const *argv[])
{
    orchestrator();
    return 0;
}
