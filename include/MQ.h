
int openMQ(int id);

typedef struct 
{
    long type;
    char payload[1000];
} message;

int sendToMQ(int mqId,  message msg);

int receiveFromMQ(int mqId,  message msg, int type);

