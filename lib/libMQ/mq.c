
#include "../../include/MQ.h"
#include <string.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int openMQ(int id, int isReceveur){
    printf("Opening MQ %d\n", id);
    key_t key = ftok("makefile", id);
    printf("key: %d\n", key);

    int msgflg;
    if(isReceveur == 0)
        msgflg = IPC_CREAT | 0666;
    else 
        msgflg = 0666;

    int mqId = msgget(key, msgflg);
    if(mqId < 0){
        printf("Error while opening MQ %d\n", mqId);
        return -1;
    }
    printf("mqId: %d\n", mqId);
    return mqId;
}


//send a message to a message queue with the type specified
int sendToMQ(int mqId, message* msg){
    printf("Sending message to MQ %d\n", mqId);
    printf("Le type du message : <%ld>\n", msg->mtype);
    printf("La payload du message : <%s>\n", msg->payload);
    //int msgSize = sizeof(msg) + 1;
    //size_t sizeM = sizeof(message);
    //size_t test = strlen(msg.payload) + 1;
    //printf("taille de sizeM : <%d>\n", sizeM);
    int msgFlag = 0;
    int msgReturn = msgsnd(mqId, msg, strlen(msg->payload) + 1, msgFlag);
    if(msgReturn < 0){
        printf("Error while sending message to MQ %d\n", mqId);
        return 1;
    }
    return 0;
}

int receiveFromMQ(int mqId, message* msg, long type){
    printf("Receiving message from MQ %d\n", mqId);
    //int msgSize = sizeof(msg) + 1;
    int msgFlag = 0;
    int msgReturn = msgrcv(mqId, msg, MAX_PAYLOAD, type, msgFlag);
    if(msgReturn < 0){
        printf("Error while receiving message from MQ %d\n", mqId);
        return 1;
    }
    printf("Fin reception\n");
    return 0;
}




int closeMq(int idMq)
{
    if (msgctl(idMq, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la fermeture de la MQ");
        return 1;
    }
    printf("La Mq <%d> a été fermée avec succès.\n", idMq);
}