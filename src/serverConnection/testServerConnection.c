#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

typedef struct {
    long type;
    char payload[256];
} Message;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <message_queue_key>\n", argv[0]);
        return 1;
    }

    key_t key = atoi(argv[1]);
    int mqServerConnection = msgget(key, 0);
    if (mqServerConnection == -1) {
        perror("msgget");
        return 1;
    }

    Message message;
    message.type = 1;
    snprintf(message.payload, sizeof(message.payload), "Test payload");

    if (msgsnd(mqServerConnection, &message, sizeof(message.payload), 0) == -1) {
        perror("msgsnd");
        return 1;
    }

    printf("Message sent to the message queue.\n");

    return 0;
}