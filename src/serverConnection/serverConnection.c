#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "../../include/wsclient1.h"
#include "../../include/wsclientArm.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"
#include "../../include/cJSON.h"

#define SHARED_MEMORY_SIZE 1024

extern int idMqServerConnection;

int shmid;

typedef struct {
    long type;
    char payload[256];
} Message;

int onclose(wsclient *c) {
	fprintf(stderr, "onclose called: %d\n", c->sockfd);
	return 0;
}

int onerror(wsclient *c, wsclient_error *err) {
	fprintf(stderr, "onerror: (%d): %s\n", err->code, err->str);
	if(err->extra_code) {
		errno = err->extra_code;
		perror("recv");
	}
	return 0;
}

int onmessage(wsclient *c, wsclient_message *msg) {
    fprintf(stderr, "onmessage: (%llu): %s\n", msg->payload_len, msg->payload);

    char *sharedMemory = shmat(shmid, NULL, 0);
    if (sharedMemory == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    strncpy(sharedMemory, msg->payload, SHARED_MEMORY_SIZE);
    sharedMemory[SHARED_MEMORY_SIZE - 1] = '\0';

    if (shmdt(sharedMemory) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}

int onopen(wsclient *c) {
	fprintf(stderr, "onopen called: %d\n", c->sockfd);
	char me[100] = "{ \"token\":\"#^lxn6`S@Z9CGD\", \"action\":\"auth\", \"me\" : \"rpi\"}";
	libwsclient_send(c, me);
	return 0;
}

void initSharedMemory() {
    shmid = shmget(IPC_PRIVATE, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
}

void cleanupSharedMemory() {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}

void processMessageFromMQ(wsclient *c) {
    int mqServerConnection = openMQ(idMqServerConnection, 0);
    Message message;

    while (1) {
        if (msgrcv(mqServerConnection, &message, sizeof(message.payload), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

		cJSON *jsonObj = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonObj, "token", "#^lxn6`S@Z9CGD");
		char *jsonStr;

        switch (message.type) {
            case 1: // goal_red
				cJSON_AddStringToObject(jsonObj, "action", "goal_red");
				cJSON_AddStringToObject(jsonObj, "payload", message.payload);
				jsonStr = cJSON_PrintUnformatted(jsonObj);
				libwsclient_send(c, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonObj);
                break;
            case 2: // goal_blue
				cJSON_AddStringToObject(jsonObj, "action", "goal_blue");
				cJSON_AddStringToObject(jsonObj, "payload", message.payload);
				jsonStr = cJSON_PrintUnformatted(jsonObj);
				libwsclient_send(c, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonObj);
                break;
			case 3: // scan_card
				cJSON_AddStringToObject(jsonObj, "action", "scan_card");
				cJSON_AddStringToObject(jsonObj, "payload", message.payload);
				jsonStr = cJSON_PrintUnformatted(jsonObj);
				libwsclient_send(c, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonObj);
                break;

            default:
                fprintf(stderr, "Unknown message type: %ld\n", message.type);
                break;
        }
    }
}


void initWs(){
	initSharedMemory();

	wsclient *client = libwsclient_new("ws://localhost:8080");
	if(!client) {
		fprintf(stderr, "Unable to initialize new WS client.\n");
		exit(1);
	}
	//set callback functions for this client
	libwsclient_onopen(client, &onopen);
	libwsclient_onmessage(client, &onmessage);
	libwsclient_onerror(client, &onerror);
	libwsclient_onclose(client, &onclose);
	//bind helper UNIX socket to "test.sock"
	//One can then use netcat (nc) to send data to the websocket server end on behalf of the client, like so:
	// $> echo -n "some data that will be echoed by echo.websocket.org" | nc -U test.sock
	libwsclient_helper_socket(client, "test.sock");
	//starts run thread.
	libwsclient_run(client);

	processMessageFromMQ(client);

	//blocks until run thread for client is done.
	libwsclient_finish(client);

	cleanupSharedMemory();
}