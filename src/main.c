
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "../include/orchestrator.h"

void sigint_handler(int signum);

int main(int argc, char const *argv[])
{
    orchestrator();
    return 0;
}
