#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>

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

int main(int argc, char ** argv) {

  if (argc < 3) {
    printf("Bad arguments!: %s <port> <path>", argv[0]);
    exit(1);
  }

  int port = strtol(argv[1], NULL, 10);
  char * path = argv[2];

  unsigned int unix_flag = 1;

  struct sockaddr_un sa,sc;
  struct sockaddr_in si, sci;
  int server_fd, server_inet_fd;

  printf("Creating socket\n");
  if ((server_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) ERROR;
  printf("Creating inet socket\n");
  if ((server_inet_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) ERROR;

  printf("Sockets created\n");

  memset(&sa, 0, sizeof(sa));
  memset(&sc, 0, sizeof(sc));
  memset(&si, 0, sizeof(si));
  memset(&sci, 0, sizeof(sci));

  si.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &(si.sin_addr.s_addr));
  si.sin_port = htons(port);

  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, path);
  unlink(path);

  sc.sun_family = AF_UNIX;

  socklen_t fromlen = sizeof(sa);
  socklen_t fromleni = sizeof(si);

  if(bind(server_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0 ) ERROR;
  if(bind(server_inet_fd, (struct sockaddr *)&si, sizeof(si)) < 0) ERROR;

  char names[LENGTH][NAME_LENGTH];

	int i, j;
	for(i = 0 ; i < LENGTH ; i++)
		for(j = 0 ; j < NAME_LENGTH ; j++)
			names[i][j] = (char) 0;

	packet_t * buffer;
  if ((buffer = (packet_t *) malloc(sizeof(packet_t))) == NULL) ERROR;


  while (1) {
    printf("SERVER: receiving\n");
		while(1)
		{
			int recsize = recvfrom(server_fd, (void *)buffer, sizeof(packet_t), MSG_DONTWAIT, (struct sockaddr *)&sa, &fromlen);
			if(recsize > 0)
			{
				unix_flag = 1;
				break;
			}
			recsize = recvfrom(server_inet_fd, (void *)buffer, sizeof(packet_t), MSG_DONTWAIT, (struct sockaddr *)&si, &fromleni);
			if(recsize > 0)
			{
				unix_flag = 0;
				break;
			}
		}
		printf("\t\t\t\033[32m[ OK ]\033[0m\n");
		printf("SERVER: protocol UNIX=%d\n", unix_flag);

		if(buffer->option == REGISTER)
		{
			printf("SERVER: registering");
			register_id(buffer->name, names);
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");
		}
		else
		if(buffer->option == LOGOFF)
		{
			printf("SERVER: Unregistering");
			unregister_id(buffer->name, names);
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");
		} else
		if(buffer->option == SHOWLIST)
		{
			strcpy(sc.sun_path, buffer->name);

			printf("SERVER: opening client socket");
			int client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
			int client_inet_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if(client_fd < 0)
				ERROR;
			if(client_inet_fd < 0)
				ERROR;


			printf("\t\t\033[32m[ OK ]\033[0m\n");

			printf("SERVER: responding");
			response_t response;
			int i;
			for(i = 0 ; i < LENGTH ; i++)
				strcpy(response.names[i], names[i]);

			if(unix_flag == 1)
			{
				if(sendto(client_fd, &response, sizeof(response), 0, (struct sockaddr *)&sc, sizeof(sc)) < 0)
					ERROR;
			}
			else
			{
				if(sendto(client_inet_fd, &response, sizeof(response), 0, (struct sockaddr *)&si, sizeof(si)) < 0)
					ERROR;
			}

			printf("\t\t\t\033[32m[ OK ]\033[0m\n");
		} else
		if(buffer->option == CLIENTINFO)
		{
			char tmp[25];
			sprintf(tmp, "%s", buffer->infoname);

			strcpy(sc.sun_path, tmp);

			printf("SERVER: opening client socket");
			int client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
			int client_inet_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if(client_fd < 0)
				ERROR;
			if(client_inet_fd < 0)
				ERROR;

			memcpy(&sci, &si, sizeof(si));
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");

			response_t r;
			r.info = 1;

			printf("SERVER: sending info request");
			if(unix_flag == 1)
			{
				if(sendto(client_fd, &r, sizeof(r), 0, (struct sockaddr *)&sc, sizeof(sc)) < 0)
					ERROR;
			}
			else
			{
				if(sendto(client_inet_fd, &r, sizeof(r), 0, (struct sockaddr *)&si, sizeof(si)) < 0)
					ERROR;
			}
			printf("\t\t\t\033[32m[ OK ]\033[0m\n");

			info_t info;
			socklen_t fromlen = sizeof(sa);

			printf("SERVER: waiting for response");
			int flag = 1, tries = 0;
			while(flag)
			{
				int recsize;
				if(unix_flag == 1)
					recsize = recvfrom(server_fd, (void *)&info, sizeof(info_t), 0, (struct sockaddr *)&sa, &fromlen);
				else
					recsize = recvfrom(server_inet_fd, (void *)&info, sizeof(info_t), 0, (struct sockaddr *)&si, &fromleni);
				if(recsize >= 0)
					flag = 0;
				else
				if(tries++ > 100000)
					break;
			}

			if(flag == 1)
			{
				printf("SERVER: client not responding");
				memset(&info, 0, sizeof(info));
			}

			printf("\t\t\t\033[32m[ OK ]\033[0m\n");

			sprintf(tmp, "%s", buffer->name);

			strcpy(sc.sun_path, tmp);

			printf("SERVER: opening client socket");

			if(unix_flag == 1)
			{
				client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
				if(client_fd < 0)
					ERROR;
			}
			else
			{
				client_inet_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if(client_inet_fd < 0)
					ERROR;
			}

			printf("\t\t\t\033[32m[ OK ]\033[0m\n");

			printf("SERVER: sending info");
			if(unix_flag == 1)
			{
				if(sendto(client_fd, &info, sizeof(info), 0, (struct sockaddr *)&sc, sizeof(sc)) < 0)
					ERROR;
			}
			else
			{
				if(sendto(client_inet_fd, &info, sizeof(info), 0, (struct sockaddr *)&sci, sizeof(sci)) < 0)
					ERROR;
			}
			printf("\t\t\t\t\033[32m[ OK ]\033[0m\n");
		}
  }

  return 0;
}
