#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../include/hardwareManager.h"
#include "../../include/orchestrator.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"

int idMqHardwareManager = 15;
int idMqServerConnection = 25;
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
    createSubprocesses(&hardwareManager, "HardwareManager");
    //createSubprocesses(&serverConnection, "ServerConnection");
}

int orchestrator()
{
    printf("Orchestartor starting....\n");
    mqHardwareManager = openMQ(idMqHardwareManager, 0);
    mqServerConnection = openMQ(idMqServerConnection, 0);
    initSubProcesses();
    printf("Orchestartor started\n");

    // test envoyer une payload a l hardware manager
    // 1er aller-retour 
    sleep(4);
    message debut;
    debut.mtype = 1;
    strcpy(debut.payload, "debut");
    sendToMQ(mqHardwareManager, &debut);

    printf("On attend le retour de l harware manager \n");
    message msgFromHard1;
    receiveFromMQ(mqHardwareManager, &msgFromHard1, 30);
    printf("Orchestrator: message de fin recu\n");

    // 2e aller-retour 
    sleep(4);
    message scan;
    scan.mtype = 3;
    strcpy(scan.payload, "scanCard");
    sendToMQ(mqHardwareManager, &scan);

    printf("On attend le retour de l harware manager \n");
    message msgFromHard2;
    receiveFromMQ(mqHardwareManager, &msgFromHard2, 30);
    printf("Orchestrator: message de fin recu\n");
    //printf("Id card = <%s>", msgFromHard2.payload);

    // 3e aller-retour 
    sleep(4);
    message manche;
    manche.mtype = 2;
    strcpy(manche.payload, "manche");
    sendToMQ(mqHardwareManager, &manche);

    printf("On attend le retour de l harware manager \n");
    message msgFromHard3;
    receiveFromMQ(mqHardwareManager, &msgFromHard3, 30);
    printf("Orchestrator: message de fin recu\n");

    // 3e aller-retour 
    sleep(4);
    message distribution;
    distribution.mtype = 4;
    strcpy(distribution.payload, "distribution");
    sendToMQ(mqHardwareManager, &distribution);

    printf("On attend le retour de l harware manager \n");
    message msgFromHard4;
    receiveFromMQ(mqHardwareManager, &msgFromHard4, 30);
    printf("Orchestrator: message de fin recu\n");

    /*
    sleep(4);
    message commence;
    commence.mtype = 2;
    strcpy(commence.payload, "commence");
    sendToMQ(mqHardwareManager, &commence);

    sleep(4);
    message commence2;
    commence2.mtype = 2;
    strcpy(commence2.payload, "commence 2");
    sendToMQ(mqHardwareManager, &commence2);
    */

    sleep(5);

    closeAllMq();

    return 0;
}


void closeAllMq()
{
    closeMq(mqHardwareManager);
    closeMq(mqServerConnection);
}