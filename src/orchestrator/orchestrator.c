#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// #include "../../include/hardwareManager.h"
#include "../../include/orchestrator.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"

int idMqHardwareManager = 16;
int idMqServerConnection = 26;
int mqHardwareManager;
//int mqHardwareManagerRecept;
int mqServerConnection;
message msgToHardware;



/*
int hardwareManager() 
{
    printf("HardwareManager started\n");
    initHardware();
    //sleep(5);
    mqHardwareManagerRecept = openMQ(idMqHardwareManager, 1);
    receiveFromMQ(mqHardwareManagerRecept, &msgToHardware, 1);
    printf("mtype received : <%ld>\n", msgToHardware.mtype);
    printf("Payload received : <%s>\n", msgToHardware.payload);
    switch(msgToHardware.mtype){
        case 1:
            printf("Debut\n");

            break;
        case 2:
            printf("Joue un point\n");
            break;
        case 3:
            printf("arret de la partie\n");
            break;
        default:
            printf("autre\n");
            break;
    }
    //sequenceJouerUnPoint(); 

    return 0;
}
*/

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
    return 0;
}

void initSubProcesses(){
    // createSubprocesses(&hardwareManager, "HardwareManager");
    createSubprocesses(&serverConnection, "ServerConnection");
}

int orchestrator()
{
    printf("Orchestartor starting....\n");
    mqHardwareManager = openMQ(idMqHardwareManager, 0);
    mqServerConnection = openMQ(idMqServerConnection, 0);
    initSubProcesses();
    printf("Orchestartor started\n");

    for (;;)
    {
        receiveFromMQ(mqHardwareManager, &msgToHardware, 1);
        printf("mtype received : <%ld>\n", msgToHardware.mtype);
    }
    

    closeAllMq();

    return 0;
}


void closeAllMq()
{
    closeMq(mqHardwareManager);
    closeMq(mqServerConnection);
}