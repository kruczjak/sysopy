#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
//server

#define FILE_PERM S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP
#define QUEUE_PERM S_IRUSR | S_IWUSR | S_IWGRP
#define SERVER_KEY_FILE "/tmp/server.key"
#define CLIENT_MSG_TYPE 1
#define SERVER_MSG_TYPE 2
#define MAX_CLIENTS 100
#define MSG_LENGTH 50
#define NAME_LENGTH 50
#define ERROR { int error_code = errno; \
				fprintf(stderr, "blad: %s\n", strerror(error_code)); \
				exit(error_code);}

struct c_msg
{
	long mtype;
	char name[NAME_LENGTH];
	char text[MSG_LENGTH];
	int queue_id;
	time_t time;
};

struct s_msg
{
	long mtype;
	char name[NAME_LENGTH];
	char text[MSG_LENGTH];
};
////////////
volatile int queue_id;

void clean()
{
	printf("SERVER: exiting...\n");
	if(msgctl(queue_id, IPC_RMID, NULL) < 0)
		ERROR;
	if(unlink(SERVER_KEY_FILE) < 0)
		ERROR;
}

int main(int argc, char** argv)
{

	int clients [MAX_CLIENTS];
	for (int i = 0; i<MAX_CLIENTS; i++) clients[i] = -1;
	int clients_pointer=0;
	key_t queue_key;		// klucz kolejki
	struct s_msg s_message;
	struct c_msg c_message;

	printf("\nSERVER: generating fifo file");
	if (close(open(SERVER_KEY_FILE, O_WRONLY | O_CREAT, FILE_PERM)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

	printf("SERVER: generating fifo key");
	if ((queue_key = ftok(SERVER_KEY_FILE, 's')) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

 	printf("SERVER: generating fifo id");
	if ((queue_id = msgget(queue_key, IPC_CREAT | QUEUE_PERM)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");
	atexit(clean);

	srand(time(NULL));
	int create = 0;

	while (1)
	{
		printf("SERVER: waiting for message ...");
		if (msgrcv(queue_id, &c_message, sizeof(struct c_msg), CLIENT_MSG_TYPE, 0) < 0)
			ERROR;
		printf("\t\t[ OK ]\n");

		printf("SERVER: got client message:\n");
		printf("===========================\n");
		printf("name    : %s\n", c_message.name);
		printf("mesasge : %s\n", c_message.text);
		printf("queue id: %d\n", c_message.queue_id);
		printf("time: %d\n", (int)c_message.time);

		if (strcmp(c_message.text,"!<con>")==0) {
			create = 1;
			sprintf(c_message.text, "connected");
			for (int i=0; i<MAX_CLIENTS; i++) {
				if (clients[i] == c_message.queue_id) {
					create = 0;
					break;
				}
			}
		} else if (strcmp(c_message.text,"exit")==0) {
			for (int i=0; i<MAX_CLIENTS; i++)
				if (clients[i] == c_message.queue_id) {
					clients[i] = -1;
					break;
				}
				sprintf(c_message.text, "exiting");
		}
		printf("SERVER: sending . . .\n");

		if (msgctl(queue_id, IPC_STAT, &data) < 0)
			ERROR;

		s_message.mtype = SERVER_MSG_TYPE;

		char buffer [26];
		struct tm * timeinfo;
		timeinfo = localtime (&c_message.time);
		strftime(buffer, 26, "%H:%M:%S", tm_info);
		strcpy(s_message.name, argv[0]);
		sprintf(s_message.text,"[%s] %s: %s", buffer, c_message.name, c_message.text);

		for (int i=0;i<MAX_CLIENTS;i++)
			if (clients[i]!=-1) {
				printf("Sending to %d\n", clients[i]);
				if(msgsnd(clients[i], &s_message, sizeof(struct s_msg), 0) < 0)
					ERROR;
				}

		if (create == 1) {
			create = 0;
			clients[clients_pointer] = c_message.queue_id;
			clients_pointer++;
		}

		printf("\t\t\t[ OK ]\n");
	}


	return EXIT_SUCCESS;
}
