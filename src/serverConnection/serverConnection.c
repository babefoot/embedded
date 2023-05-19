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

cJSON *parseSharedMemory(){
	char *sharedMemory = shmat(shmid, NULL, 0);
	if (sharedMemory == (void*)-1) {
		perror("shmat");
		exit(1);
	}

	cJSON *jsonObj = cJSON_Parse(sharedMemory);
	if (jsonObj == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		exit(1);
	}

	if (shmdt(sharedMemory) == -1) {
		perror("shmdt");
		exit(1);
	}

	return jsonObj;
}

int onmessage(wsclient *c, wsclient_message *msg) {
    fprintf(stderr, "onmessage: (%llu): %s\n", msg->payload_len, msg->payload);

    char *sharedMemory = shmat(shmid, NULL, 0);
    if (sharedMemory == (void*)-1) {
        perror("shmat");
        exit(1);
    }
	
	//the msh is a string in json format with a payload object, store it in shared memory

	cJSON *jsonObj = cJSON_Parse(msg->payload);
	if (jsonObj == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		exit(1);
	}
	cJSON *payload = cJSON_GetObjectItemCaseSensitive(jsonObj, "payload");
	char *payloadString = cJSON_Print(payload);
	strcpy(sharedMemory, payloadString);


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
    int mqServerConnection = openMQ(idMqServerConnection, 1);
    Message message;

    while (1) {
		receiveFromMQ(mqServerConnection, &message, 0);
		fprintf(stderr, "mtype received : <%ld>\n", message.type);
		fprintf(stderr, "Payload received : <%s>\n", message.payload);

		cJSON *jsonAction = cJSON_CreateObject();
		cJSON *jsonPayload = cJSON_CreateObject();

		char *jsonStr;
		cJSON_AddStringToObject(jsonAction, "token", TOKEN);
        switch (message.type) {
            case 2: // goal
				printf("goal case\n");
				cJSON_AddStringToObject(jsonAction, "action", "goal");
				cJSON* jsonShared = parseSharedMemory();

				cJSON_AddStringToObject(jsonPayload, "id_game", cJSON_GetObjectItem(jsonShared, "id")->valuestring);
				cJSON_AddStringToObject(jsonPayload, "team", message.payload);
				cJSON_AddArrayToObject(jsonPayload, "scorers");
				cJSON *players = cJSON_GetObjectItem(jsonShared, "players");
				cJSON *player = NULL;
				cJSON_ArrayForEach(player, players) {
					if (strcmp(cJSON_GetObjectItem(player, "team")->valuestring, message.payload) == 0) {
						printf("player : %s\n", cJSON_GetObjectItem(player, "id")->valuestring);
						cJSON_AddItemToArray(cJSON_GetObjectItem(jsonPayload, "scorers"), cJSON_CreateString(cJSON_GetObjectItem(player, "id")->valuestring));
						break;
					}
				}
				cJSON_AddItemToObject(jsonAction, "payload", jsonPayload);

				jsonStr = cJSON_PrintUnformatted(jsonAction);
				printf("jsonPaylod : %s\n", jsonStr);
				libwsclient_send(c, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonAction);
                break;
			case 3: // scan_card
				cJSON_AddStringToObject(jsonAction, "action", "scan_card");
				cJSON_AddStringToObject(jsonAction, "payload", message.payload);
				jsonStr = cJSON_PrintUnformatted(jsonAction);
				libwsclient_send(c, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonAction);
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