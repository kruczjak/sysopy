//!TODO pytanie sie o siebie nie dziala

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#define LENGTH 10
#define NAME_LENGTH 50
#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}
        
typedef enum {
	REGISTER,
	SHOWLIST,
	CLIENTINFO,
	LOGOFF
} option_t;

typedef struct {
	char name[NAME_LENGTH];
	option_t option;
	char infoname[NAME_LENGTH];
} packet_t;

typedef struct {
	char names[LENGTH][NAME_LENGTH];
	int info;
} response_t;

typedef struct {
	int nprocs;
	int nusers;
	float lavg;
	int fmem;
	int bmem;
} info_t;

int server_fd;
int client_fd;
struct sockaddr_un sa, sc;
struct sockaddr_in si, sci;

pthread_mutex_t mutex;
pthread_cond_t cv;
int play = 1;

void * thread_responder(void * args)
{
	response_t response;
	while(1)
	{
		response.info = 0;
		pthread_mutex_lock(&mutex);

		while(!play)
			pthread_cond_wait(&cv, &mutex);

		pthread_mutex_unlock(&mutex);

		socklen_t fromlen = sizeof(sc);
		// Nie musi sprawdzac EAGAIN
		recvfrom(client_fd, (void *) &response, sizeof(response), MSG_DONTWAIT, (struct sockaddr *)&sc, &fromlen);
			//ERROR;

		if(response.info == 1)
		{
			printf("CLIENT: preparing info");
			info_t info;

			FILE *fp = popen("ps -e | wc -l", "r");
			fscanf(fp, "%d", &info.nprocs);
			pclose(fp);
			info.nprocs--;

			fp = popen("who | awk '{ print $1 }' | uniq | wc -l", "r");
			fscanf(fp, "%d", &info.nusers);
			pclose(fp);

			fp = fopen("/proc/loadavg", "r");
			fscanf(fp, "%f", &info.lavg);
			fclose(fp);

			fp = fopen("/proc/meminfo", "r");
			char * tmp = (char *) malloc(sizeof(char) * 50);
			fscanf(fp, "%s %d %s\n", tmp, &info.fmem, tmp);
			fscanf(fp, "%s %d %s\n", tmp, &info.bmem, tmp);
			free(tmp);
			fclose(fp);

			info.bmem -= info.fmem;
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");


			printf("CLIENT: sending info response");
			if(sendto(server_fd, (void *) &info, sizeof(info_t), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
				ERROR;
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");
		}
	}
}

void pause_thread()
{
	pthread_mutex_lock(&mutex);
	play = 0;
	pthread_mutex_unlock(&mutex);
}

void play_thread()
{
	pthread_mutex_lock(&mutex);
	play = 1;
	pthread_cond_signal(&cv);
	pthread_mutex_unlock(&mutex);
}

void getname(char * tab)
{
	FILE * fp = popen("hostname", "r");
	if(fp == NULL)
		ERROR;

	char tmp[NAME_LENGTH];
	char host[NAME_LENGTH];
	char domain[NAME_LENGTH];
	fscanf(fp, "%s", host);
	pclose(fp);

	fp = popen("domainname", "r");
	if(fp == NULL)
		ERROR;

	fscanf(fp, "%s", domain);
	pclose(fp);

	sprintf(tmp, "%s.%s", host, domain);

	strcpy(tab, tmp);
}

int main(int argc, char * argv[])
{
	if(argc < 2)
	{
		printf("usage : %s [unix | inet] [port ip | path]\n", argv[0]);
		exit(1);
	}

	unsigned int unix_flag = -1;
	char * path = NULL;
	unsigned int port = 0;
	char * ip = NULL;

	if(!strcmp(argv[1], "unix"))
	{
		unix_flag = 1;
		path = argv[2];
	}
	else
	if(!strcmp(argv[1], "inet"))
	{
		unix_flag = 0;
		port = atoi(argv[2]);
		if(argc < 4)
		{
			printf("Bad flags!\n");
			exit(1);
		}
		ip = argv[3];
	}

	if(unix_flag == -1)
	{
		printf("Bad flag!\n");
		exit(1);
	}

	int bytes;

	char CLIENT_SOCKET_NAME[NAME_LENGTH];
	getname(CLIENT_SOCKET_NAME);

	printf("CLIENT: opening socket");
	if(unix_flag == 1)
		server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	else
		server_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(server_fd < 0)
		ERROR;

	printf("\t\t\t\033[32m[ OK ]\033[0m\n");

	memset(&sa, 0, sizeof(sa));
	memset(&sc, 0, sizeof(sc));
	memset(&si, 0, sizeof(si));
	memset(&sci, 0, sizeof(sci));

	si.sin_family = AF_INET;
	si.sin_port = htons(port);

	if(!unix_flag)
		si.sin_addr.s_addr = inet_addr(ip);

	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, path);

	sc.sun_family = AF_UNIX;
	strcpy(sc.sun_path, CLIENT_SOCKET_NAME);

	printf("CLIENT: opening client socket");
	if(unix_flag == 1)
	{
		client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
		if(client_fd < 0)
			ERROR;
	} else
	{
		client_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(client_fd < 0)
			ERROR;
	}
	printf("\t\t\033[32m[ OK ]\033[0m\n");

	printf("CLIENT: binding client socket");
	if(unix_flag == 1)
		if(bind(client_fd, (struct sockaddr *)&sc, sizeof(sc)) < 0)
			ERROR;
	printf("\t\t\033[32m[ OK ]\033[0m\n");

	printf("CLIENT: creating register packet");
	packet_t * packet;
	packet = (packet_t *) malloc(sizeof(packet_t));
	if(packet == NULL)
		ERROR;
	printf("\t\033[32m[ OK ]\033[0m\n");

	packet->option = REGISTER;
	getname(packet->name);

	printf("CLIENT: registering");
	if(unix_flag == 1)
	{
		bytes = sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&sa, sizeof(sa));
		if(bytes < 0)
			ERROR;
	} else
	{
		bytes = sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&si, sizeof(si));
		if(bytes < 0)
			ERROR;
	}
	printf("\t\t\t\033[32m[ OK ]\033[0m\n");

	printf("CLIENT: creating info thread");
	pthread_t responder;
	if(pthread_create(&responder, NULL, thread_responder, NULL) < 0)
		ERROR;
	printf("\t\t\033[32m[ OK ]\033[0m\n");

	while(1)
	{
		printf("Choose option\n");
		printf("1) Show client list\n");
		printf("2) Show client info\n");
		printf("3) End session\n");

		char opt;
		scanf("%c", &opt);

		switch(opt)
		{
			case '2':
				packet->option = CLIENTINFO;
				printf("ID: ");
				scanf("%s", packet->infoname);

				printf("CLIENT: sending info request");
				if(unix_flag == 1)
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
						ERROR;
				} else
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&si, sizeof(si)) < 0)
						ERROR;
				}
				printf("\t\t\t\033[32m[ OK ]\033[0m\n");

				info_t info;

				printf("CLIENT: waiting for response");
				socklen_t fromlen = sizeof(sc);
				socklen_t fromleni = sizeof(si);
				if(unix_flag == 1)
				{
					if(recvfrom(client_fd, (void *) &info, sizeof(info), 0, (struct sockaddr *)&sc, &fromlen) < 0)
						ERROR;
				} else
				{
					if(recvfrom(client_fd, (void *) &info, sizeof(info), 0, (struct sockaddr *)&sci, &fromleni) < 0)
						ERROR;
				}
				printf("\t\t\t\033[32m[ OK ]\033[0m\n");

				printf("NPROCS  =%d\n", info.nprocs);
				printf("NUSERS  =%d\n", info.nusers);
				printf("LOAD AVG=%f\n", info.lavg);
				printf("FREE MEM=%d\n", info.fmem);
				printf("BUSY MEM=%d\n", info.bmem);

				break;
			case '3':
				packet->option = LOGOFF;

				printf("CLIENT: sending logoff request");
				if(unix_flag == 1)
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
						ERROR;
				} else
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&si, sizeof(si)) < 0)
						ERROR;
				}
				printf("\t\t\t\033[32m[ OK ]\033[0m\n");

				if(unix_flag == 1)
					unlink(CLIENT_SOCKET_NAME);
				exit(1);

				break;
			case '1':
				packet->option = SHOWLIST;
				response_t response;

				printf("CLIENT: sending request");
				if(unix_flag == 1)
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
						ERROR;
				} else
				{
					if(sendto(server_fd, packet, sizeof(packet_t), 0, (struct sockaddr *)&si, sizeof(si)) < 0)
						ERROR;
				}
				printf("\t\t\t\033[32m[ OK ]\033[0m\n");

				pause_thread();

				printf("CLIENT: waiting for response");
				fromlen = sizeof(sc);
				if(unix_flag == 1)
				{
					if(recvfrom(client_fd, (void *) &response, sizeof(response), 0, (struct sockaddr *)&sc, &fromlen) < 0)
						ERROR;
				} else
				{
					if(recvfrom(client_fd, (void *) &response, sizeof(response), 0, (struct sockaddr *)&sci, &fromleni) < 0)
						ERROR;
				}
				printf("\t\t\t\033[32m[ OK ]\033[0m\n");

				play_thread();

				int i = 0;
				while(i < LENGTH && strlen(response.names[i]) > 0)
						printf("%s\n", response.names[i++]);

				break;

			default:
				printf("Bad option\n");
				continue;
		}

	}

	return EXIT_SUCCESS;
}
