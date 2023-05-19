#include "../../include/hardwareManager.h"
#include "../../include/MQ.h"
#include "../../include/RFID/config.h"
#include "../../include/RFID/rc522.h"
#include "../../include/RFID/rfid_sensor.h"
#include "../../include/RFID/rfid.h"
#include "../../include/RFID/value.h"

// Seul protitype concernant le RFID
char * readIdCard();

pid_t pidPere;
pid_t pidRouge;
pid_t pidBleu;
pid_t pidLeds;

extern int idMqHardwareManager;
//int idMqServerConnection;
//int mqHardwareManager;
int mqHardwareManagerRecept;
message msgFromOrches;
message finInstruction;

pthread_t threadEcouteSon;

void hardwareManager() 
{
    printf("HardwareManager started\n");
    initHardware();
    mqHardwareManagerRecept = openMQ(idMqHardwareManager, 1);
    
    // MQ infinie 
    for(;;){
        int equipeBut = 0;
        char * card; 
        printf("HM : on attend un message\n");
        receiveFromMQ(mqHardwareManagerRecept, &msgFromOrches, 0);
        printf("HM : mtype received : <%ld>\n", msgFromOrches.mtype);
        printf("HM : Payload received : <%s>\n", msgFromOrches.payload);

        switch(msgFromOrches.mtype){
            case 1:
                // Debut de la partie, on jour un son 
                printf("HM : Debut\n");
                const char * command = COMMANDE_SON;
                int cr = system(command);
                if (cr != 0) {
                    fprintf(stderr, "Impossible de lancer la commande : %s\n", command);
                }
                break;
            case 2:
                // useless
                printf("HM : Joue un point\n");
                equipeBut = sequenceJouerUnPoint();
                message finBut;
                finBut.mtype = 31;
                if(equipeBut == 1)
                    strcpy(finBut.payload, "rouge");
                else
                    strcpy(finBut.payload, "bleu");
                sendToMQ(mqHardwareManagerRecept, &finBut);
                break;
            case 3:
                // Scanner une carte puis envoyer dans la MQ vers l orchestrateur
                printf("HM : On demande de scanner une carte\n");
                card = readIdCard();
                message finCard;
                finCard.mtype = 32;
                strcpy(finCard.payload, card);
                sendToMQ(mqHardwareManagerRecept, &finCard);
                break;
            case 4:
                // Distribuer une balle, on n'envoie pas de message 
                printf("HM : On distribue une balle\n");
                tourner(1, 100);
                break;
            case 5:
                // useless 
                printf("HM: arret de la partie\n");
                break;
            case 10:
                printf("HM : Animation pour le buteur\n");
                createThreadSon();
                if(strcmp(msgFromOrches.payload, "rouge") == 0){
                    LedRouge();
                }
                else{
                    LedBleu();
                }
                closeThreadSon();
                break;
            default:
                printf("autre\n");
                break;
            
        }
    }
    printf("On est sortie du hardwware\n");
}





/*
Initialise WiringPi
*/
void initHardware(){
    printf("Initialisaiton de WiringPi\n");
    if (wiringPiSetup() == -1) {
        printf("Erreur : librairie wiringPi non initialisée\n"); 
        exit(1);
    }

    // interruptions pour détecter les buts
    wiringPiISR(LASER_ROUGE, INT_EDGE_RISING, handle_interrupt_rouge);
    wiringPiISR(LASER_BLEU, INT_EDGE_RISING, handle_interrupt_bleu);
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
        //pid_t pidBut;
        //pidBut = wait(&status);
        wait(&status);
        
    }
    // gérer la mort du fils 
    int statusTest;
    //pid_t pidButTest;
    //pidButTest = wait(&statusTest);
    wait(&statusTest);
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

        printf("Distance : <%d>\n", dist);

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
        usleep(FREQUENCE); // 0.5s

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



/*
Faire tourner le moteur, 1 tour = 200 pas
Dans le sens horaire, mettre sens à 1, dans l'autre sens anti-horaire,
mettre le sens différent de 1
*/
void tourner(int sens, int pas){

    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);

    printf("On fait tourne le moteur de %d ", pas);

    if(sens == 1){
        printf("dans le sens horaire\n");
        digitalWrite(DIR_PIN, HIGH);
    }
    else{
        printf("dans le sens anti-horaire\n");
        digitalWrite(DIR_PIN, LOW);
    }
        
    for(int i = 0; i<pas ; i++){
        digitalWrite(STEP_PIN, HIGH);
        usleep(VITESSE); // == 5ms == 0.005s
        digitalWrite(STEP_PIN, LOW);
        usleep(VITESSE); // == 5ms == 0.005s
    }

    printf("Fin de la rotation du moteur\n");

}



/*
Renvoie l'Id de la carte scannée en Hexadecimal
*/
char * readIdCard() {

	//double set_val = 0;
 
    if (geteuid() != 0)
    {
        p_printf(RED,"Must be run as root.\n");
        exit(1);
    }

	set_signals();
    if (get_config_file()) exit(1);
    if (HW_init(spi_speed,gpio)) close_out(1);
    
	InitRc522();

    printf("Scan de la carte en cours \n");
    char * retourCard = disp_card_details_idCard(); 
    printf("ID reçu : <%s>\n", retourCard);
    close_out2(); 

    return retourCard;
}

/*
Interruption, l'équipe rouge a marqué, on prévient le serveur et on allume les LED en rouge
*/
void handle_interrupt_rouge(){
    printf("R O U G E\n");
    message finInstruction;
    finInstruction.mtype = 30;
    strcpy(finInstruction.payload, "rouge");
    sendToMQ(mqHardwareManagerRecept, &finInstruction);
}


/*
Interruption, l'équipe bleu a marqué, on prévient le serveur et on allume les LED en bleu
*/
void handle_interrupt_bleu(){
    printf("B L E U\n");
    message finInstruction;
    finInstruction.mtype = 30;
    strcpy(finInstruction.payload, "bleu");
    sendToMQ(mqHardwareManagerRecept, &finInstruction);
}


/*
Allume les LED en rouge pendant 5 secondes
*/
void LedRouge(){
    const char * commandLedRouge = COMMANDE_LED_ROUGE;
    int cr = system(commandLedRouge);
    printf("Code retour de la commande : <%d>\n", cr);
    if (cr != 0) {
        fprintf(stderr, "Impossible de lancer la commande : %s\n", commandLedRouge);
    }
}

/*
Allume les LED en bleu pendant 5 secondes
*/
void LedBleu(){
    const char * commandLedBleu = COMMANDE_LED_BLEU;
    int cr = system(commandLedBleu);
    printf("Code retour de la commande : <%d>\n", cr);
    if (cr != 0) {
        fprintf(stderr, "Impossible de lancer la commande : %s\n", commandLedBleu);
    }
}


/*
Joue un son pendant 3 secondes
*/
void * jouerSon(){
    const char * command = COMMANDE_SON;
    int cr = system(command);
    if (cr != 0) {
        fprintf(stderr, "Impossible de lancer la commande : %s\n", command);
    }
    pthread_exit(NULL);
}


/*
Crée un thread pour jouer un son
*/
int createThreadSon(){
    int resultat = pthread_create(&threadEcouteSon, NULL, jouerSon, NULL);
    if (resultat != 0) {
        fprintf(stderr, "Erreur lors de la création du thread\n");
        return 1; 
    }
    return 0;
}

/*
Fermer le thread d'ecoute pour le Son
*/
void closeThreadSon(){
    int resultat_join = pthread_join(threadEcouteSon, NULL);
    if (resultat_join != 0) {
        fprintf(stderr, "Erreur lors de la récupération du thread\n");
    }
}