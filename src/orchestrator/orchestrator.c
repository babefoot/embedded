#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

// #include "../../include/hardwareManager.h"
#include "../../include/orchestrator.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"
#include "../../include/cJSON.h"


int idMqHardwareManager = 16;
int idMqServerConnection = 26;
int mqHardwareManager;
//int mqHardwareManagerRecept;
int mqServerConnection;
message msgToHardware;

pthread_t threadEcouteServeur;

pid_t pidSubProcesses[2];
int pidSubProcessesCount = 0;


cJSON *json;
char state[10];
int shmid;
char * sharedMemoryOrchestrator;

void callbackServerConnection(){
    printf("ServerConnection started\n");
    serverConnection(shmid);
}


int createSubprocesses(void (*callback)(void), char* processName){
    printf("Creating process %s\n", processName);
    pid_t pid = fork();
    if(pid < 0){
        printf("Error while forking %s\n", processName);
        return 1;
    }else if(pid == 0){
        callback();
        pidSubProcesses[pidSubProcessesCount++] = pid;
        exit(0);
    }
    return 0;
}

int createThreadServeur(){
    int resultat = pthread_create(&threadEcouteServeur, NULL, threadMqServeur, NULL);
    if (resultat != 0) {
        fprintf(stderr, "Erreur lors de la création du thread\n");
        return 1; 
    }
    return 0;
}

void initSubProcesses(){
    // createSubprocesses(&hardwareManager, "HardwareManager");
    initSharedMemory();
    createSubprocesses(&callbackServerConnection, "ServerConnection");
}

void sigint_handler(int signum) {
    printf("Signal SIGINT reçu, on arrete le programme main\n");
    closeAllMq();
    closeThread();
    closeSubProcesses();
    exit(signum);
}

void initSharedMemory() {
    shmid = shmget(IPC_PRIVATE, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
}

int orchestrator()
{
    printf("Orchestartor starting....\n");
    initSubProcesses();
    mqHardwareManager = openMQ(idMqHardwareManager, 0);
    mqServerConnection = openMQ(idMqServerConnection, 0);
    createThreadServeur();
    printf("Orchestartor started\n");
    signal(SIGINT, sigint_handler);

    sharedMemoryOrchestrator = shmat(shmid, NULL, 0);
	if (sharedMemoryOrchestrator == (void*)-1) {
		perror("shmat");
		exit(1);
	}

    // Mq infinie 
    for(;;){
        printf("OR : On attend un message \n");
        receiveFromMQ(mqHardwareManager, &msgToHardware, 0);
        printf("OR : mtype received : <%ld>\n", msgToHardware.mtype);
        printf("OR : Payload received : <%s>\n", msgToHardware.payload);

        switch(msgToHardware.mtype){
            case 30:
                // Une équipe a marqué, on regarde la payload pour savoir si c est rouge ou bleu, puis on envoie dans la MQ de la conenxion serveur
                printf("L'équipe <%s> a marqué\n", msgToHardware.payload);
                printf("On envoie l'information au serveur\n");
                message butEquipeServ;
                butEquipeServ.mtype = 51;
                strcpy(butEquipeServ.payload, msgToHardware.payload);
                sendToMQ(mqServerConnection, &butEquipeServ);

                // on renvoie a l HM le fait d allumer les LED et de jouer un son 
                message animationBut;
                animationBut.mtype = 10;
                strcpy(animationBut.payload, msgToHardware.payload);
                sendToMQ(mqHardwareManager, &animationBut);

                break;
            case 32:
                // On a recu l id de la carte depuis l HM
                printf("On a reçu l'ID de la carte <%s>\n", msgToHardware.payload);
                printf("On envoie l'information au serveur\n");
                message idCardServ;
                idCardServ.mtype = 52;
                strcpy(idCardServ.payload, msgToHardware.payload);
                sendToMQ(mqServerConnection, &idCardServ);
                break;
            default:
                break;
        }
    }

    return 0;
}

/*
Ferme toutes les Message Queue ouvertes précédemment 
*/
void closeAllMq()
{
    closeMq(mqHardwareManager);
    closeMq(mqServerConnection);
}


/*
Créer le thread pour écouter les message venant du processus du serveur
*/
void* threadMqServeur(void* arg) {
    message msgFromServer;
    printf("Thread Serveur started\n");

    for (;;)
    {
        printf("Thread On attend un message du serveur\n");
        receiveFromMQ(mqServerConnection, &msgFromServer, 0);
        printf("thread mtype received : <%ld>\n", msgFromServer.mtype);
        printf("thread : Payload received : <%s>\n", msgFromServer.payload);

        switch(msgFromServer.mtype){
            case 50:
                printf("On a reçu l'information du serveur : <%s>\n", msgFromServer.payload);
                json = parseSharedMemory(sharedMemoryOrchestrator);
                strcpy(state, cJSON_GetObjectItem(json, "state")->valuestring);
                if(strcmp(state, "0") == 0){
                    printf("On a reçu l'information du serveur : <%s>\n", state);
                    printf("On envoie l'information à l'HM\n");
                    message msgInitServ;
                    msgInitServ.mtype = 3;
                    strcpy(msgInitServ.payload, "Get Card ID");
                    sendToMQ(mqHardwareManager, &msgInitServ);
                }

                break;
            default:
                break;
        }
    }
    


    pthread_exit(NULL);
}


/*
Fermer le thread d'ecoute pour le Serveur
*/
void closeThread(){
    int resultat_join = pthread_join(threadEcouteServeur, NULL);
    if (resultat_join != 0) {
        fprintf(stderr, "Erreur lors de la récupération du thread\n");
    }
}

void closeSubProcesses(){
    for(int i = 0; i < pidSubProcessesCount; i++){
        kill(pidSubProcesses[i], SIGKILL);
    }
}


