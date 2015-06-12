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

typedef struct msg
{
    char from[32];
    char content[480];
} msg;

int exception (int ret, char* msg)
{
    if(ret == -1)
    {
        printf("%s\n", msg);
        perror("");
        exit(-1);
    }
    return ret;
}

void exit_handler()
{
    exit(0);
}

void exitme()
{
    int i;
    for(i = 0; i < number; i++)
    {
        shutdown(sockets[i], SHUT_RDWR);
        exception(close(sockets[i]), "Close error.");
    }
    exception(close(socket_net), "Close error.");
    exception(close(socket_unix), "Close error.");
    exception(unlink(file), "Unlink error.");
}

void trap_signal(int sig, void (*handler)(void))
{
    struct sigaction act_usr;
    exception(sigemptyset(&act_usr.sa_mask), "Couldn't initialize a signal set.");
    exception(sigaddset(&act_usr.sa_mask, sig), "Couldn't add a signal to the set.");
    act_usr.sa_handler = (void*)(handler);
    act_usr.sa_flags = 0;
    exception(sigaction(sig, &act_usr, NULL), "Sigaction failed.");
}

void init(char *path)
{
    file = path;
    trap_signal(SIGINT, &exit_handler);
    exception(atexit(&exitme), "Couldn't set cleaning function.");
}

void create_unix_socket()
{
    struct sockaddr_un addr_unix;

    socket_unix = exception(socket(AF_UNIX, SOCK_STREAM, 0), "Cannot create UNIX domain socket.");

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, file, sizeof(addr_unix.sun_path) - 1);
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    exception(bind(socket_unix, (struct sockaddr*)&addr_unix, sizeof(addr_unix)), "Bind UNIX domain socket failed.");
    exception(listen(socket_unix, 10), "Listen failed.");
}

void create_net_socket(int port)
{
    struct sockaddr_in addr_net;

    socket_net = exception(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP), "Cannot create TCP socket.");

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);
    addr_net.sin_addr.s_addr = htonl(INADDR_ANY);

    exception(bind(socket_net, (struct sockaddr*)&addr_net, sizeof(addr_net)), "Bind TCP socket failed.");
    exception(listen(socket_net, 10), "Listen failed.");
}



int main(int argc, char *argv[])
{
    fd_set set;
    int i, j, max;
    msg message;

    if(argc != 3)
        exception(-1, "Two arguments needed: port number (greater than 1000) and a filepath for the UNIX domain socket.\n");

    init(argv[2]);
    create_unix_socket();
    create_net_socket(atoi(argv[1]));

    if(socket_net > socket_unix)
        max = socket_net + 1;
    else
        max = socket_unix + 1;

    while(true)
    {
        FD_ZERO(&set);
        FD_SET(socket_unix, &set);
        FD_SET(socket_net, &set);
        for(i = 0; i < number; i++)
            FD_SET(sockets[i], &set);


        exception(select(max, &set, NULL, NULL, NULL), "Select error.");


        if(FD_ISSET(socket_unix, &set))
        {
            sockets[number] = accept(socket_unix, NULL, NULL);
            if(0 > sockets[number])
            {
              perror("error accept failed");
              close(sockets[number] );
              exit(EXIT_FAILURE);
            }

            if(max < sockets[number] + 1)
                max = sockets[number] + 1;
            number++;
        }

        if(FD_ISSET(socket_net, &set))
        {
            sockets[number] = accept(socket_net, NULL, NULL);
            if(0 > sockets[number])
            {
              perror("error accept failed");
              close(sockets[number]);
              exit(EXIT_FAILURE);
            }

            if(max < sockets[number] + 1)
                max = sockets[number] + 1;
            number++;
        }


        for(i = 0; i < number; i++)
            if(FD_ISSET(sockets[i], &set))
            {
                memset(&message, 0, sizeof(msg));
                if(read(sockets[i], &message, sizeof(msg)) == 0)
                    sockets[i] = sockets[--number];
                else
                    for(j = 0; j< number; j++)
                        if(j != i)
                            exception(write(sockets[j], &message, sizeof(msg)), "Write error.");
            }
    }

    return 0;
}
