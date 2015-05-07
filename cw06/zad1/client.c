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
#define MSG_LENGTH 25
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
};

struct s_msg
{
	long mtype;
	char name[NAME_LENGTH];
	char text[MSG_LENGTH];
	struct msqid_ds msqid;
};
/////////////////

int main(int argc, char * argv[])
{
	key_t server_key;
	key_t queue_key;
	int server_id;
	int queue_id;
	int nr;

	struct s_msg s_message;
	struct c_msg c_message;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s [name]\n", argv[0]);
		exit(1);
	}

	char * name = argv[1];

	printf("CLIENT: obtaining server fifo key");
    if ((server_key = ftok(SERVER_KEY_FILE, 's')) < 0)
		ERROR;
	printf("\t[ OK ]\n");

	printf("CLIENT: obtaining server fifo id");
    if ((server_id = msgget(server_key, 0)) < 0)
		ERROR;
	printf("\t[ OK ]\n");


	printf("CLIENT: generating own fifo file");
	if (close(open("c", O_WRONLY | O_CREAT, FILE_PERM)) < 0)
		ERROR;
	printf("\t[ OK ]\n");

	printf("CLIENT: generating own fifo key");
	if ((queue_key = ftok("c", CLIENT_QUEUE_KEY)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

 	printf("CLIENT: generating own fifo id");
	if ((queue_id = msgget(queue_key, IPC_CREAT | QUEUE_PERM)) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

	srand(time(NULL));

	while(1)
	{
		printf(">> ");
		if (scanf("%d", &nr) < 0)
		{
			fprintf(stderr, "Integer expected\n");
			exit(1);
		}

		int i;
		for(i = 0; i < nr; i++)
		{
			printf("CLIENT: generating message");
			c_message.queue_id = queue_id;
			memcpy(c_message.name, name, strlen(name) + 1);

			c_message.mtype = CLIENT_MSG_TYPE;

			int j;
			for(j = 0 ; j < MSG_LENGTH ; j++)
				c_message.text[j] = rand() % 26 + 'a';
			printf("\t\t[ OK ]\n");

			printf("CLIENT: sending to server");
			if (msgsnd(server_id, &c_message, sizeof(struct c_msg), 0) < 0)
				ERROR;
			printf("\t\t[ OK ]\n");

			printf("CLIENT: waiting for response");
			if (msgrcv(queue_id, &s_message, sizeof(struct s_msg), SERVER_MSG_TYPE, 0) < 0)
				ERROR;
			printf("\t\t[ OK ]\n");

			printf("CLIENT: got server response:\n");
			printf("============================\n");
			printf("name   : %s\n", s_message.name);
			printf("text   : %s\n", s_message.text);
			printf("qnum   : %ld\n", s_message.msqid.msg_qnum);
			printf("cbytes : %ld\n", s_message.msqid.msg_cbytes);
			printf("qbytes : %ld\n", s_message.msqid.msg_qbytes);
			printf("lspid  : %d\n", s_message.msqid.msg_lspid);
			printf("lrpid  : %d\n", s_message.msqid.msg_lrpid);
			printf("stime  : %ld\n", s_message.msqid.msg_stime);
			printf("rtime  : %ld\n", s_message.msqid.msg_rtime);
		}
	}

	return EXIT_SUCCESS;
}
