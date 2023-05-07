#include "../../include/hardwareManager.h"


/*
Initialise WiringPi
*/
void initHardware(){
    printf("Initialisaiton de WiringPi\n");
    if (wiringPiSetup() == -1) {
        printf("Erreur : librairie wiringPi non initialisée\n"); 
        exit(1);
    }
}




/*
Permet de détecter quel PID à marquer un but en premier, puis tue le 2e programme de détection de but
return 1 si l'équipe rouge a marqué 
return 2 si l'équipe bleue a marqué 
*/
int waitButeur(pid_t rouge, pid_t bleu){
    int status;
	pid_t pidBut;
    pidBut = wait(&status);
    printf("PID du buteur : <%d>\n", pidBut);
    printf("PID rouge : <%d>\n", rouge);
    printf("PID bleu : <%d>\n", bleu);
    if(pidBut == rouge){
        printf("L'équipe rouge a marqué\n");
        kill(bleu, SIGTERM);
        return 1;
    }
    else if(pidBut == bleu){
        printf("L'équipe bleue a marqué\n");
        kill(rouge, SIGTERM);
        return 2;
    }
    else{
        printf("cas impossible\n");
    }
    return 0;
}



/*
Permet de jouer de détecter un but
renvoie 1 si l'équipe rouge a marqué et 2 si l'équipe bleue a marqué 
*/
int sequenceJouerUnPoint(){

    pidPere = getpid();
    pidRouge = fork();
    if (pidRouge == -1) {
        perror("fork");
        return EXIT_FAILURE;
    } else if (pidRouge == 0) { // Détecter but rouge 
        detectionBut(TRIG_PIN_ROUGE, ECHO_PIN_ROUGE);
        exit(EXIT_SUCCESS);
    } else {
        pidBleu = fork();
        if (pidBleu == -1) {
            perror("fork");
            return EXIT_FAILURE;
        } else if (pidBleu == 0) { // Détecter but bleu
            detectionBut(TRIG_PIN_BLEU, ECHO_PIN_BLEU);
            exit(EXIT_SUCCESS); 
        } else {
            printf("pid du père : <%d>\n", pidPere);
        }
    }

    int buteur = waitButeur(pidRouge, pidBleu);

    pidLeds = fork();
    if (pidLeds == -1) {
        perror("fork");
        return EXIT_FAILURE;
    } else if (pidLeds == 0) { // jouer un son
        const char * command = COMMANDE_SON;
        int cr = system(command);
        if (cr != 0) {
            fprintf(stderr, "Impossible de lancer la commande : %s\n", command);
        }
        exit(EXIT_SUCCESS);
    } else { 
        if(buteur == 1){    // allumer les LED en rouges
            const char * commandLedRouge = COMMANDE_LED_ROUGE;
            int cr = system(commandLedRouge);
            printf("Code retour de la commande : <%d>\n", cr);
            if (cr != 0) {
                fprintf(stderr, "Impossible de lancer la commande : %s\n", commandLedRouge);
            }
        }   
        else{               // allumer les LED en bleu
            const char * commandLedBleu = COMMANDE_LED_BLEU;
            int cr = system(commandLedBleu);
            printf("Code retour de la commande : <%d>\n", cr);
            if (cr != 0) {
                fprintf(stderr, "Impossible de lancer la commande : %s\n", commandLedBleu);
            }
        }

        // gérer la mort du fils 
        int status;
        pid_t pidBut;
        pidBut = wait(&status);
        
    }
    // gérer la mort du fils 
    int statusTest;
    pid_t pidButTest;
    pidButTest = wait(&statusTest);
    //printf("On a attrapé la mort d'un fils <%d>\n", pidButTest);

    return buteur;
}


/*
Initialise la boite aux lettre réceptrice et détruit la boite aux lettre apres avoir recu le message
*/
void initMqRecepteur(){
    key_t key;
    int msgid;
    struct message msg;

    key = ftok("led.h", 10);
    msgid = msgget(key, 0666 | IPC_CREAT);
    msgrcv(msgid, &msg, sizeof(msg), 1, 0);
    printf("Message reçu : %s\n", msg.echange);
    msgctl(msgid, IPC_RMID, NULL);
}


/*
Initalise la boite aux lettres emetteuse, en créant la boite aux lettres 
*/
void initMqEmetteur(){
    key_t key;
    int msgid;
    struct message msg;

    key = ftok("led.h", 10);
    msgid = msgget(key, 0666 | IPC_CREAT);

    msg.mtype = 1;
    strcpy(msg.echange, "salut\n");
    msgsnd(msgid, &msg, sizeof(msg), 0);
}





/**
 * Détecte un but grace au capteur de mouvement 
 * Si une différence trop grande est détectée entre 2 points de mesure, on détecte un but
*/
int detectionBut(int pinTrig, int pinEcho){
    //printf("Initialisation des pins %d et %d\n", pinTrig, pinEcho);
    pinMode(pinTrig, OUTPUT);
    pinMode(pinEcho, INPUT);

    int dist;
    int lastDist;
    int premierTour = 1;
    int pointMesure = 0;

    while(1){
        dist = distance(pinTrig, pinEcho);
        if(premierTour == 1){
            lastDist = dist;
            premierTour = 0;
        }

        if(pointMesure > LANCEMENT){
            if(lastDist - dist > SENSI){
                printf("BUT du pin : <%d>\n", pinEcho);
                sleep(1);
                pointMesure = 0;
                return 0;
            }
        }
        else{
            printf("INITIALISATION\n");
        }
        

        //printf("PIN <%d> vaut %d mm\n", pinEcho, dist);
        //sleep(0.5);
        usleep(500000); // 0.5s

        lastDist = dist;
        pointMesure++;
    }
}




/**
 * Permet de mesurer une distance en utilisant le capteur de distance
*/
int distance(int pinTrig, int pinEcho){

    //printf("Debut de la fonction calculant la distance\n");

    digitalWrite(pinTrig, HIGH);
    //sleep(0.00001);
    usleep(10);
    digitalWrite(pinTrig, LOW);

    clock_t debut = clock();
    clock_t fin = clock();

    while(digitalRead(pinEcho) == 0){
        debut = clock();
    }

    while(digitalRead(pinEcho) == 1){
        fin = clock();
    }

    unsigned long micro = (fin -  debut) * 1000000 / CLOCKS_PER_SEC;
    int distance = ((micro * 340290) / 1000000) / 2;
    return distance;

}


/**
 * Permet d'allumer les LEDs en rouge ou en bleu
*/
/*
void allumerLeds(int equipe){
    if(equipe == 1){
        printf("On allume les LEDs en rouge\n");
        const char * command = COMMANDE_LED_ROUGE;
        int cr = system(command);
        printf("Code retour de la commande : <%d>\n", cr);
        if (cr != 0) {
            fprintf(stderr, "Impossible de lancer la commande : %s\n", command);
        }
        printf("Fin de l'allumage des LEDs en rouge\n");
    }
    else{
        printf("On allume les LEDs en bleu\n");
        const char * command = COMMANDE_LED_BLEU;
        int cr = system(command);
        printf("Code retour de la commande : <%d>\n", cr);
        if (cr != 0) {
            fprintf(stderr, "Impossible de lancer la commande : %s\n", command);
        }
        printf("Fin de l'allumage des LEDs en bleu\n");
    }
}
*/


