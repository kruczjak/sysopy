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

#define TIMEOUT 30
#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

typedef struct msg
{
    char from[31];
    char content[480];
    bool registration;
} msg;

int sock, curr = 0, size;
char buf[480], nick[31];

struct sockaddr_in addr_net;
struct sockaddr_un addr_unix;
struct sockaddr* server;

void exit_handler() {
    exit(0);
}

void exitme() {
    if (shutdown(sock, SHUT_RDWR) < 0) ERROR;
    if (close(sock) < 0) ERROR;
    if (unlink(nick) < 0) ERROR;
}

void alarm_handler() {
    msg message;
    message.registration = true;
    if (sendto(sock, &message, sizeof(msg), 0, server, size) < 0) ERROR;
    alarm(TIMEOUT - 5);
}

void trap_signal(int sig, void (*handler)(int)) {
    struct sigaction act_usr;
    if (sigemptyset(&act_usr.sa_mask) < 0) ERROR;
    if (sigaddset(&act_usr.sa_mask, sig) < 0) ERROR;
    act_usr.sa_handler = handler;
    act_usr.sa_flags = 0;
    if (sigaction(sig, &act_usr, NULL) < 0) ERROR;
}

void init() {
    trap_signal(SIGINT, &exit_handler);
    if (atexit(&exitme) < 0) ERROR;
    trap_signal(SIGALRM, &alarm_handler);

    alarm_handler();

    struct termios term;
    tcgetattr( STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON);
    tcsetattr( STDIN_FILENO, TCSANOW, &term);

    memset(buf, 0, sizeof(buf));
    printf("\n\n%s: ", nick);

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
            if(recvfrom(sock, &message, sizeof(msg), 0, NULL, 0) > 0) {
                fflush(stdout);

                printf("%s: %s\n", message.from, message.content);

                printf("\n%s: ", nick);
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
    message.registration = false;
    if (strcpy(message.from, nick) == NULL) ERROR;
    if (strncpy(message.content, buf, curr-1) == NULL) ERROR;
    if (sendto(sock, &message, sizeof(msg), 0, server, size) < 0) ERROR;
}

void* reader(void* arg) {
    while(true) {
        buf[curr++] = getchar();

        if(buf[curr - 1] == '\x7f') {
            if(--curr)
                curr--;
            printf("%s: ", nick);
            printf("%.*s", curr, buf);
        }
        else if(curr > 2 && buf[curr - 3] == '\x1b' && buf[curr - 2] == '[') {
            curr -= 3;
            printf("%s: ", nick);
            printf("%.*s", curr, buf);
        } else if(buf[curr - 1] == '\n') {

            if(curr != 1) {
                send_message();
                printf("%s: %.*s\n", nick, curr-1, buf);
                memset(buf, 0, sizeof(buf));
            }

            curr = 0;
            printf("\n%s: ", nick);
        }
    }
}

void create_net_socket(char *address, int port) {
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) ERROR;

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);
    addr_net.sin_addr.s_addr = inet_addr(address);

    server = (struct sockaddr*)&addr_net;
    size = sizeof(addr_net);
    init();
}

void create_unix_socket(char *filename) {

    if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) ERROR;

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    if (strncpy(addr_unix.sun_path, nick, sizeof(addr_unix.sun_path) - 1) == NULL) ERROR;
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    if (bind(sock, (struct sockaddr*)&addr_unix, sizeof(addr_unix)) < 0) ERROR;

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    if (strncpy(addr_unix.sun_path, filename, sizeof(addr_unix.sun_path) - 1) == NULL) ERROR;
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    server = (struct sockaddr*)&addr_unix;
    size = sizeof(addr_unix);
    init();

}


int main(int argc, char *argv[]) {
    pthread_t reader_t, writer_t;

    if(argc == 4 && strcmp(argv[2], "local") == 0) {
        if (strcpy(nick, argv[1]) == NULL) ERROR;
        create_unix_socket(argv[3]);
    } else if(argc == 5 && strcmp(argv[2], "remote") == 0) {
        if (strcpy(nick, argv[1]) == NULL) ERROR;
        create_net_socket(argv[3], atoi(argv[4]));
    } else {
      printf("Bad arguments: client.run <id> <local/remote> <ip port | if local>|<path | if remote>\n");
      exit(1);
    }

    if (pthread_create(&writer_t, NULL, writer, NULL) < 0) ERROR;
    if (pthread_create(&reader_t, NULL, reader, NULL) < 0) ERROR;
    if (pthread_join(writer_t, NULL) < 0) ERROR;
    if (pthread_join(reader_t, NULL) < 0) ERROR;

    exit(0);
}
