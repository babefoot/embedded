
int createSubprocesses(void (*callback)(void), char* processName);
void initSubProcesses();
int orchestrator();
void closeAllMq();
void* threadMqServeur(void* arg);
int createThreadServeur();
void closeThread();

#define TYPE_DEBUT 1