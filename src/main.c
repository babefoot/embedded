
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "../include/orchestrator.h"
void sigint_handler(int signum);

int main(int argc, char const *argv[])
{
    // on déroute les signaux SIGINT
    signal(SIGINT, sigint_handler);

    orchestrator();
    return 0;
}


void sigint_handler(int signum) {
    printf("Signal SIGINT reçu, on arrete le programme main\n");
    printf("On ferme toutes les MQ\n");
    closeAllMq();
    exit(signum);
}