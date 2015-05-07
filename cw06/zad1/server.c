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

	// int sleep_time;
	key_t queue_key;		// klucz kolejki
	struct s_msg s_message;
	struct c_msg c_message;
	struct msqid_ds data;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s [sleep_time]\n", argv[0]);
		exit(1);
	}

	// moze lepiej strtol ?
	// sleep_time = atoi(argv[1]);
	// if(sleep_time < 0)
	// {
	// 	fprintf(stderr, "error: sleep_time < 0\n");
	// 	exit(1);
	// }

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

		// sleep(rand() % sleep_time);

		printf("SERVER: sending . . .");

		if (msgctl(queue_id, IPC_STAT, &data) < 0)
			ERROR;

		s_message.mtype = SERVER_MSG_TYPE;

		s_message.msqid.msg_qnum = data.msg_qnum;
		s_message.msqid.msg_cbytes = data.msg_cbytes;
		s_message.msqid.msg_qbytes = data.msg_qbytes;
		s_message.msqid.msg_lspid = data.msg_lspid;
		s_message.msqid.msg_lrpid = data.msg_lrpid;
		s_message.msqid.msg_stime = data.msg_stime;
		s_message.msqid.msg_rtime = data.msg_rtime;

		strcpy(s_message.name, argv[0]);
		memcpy(s_message.text, c_message.text, MSG_LENGTH+1);

		if(msgsnd(c_message.queue_id, &s_message, sizeof(struct s_msg), 0) < 0)
			ERROR;

		printf("\t\t\t[ OK ]\n");
	}


	return EXIT_SUCCESS;
}
