#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

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
pthread_t threadEcouteServeur;

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

int createThreadServeur(){
    int resultat = pthread_create(&threadEcouteServeur, NULL, threadMqServeur, NULL);
    if (resultat != 0) {
        fprintf(stderr, "Erreur lors de la création du thread\n");
        return 1; 
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
    createThreadServeur();
    initSubProcesses();
    printf("Orchestartor started\n");

    // Simulation : on, recupère un message de la MQ serveur
    sleep(5);
    printf("OR : On a recu un message du serveur: DEBUT\n");
    message msgInitServ;
    msgInitServ.mtype = 3;
    strcpy(msgInitServ.payload, "simulation debut");
    sendToMQ(mqHardwareManager, &msgInitServ);

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


