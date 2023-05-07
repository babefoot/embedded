
#include "../../include/MQ.h"
#include <string.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int openMQ(int id){
    printf("Opening MQ %d\n", id);
    key_t key = ftok("makefile", id);
    printf("key: %d\n", key);
    int mqId = msgget(key, 0666 | IPC_CREAT);
    if(mqId < 0){
        printf("Error while opening MQ %d\n", id);
        return 1;
    }
    printf("mqId: %d\n", mqId);
    return mqId;
}


//send a message to a message queue with the type specified
int sendToMQ(int mqId, message msg){
    printf("Sending message to MQ %d\n", mqId);
    int msgSize = sizeof(msg) + 1;
    int msgFlag = 0;
    int msgReturn = msgsnd(mqId, &msg, msgSize, msgFlag);
    if(msgReturn < 0){
        printf("Error while sending message to MQ %d\n", mqId);
        return 1;
    }
    return 0;
}

int receiveFromMQ(int mqId, message msg, int type){
    printf("Receiving message from MQ %d\n", mqId);
    int msgSize = sizeof(msg) + 1;
    int msgFlag = 0;
    int msgReturn = msgrcv(mqId, &msg, msgSize, 1, msgFlag);
    if(msgReturn < 0){
        printf("Error while receiving message from MQ %d\n", mqId);
        return 1;
    }
    return 0;
}