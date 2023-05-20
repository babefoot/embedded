#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


#include "../../include/wsclient1.h"
#include "../../include/wsclientArm.h"
#include "../../include/serverConnection.h"
#include "../../include/MQ.h"


extern int idMqServerConnection;
int mqServerConnectionRecept;

int shmidServerConnection;
char *sharedMemoryServerConnection;

wsclient *client;

int compteurPlayers = 0;

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

cJSON *parseSharedMemory(char *sharedMemory){

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

	//get the payload object and store it in char *payloadString
	char *payloadString = cJSON_Print(payload);

	//afficher l'adresse de la shared memory
	fprintf(stderr, "shared memory address : %p\n", sharedMemoryServerConnection);
	strcpy(sharedMemoryServerConnection, payloadString);

	message msgInitServ;
    msgInitServ.mtype = 31;

	//make the shared memory empty
    strcpy(msgInitServ.payload, compteurPlayers == 4 ? "full" : "nfull");
    sendToMQ(mqServerConnectionRecept, &msgInitServ);
    return 0;
}

int onopen(wsclient *c) {
	fprintf(stderr, "onopen called: %d\n", c->sockfd);
	char me[100] = "{ \"token\":\"#^lxn6`S@Z9CGD\", \"action\":\"auth\", \"me\" : \"rpi\"}";
	libwsclient_send(c, me);
	return 0;
}



void cleanupSharedMemory() {
    if (shmctl(shmidServerConnection, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		exit(1);
	}
}

void processMessageFromMQ() {
    message messageRcv;

    while (1) {
		receiveFromMQ(mqServerConnectionRecept, &messageRcv, -60);
		fprintf(stderr, "mtype received : <%ld>\n", messageRcv.mtype);
		fprintf(stderr, "Payload received : <%s>\n", messageRcv.payload);

		cJSON *jsonAction = cJSON_CreateObject();
		cJSON *jsonPayload = cJSON_CreateObject();

		char *jsonStr;
		cJSON_AddStringToObject(jsonAction, "token", TOKEN);
        switch (messageRcv.mtype) {
            case 51: // goal
				printf("goal case\n");
				cJSON_AddStringToObject(jsonAction, "action", "goal");
                cJSON *jsonShared = cJSON_Parse(sharedMemoryServerConnection);


				cJSON_AddStringToObject(jsonPayload, "id_game", cJSON_GetObjectItem(jsonShared, "id")->valuestring);
				cJSON_AddStringToObject(jsonPayload, "team", messageRcv.payload);
				cJSON_AddArrayToObject(jsonPayload, "scorers");
				cJSON *players = cJSON_GetObjectItem(jsonShared, "players");
				cJSON *player = NULL;
				cJSON_ArrayForEach(player, players) {
					if (strcmp(cJSON_GetObjectItem(player, "team")->valuestring, messageRcv.payload) == 0) {
						printf("player : %s\n", cJSON_GetObjectItem(player, "id")->valuestring);
						cJSON_AddItemToArray(cJSON_GetObjectItem(jsonPayload, "scorers"), cJSON_CreateString(cJSON_GetObjectItem(player, "id")->valuestring));
						break;
					}
				}
				cJSON_AddItemToObject(jsonAction, "payload", jsonPayload);

				jsonStr = cJSON_PrintUnformatted(jsonAction);
				printf("jsonPaylod : %s\n", jsonStr);
				libwsclient_send(client, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonAction);
				printf("But envoyé au serveur \n");
                break;
			case 52: // scan_card
				cJSON_AddStringToObject(jsonAction, "action", "scan_card");
				cJSON_AddStringToObject(jsonPayload, "id_card", messageRcv.payload);
				cJSON_AddItemToObject(jsonAction, "payload", jsonPayload);
				jsonStr = cJSON_PrintUnformatted(jsonAction);
				libwsclient_send(client, jsonStr);
				cJSON_free(jsonStr);
				cJSON_Delete(jsonAction);
				compteurPlayers++;
                break;
			case 31:
				printf("SC : je suis un relais vers OR\n");
				message relais;
				relais.mtype = 31;
				strcpy(relais.payload, compteurPlayers == 4 ? "full" : "nfull");
				sendToMQ(mqServerConnectionRecept, &relais);
				return 0;
                break;
            default:
                fprintf(stderr, "Unknown message type: %ld\n", messageRcv.mtype);
                break;
        }
    }
}


void initWs(){
    mqServerConnectionRecept = openMQ(idMqServerConnection, 1);
	client = libwsclient_new("ws://192.168.1.80:8080");
	if(!client) {
		fprintf(stderr, "Unable to initialize new WS client.\n");
		exit(1);
	}

	sharedMemoryServerConnection = shmat(shmidServerConnection, NULL, 0);
    if (sharedMemoryServerConnection == (void*)-1) {
        perror("shmat");
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


	//blocks until run thread for client is done.

}

void serverConnection(int shmid){
	shmidServerConnection = shmid;
	initWs();
	processMessageFromMQ();
	printf("SC : on est sortit du process\n");
	cleanupSharedMemory();
	libwsclient_finish(client);
}