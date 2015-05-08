#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <time.h>
#include <locale.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
//client

#define SERVER_KEY_FILE "/server.key"
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


int main(int argc, char * argv[])
{
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

	char* new_name;
	new_name = malloc(strlen(name)+1+4); /* make space for the new string (should check the return value ...) */
	strcpy(new_name, name);
	prepend(new_name, "/");


	printf("CLIENT: obtaining server fifo id");
	struct mq_attr ma;
	ma.mq_flags = 0;                // blocking read/write
	ma.mq_maxmsg = 16;              // maximum number of messages allowed in queue
	ma.mq_msgsize = sizeof(struct c_msg);    // messages are contents of an int
	ma.mq_curmsgs = 0;              // number of messages currently in queue
	server_id = mq_open(SERVER_KEY_FILE, O_WRONLY, 0644, &ma);
	printf("\t[ OK ]\n");


	printf("CLIENT: opening mqueue");
	ma.mq_msgsize = sizeof(struct s_msg);    // messages are contents of an int
	queue_id = mq_open(new_name, O_RDONLY | O_CREAT, 0644, &ma);
	printf("\n%s\n", new_name);
	printf("\t\t[ OK ]\n");

	srand(time(NULL));

//sending first message
	c_message.queue_id = queue_id;
	memcpy(c_message.name, name, strlen(name) + 1);
	c_message.mtype = CLIENT_MSG_TYPE;
	memcpy(c_message.text, "!<con>", strlen("!<con>") + 1);
	time(&c_message.time);

	if (mq_send(server_id, (const char *)&c_message, sizeof(struct c_msg), 0) < 0)
		ERROR;
//end

	pid_t writer = fork();
	if (writer < 0 ) {
	  ERROR;
	} else if (writer == 0) {
		//fork
		while(1) {
			if (mq_receive(queue_id, (char *) &s_message, sizeof(struct s_msg), NULL) < 0)
				ERROR;
			printf("%s", s_message.text);
		}
	} else {
		char msg [MAX_MSG_LENGTH];
		while(1) {

			fgets(msg, MAX_MSG_LENGTH, stdin);

			c_message.queue_id = queue_id;
			memcpy(c_message.name, name, strlen(name) + 1);
			c_message.mtype = CLIENT_MSG_TYPE;
			memcpy(c_message.text, msg, strlen(msg) + 1);
			time(&c_message.time);

			if (mq_send(server_id, (const char *)&c_message, sizeof(struct c_msg), 0) < 0)
				ERROR;

			if (strcmp(msg,"exit\n")==0) exit(0);
		}
	}

	return EXIT_SUCCESS;
}
