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
#include <mqueue.h>
#include <string.h>
//server

#define SERVER_KEY_FILE "/server.key"
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

void prepend(char* s, const char* t)
{
    size_t len = strlen(t);
    size_t i;

    memmove(s + len, s, strlen(s) + 1);

    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
}

void clean()
{
	printf("SERVER: exiting...\n");
	if(mq_unlink(SERVER_KEY_FILE) < 0)
		ERROR;
}

int main(int argc, char** argv)
{

	char clients [MAX_CLIENTS][NAME_LENGTH];
	for (int i=0; i<MAX_CLIENTS;i++) clients[i][0] = '\0';
	int clients_pointer=0;
	struct s_msg s_message;
	struct c_msg c_message;

	printf("\nSERVER: generating fifo file");
	struct mq_attr ma;
	ma.mq_flags = 0;                // blocking read/write
  ma.mq_maxmsg = 16;              // maximum number of messages allowed in queue
  ma.mq_msgsize = sizeof(struct c_msg);    // messages are contents of an int
  ma.mq_curmsgs = 0;              // number of messages currently in queue
	queue_id = mq_open(SERVER_KEY_FILE, O_RDWR | O_CREAT, 0644, &ma);
	if (queue_id<0)
		ERROR;
	printf("\t\t[ OK ]\n");
	ma.mq_msgsize = sizeof(struct s_msg); //set for sending

	atexit(clean);

	srand(time(NULL));
	int create = 0;

	while (1)
	{
		printf("SERVER: waiting for message ...");
		if (mq_receive(queue_id, (char *)&c_message, sizeof(struct c_msg), NULL) < 0)
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
			sprintf(c_message.text, "connected\n");
			for (int i=0; i<MAX_CLIENTS; i++) {
				if (strcmp(clients[i], (char *)c_message.name)==0) {
					create = 0;
					break;
				}
			}
		} else if (strcmp(c_message.text,"exit\n")==0) {
			for (int i=0; i<MAX_CLIENTS; i++)
				if (strcmp(clients[i], c_message.name)==0) {
					clients[i][0] = '\0';
					break;
				}
				sprintf(c_message.text, "exiting\n");
		}
		printf("SERVER: sending . . .\n");

		s_message.mtype = SERVER_MSG_TYPE;

		char buffer [26];
		struct tm * timeinfo;
		timeinfo = localtime (&c_message.time);
		strftime(buffer, 26, "%H:%M:%S", timeinfo);
		strcpy(s_message.name, argv[0]);
		sprintf(s_message.text,"[%s] %s: %s", buffer, c_message.name, c_message.text);

		for (int i=0;i<MAX_CLIENTS;i++)
			if (clients[i][0]!='\0') {
				printf("Sending to %s\n", clients[i]);

				char* new_name;
				new_name = malloc(strlen(clients[i])+1+4); /* make space for the new string (should check the return value ...) */
				strcpy(new_name, clients[i]);
				prepend(new_name, "/");

				int queue_id = mq_open(new_name, O_WRONLY, 0644, &ma);
				if (queue_id<0)
					ERROR;
				if(mq_send(queue_id, (const char *) &s_message, sizeof(struct s_msg), 0) < 0)
					ERROR;
				}

		if (create == 1) {
			create = 0;
			sprintf(clients[clients_pointer], c_message.name);
			clients_pointer++;
		}

		printf("\t\t\t[ OK ]\n");
	}


	return EXIT_SUCCESS;
}
