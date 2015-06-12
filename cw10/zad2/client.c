#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

typedef struct msg
{
    char from[32];
    char content[480];
} msg;

int sock, curr = 0;
char *sender, buf[480];

void exit_handler() {
    exit(0);
}

void exitme() {
    if (shutdown(sock, SHUT_RDWR) < 0) ERROR;
    if (close(sock) < 0) ERROR;
}

void trap_signal(int sig, void (*handler)(int)) {
    struct sigaction act_usr;
    if (sigemptyset(&act_usr.sa_mask) < 0) ERROR;
    if (sigaddset(&act_usr.sa_mask, sig) < 0) ERROR;
    act_usr.sa_handler = handler;
    act_usr.sa_flags = 0;
    if (sigaction(sig, &act_usr, NULL) < 0) ERROR;
}

void init(char *snd) {
    struct termios term;
    tcgetattr( STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON);
    tcsetattr( STDIN_FILENO, TCSANOW, &term);

    memset(buf, 0, sizeof(buf));
    sender = snd;
    printf("\n\n%s: ", sender);

    trap_signal(SIGINT, &exit_handler);
    if (atexit(&exitme) < 0) ERROR;
}

void* writer(void* arg) {
    msg message;
    fd_set set;

    while(true) {
        FD_ZERO(&set);
        FD_SET(sock, &set);

        if (select(sock + 1, &set, NULL, NULL, NULL) < 0) ERROR;


        if(FD_ISSET(sock, &set)) {
            memset(&message, 0, sizeof(msg));
            if(read(sock, &message, sizeof(msg)) > 0) {
                fflush(stdout);

                printf("%s: %s\n", message.from, message.content);

                printf("\n%s: ", sender);
                buf[curr] = 0;
                printf("%.*s", curr, buf);
                fflush(stdout);
            }
        }
    }
}

void send_message() {
    msg message;
    memset(&message, 0, sizeof(msg));
    if (strcpy(message.from, sender) == NULL) ERROR;
    if (strncpy(message.content, buf, curr-1) == NULL) ERROR;
    if (write(sock, &message, sizeof(msg)) < 0) ERROR;
}

void* reader(void* arg) {
    while(true) {
        buf[curr++] = getchar();

        if(buf[curr - 1] == '\x7f') {
            if(--curr)
                curr--;
            printf("%s: ", sender);
            printf("%.*s", curr, buf);
        } else if(curr > 2 && buf[curr - 3] == '\x1b' && buf[curr - 2] == '[') {
            curr -= 3;
            printf("%s: ", sender);
            printf("%.*s", curr, buf);
        } else if(buf[curr - 1] == '\n') {

            if(curr != 1) {
                send_message();
                printf("%s: %.*s\n", sender, curr-1, buf);
                memset(buf, 0, sizeof(buf));
            }

            curr = 0;
            printf("\n%s: ", sender);
        }
    }
}

void create_net_socket(char *nick, char *address, int port) {
    struct sockaddr_in addr_net;
    int res;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) ERROR;

    init(nick);

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);

    if ((res = inet_pton(AF_INET, address, &addr_net.sin_addr)) < 0) {
        printf("error: first parameter is not a valid address family");
        exit(1);
    } else if (res == 0) {
        printf("char string (second parameter does not contain valid ipaddress)");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&addr_net, sizeof(addr_net)) < 0) ERROR;
}

void create_unix_socket(char *nick, char *fileaddr_unix) {
    struct sockaddr_un addr_unix;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) ERROR;

    init(nick);

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, fileaddr_unix, sizeof(addr_unix.sun_path) - 1);
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    if (connect(sock, (struct sockaddr *)&addr_unix, sizeof(addr_unix)) < 0) ERROR;
}


int main(int argc, char *argv[]) {
    pthread_t reader_t, writer_t;

    if(argc == 4 && strcmp(argv[2], "local") == 0)
        create_unix_socket(argv[1], argv[3]);
    else
    if(argc == 5 && strcmp(argv[2], "remote") == 0)
        create_net_socket(argv[1], argv[3], atoi(argv[4]));
    else  {
      printf("Bad arguments: client.run <id> <local/remote> <ip port | if local>|<path | if remote>\n");
      exit(1);
    }

    if (pthread_create(&writer_t, NULL, writer, NULL) < 0) ERROR;
    if (pthread_create(&reader_t, NULL, reader, NULL) < 0) ERROR;
    if (pthread_join(writer_t, NULL) < 0) ERROR;
    if (pthread_join(reader_t, NULL) < 0) ERROR;

    exit(0);
}
