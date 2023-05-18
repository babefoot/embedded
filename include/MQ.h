
#define MAX_PAYLOAD 1000

int openMQ(int id, int isReceveur);

typedef struct 
{
    long mtype;
    char payload[MAX_PAYLOAD];
} message;

int sendToMQ(int mqId, message* msg);

int receiveFromMQ(int mqId, message* msg, long type);

int closeMq(int idMq);

