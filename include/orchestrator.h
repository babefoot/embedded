
int createSubprocesses(void (*callback)(void), char* processName);
void initSubProcesses();
int orchestrator();
void closeAllMq();

#define TYPE_DEBUT 1