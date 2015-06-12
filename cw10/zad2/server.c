#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/un.h>

#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

int sockets[10];
int socket_net, socket_unix, number = 0;
char *file;

typedef struct msg {
    char from[32];
    char content[480];
} msg;

void exit_handler() {
    exit(0);
}

void exitme() {
    int i;
    for(i = 0; i < number; i++) {
        shutdown(sockets[i], SHUT_RDWR);
        if (close(sockets[i]) < 0) ERROR;
    }
    if (close(socket_net) < 0) ERROR;
    if (close(socket_unix) < 0) ERROR;
    if (unlink(file) < 0) ERROR;
}

void trap_signal(int sig, void (*handler)(int)) {
    struct sigaction act_usr;
    if (sigemptyset(&act_usr.sa_mask) < 0) ERROR;
    if (sigaddset(&act_usr.sa_mask, sig) < 0) ERROR;
    act_usr.sa_handler = handler;
    act_usr.sa_flags = 0;
    if (sigaction(sig, &act_usr, NULL) < 0) ERROR;
}

void init(char *path) {
    file = path;
    trap_signal(SIGINT, &exit_handler);
    if (atexit(&exitme) < 0) ERROR;
}

void create_unix_socket() {
    struct sockaddr_un addr_unix;

    if ((socket_unix = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) ERROR;

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, file, sizeof(addr_unix.sun_path) - 1);
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    if (bind(socket_unix, (struct sockaddr*)&addr_unix, sizeof(addr_unix)) < 0) ERROR;
    if (listen(socket_unix, 10) < 0) ERROR;
}

void create_net_socket(int port) {
    struct sockaddr_in addr_net;

    if ((socket_net = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) ERROR;

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);
    addr_net.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_net, (struct sockaddr*)&addr_net, sizeof(addr_net)) < 0) ERROR;
    if (listen(socket_net, 10) < 0) ERROR;
}



int main(int argc, char *argv[]) {
    fd_set set;
    int i, j, max;
    msg message;

    if(argc != 3) {
				printf("Bad arguments: server.run <port> <path>\n");
				exit(1);
		}

    init(argv[2]);
    create_unix_socket();
    create_net_socket(atoi(argv[1]));

    if(socket_net > socket_unix)
        max = socket_net + 1;
    else
        max = socket_unix + 1;

    while(true) {
        FD_ZERO(&set);
        FD_SET(socket_unix, &set);
        FD_SET(socket_net, &set);
        for(i = 0; i < number; i++)
            FD_SET(sockets[i], &set);


        if (select(max, &set, NULL, NULL, NULL) < 0) ERROR;


        if(FD_ISSET(socket_unix, &set)) {
            sockets[number] = accept(socket_unix, NULL, NULL);
            if(0 > sockets[number]) {
              perror("error accept failed");
              close(sockets[number] );
              exit(EXIT_FAILURE);
            }

            if(max < sockets[number] + 1)
                max = sockets[number] + 1;
            number++;
        }

        if(FD_ISSET(socket_net, &set)) {
            sockets[number] = accept(socket_net, NULL, NULL);
            if(0 > sockets[number]) {
              perror("error accept failed");
              close(sockets[number]);
              exit(EXIT_FAILURE);
            }

            if(max < sockets[number] + 1)
                max = sockets[number] + 1;
            number++;
        }


        for(i = 0; i < number; i++)
            if(FD_ISSET(sockets[i], &set)) {
                memset(&message, 0, sizeof(msg));
                if(read(sockets[i], &message, sizeof(msg)) == 0)
                    sockets[i] = sockets[--number];
                else
                    for(j = 0; j< number; j++)
                        if(j != i)
                            if (write(sockets[j], &message, sizeof(msg)), "Write error.");
            }
    }

    return 0;
}
