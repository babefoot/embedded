/*******************************************************************/
/*                   LIBRAIRIES STANDARDS                          */
/*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>



/*******************************************************************/
/*                          CONSTANTES                             */
/*******************************************************************/
// Capteurs de mouvements
#define SENSI 100
#define LANCEMENT 5
#define FREQUENCE 50000
// LEDs
#define COMMANDE_LED_ROUGE "./data/execLeds -s rouge -g 21"
#define COMMANDE_LED_ROUGE_ABS "/home/julexis/Documents/objetConnecte/projetEmbarque/embedded/lib/rpi_ws281x/test -s rouge -g 21"
#define COMMANDE_LED_BLEU "./data/execLeds -s bleu -g 21"
#define COMMANDE_LED_BLEU_ABS "/home/julexis/Documents/objetConnecte/projetEmbarque/embedded/lib/rpi_ws281x/test -s bleu -g 21"
// Son
#define COMMANDE_SON_ABS "mpg321 /home/julexis/Documents/objetConnecte/son/Goal.mp3"
#define COMMANDE_SON "mpg321 ./data/Goal.mp3"
//Pins
#define TRIG_PIN_ROUGE 8  // Wiring pi 8 === GPIO 2
#define ECHO_PIN_ROUGE 9 // Wiring pi 9 === GPIO 3
#define TRIG_PIN_BLEU 7  // Wiring pi 7 === GPIO 4
#define ECHO_PIN_BLEU 0 // Wiring pi 0 === GPIO 17
// hardwareManager
#define NB_FORK 2
#define NB_BUT 3
// moteur 
#define STEP_PIN 24  // Winring pi 24 === GPIO 19
#define DIR_PIN 25   // Winring pi 25 === GPIO 26



/*******************************************************************/
/*                          PROTOTYPES                             */
/*******************************************************************/
// Capteurs de mouvements 
int distance(int pinTrig, int pinEcho);
int detectionBut(int pinTrig, int pinEcho);
// hardwareManager
int waitButeur(pid_t rouge, pid_t bleu);
int sequenceJouerUnPoint();
void initHardware();
void hardwareManager();
// MQ
void initMqRecepteur();
void initMqEmetteur();
// moteur
void tourner(int sens, int pas);
// RFID
char * readIdCard();



/*******************************************************************/
/*                          VARIABLES GLOBALES                     */
/*******************************************************************/
// hardwareManager
/*
#ifndef HARDWARE_H
#define HARDWARE_H 
pid_t pidPere;
pid_t pidRouge;
pid_t pidBleu;
pid_t pidLeds;
#endif
*/

// MQ
/*
mtype == 10      echange = UUID carte
mtype == 20      echange = "rouge" ou "bleu"
mtype == 30      echnage = vide, action pour faire tourner le moteur si on a perdu une balle     
*/
struct message {
    long mtype;
    char echange[100]; 
};

