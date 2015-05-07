#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <time.h>
#include <locale.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
//client

#define FILE_PERM S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP
#define QUEUE_PERM S_IRUSR | S_IWUSR | S_IWGRP
#define SERVER_KEY_FILE "/tmp/server.key"
#define CLIENT_QUEUE_KEY 'c'
#define CLIENT_MSG_TYPE 1
#define SERVER_MSG_TYPE 2
#define MAX_MSG_LENGTH 50
#define NAME_LENGTH 50
#define ERROR { int error_code = errno; \
				fprintf(stderr, "blad: %s\n", strerror(error_code)); \
				exit(error_code);}

struct c_msg
{
	long mtype;
	char name[NAME_LENGTH];
	char text[MAX_MSG_LENGTH];
	int queue_id;
	time_t time;
};

struct s_msg
{
	long mtype;
	char name[NAME_LENGTH];
	char text[MAX_MSG_LENGTH];
};
/////////////////

int main(int argc, char * argv[])
{
	key_t server_key;
	key_t queue_key;
	int server_id;
	int queue_id;

	struct s_msg s_message;
	struct c_msg c_message;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s [name]\n", argv[0]);
		exit(1);
	}

	char * name = argv[1];
	char CLIENT_KEY_FILE [NAME_LENGTH];
	sprintf(CLIENT_KEY_FILE, "/tmp/%s.key", name);

	printf("CLIENT: obtaining server fifo key");
    if ((server_key = ftok(SERVER_KEY_FILE, 's')) < 0)
		ERROR;
	printf("\t[ OK ]\n");

	printf("CLIENT: obtaining server fifo id");
    if ((server_id = msgget(server_key, 0)) < 0)
		ERROR;
	printf("\t[ OK ]\n");


	printf("CLIENT: generating own fifo file");
	if (close(open(CLIENT_KEY_FILE, O_WRONLY | O_CREAT, FILE_PERM)) < 0)
		ERROR;
	printf("\t[ OK ]\n");

	printf("CLIENT: generating own fifo key");
	if ((queue_key = ftok(CLIENT_KEY_FILE, CLIENT_QUEUE_KEY)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

 	printf("CLIENT: generating own fifo id");
	if ((queue_id = msgget(queue_key, IPC_CREAT | QUEUE_PERM)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

	srand(time(NULL));

//sending first message
	c_message.queue_id = queue_id;
	memcpy(c_message.name, name, strlen(name) + 1);
	c_message.mtype = CLIENT_MSG_TYPE;
	memcpy(c_message.text, "!<con>", strlen("!<con>") + 1);
	time(&c_message.time);

	if (msgsnd(server_id, &c_message, sizeof(struct c_msg), 0) < 0)
		ERROR;
//end

	pid_t writer = fork();
	if (writer < 0 ) {
	  ERROR;
	} else if (writer == 0) {
		//fork
		while(1) {
			if (msgrcv(queue_id, &s_message, sizeof(struct s_msg), SERVER_MSG_TYPE, 0) < 0)
				ERROR;
			printf("%s", s_message.text);
		}
	} else {
		char msg [MAX_MSG_LENGTH];
		while(1) {

			fgets (msg, MAX_MSG_LENGTH, stdin);

			c_message.queue_id = queue_id;
			memcpy(c_message.name, name, strlen(name) + 1);
			c_message.mtype = CLIENT_MSG_TYPE;
			memcpy(c_message.text, msg, strlen(msg) + 1);
			time(&c_message.time);

			if (msgsnd(server_id, &c_message, sizeof(struct c_msg), 0) < 0)
				ERROR;

			if (strcmp(msg,"exit\n")==0) exit(0);
		}
	}

	return EXIT_SUCCESS;
}
