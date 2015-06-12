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

#define TIMEOUT 30
#define CLIENTS 10
#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

typedef struct msg {
    char from[31];
    char content[480];
    bool registration;
} msg;

typedef struct client_net {
    struct sockaddr addr;
    int timeout;
} client_net;

typedef struct client_unix {
    struct sockaddr_un addr;
    int timeout;
} client_unix;


int socket_net, socket_unix, number_net = 0, number_unix = 0, expiring_client = -1;
char *file;
struct client_net data_net[CLIENTS];
struct client_unix data_unix[CLIENTS];


int exception (int ret, char* msg) {
    if(ret == -1) {
        printf("%s\n", msg);
        perror("");
        exit(-1);
    }
    return ret;
}

void exit_handler() {
    exit(0);
}

void exitme() {
    if (close(socket_net) < 0) ERROR;
    if (close(socket_unix) < 0) ERROR;
    if (unlink(file) < 0) ERROR;
}

int find_expiring_client(int difference) {
    int i, min = TIMEOUT + 1;

    for(i = 0; i < number_net; i++) {
        data_net[i].timeout -= difference;

        if(data_net[i].timeout == 0) {
            number_net--;
            data_net[i].addr = data_net[number_net].addr;
            data_net[i].timeout = data_net[number_net].timeout;
            i--;
            continue;
        }

        if(data_net[i].timeout < min) {
            min = data_net[i].timeout;
            expiring_client = i;
        }
    }

    for(i = 0; i < number_unix; i++) {
        data_unix[i].timeout -= difference;

        if(data_unix[i].timeout == 0) {
            number_unix--;
            data_unix[i].addr = data_unix[number_unix].addr;
            data_unix[i].timeout = data_unix[number_unix].timeout;
            i--;
            continue;
        }

        if(data_unix[i].timeout < min) {
            min = data_unix[i].timeout;
            expiring_client = CLIENTS + i;
        }
    }

    return min;
}

void alarm_handler() {
    int difference, min;

    if(expiring_client < CLIENTS) {
        number_net--;
        data_net[expiring_client].addr = data_net[number_net].addr;
        difference = data_net[expiring_client].timeout;
        data_net[expiring_client].timeout = data_net[number_net].timeout;
    } else {
        expiring_client -= CLIENTS;
        number_unix--;
        data_unix[expiring_client].addr = data_unix[number_unix].addr;
        difference = data_unix[expiring_client].timeout;
        data_unix[expiring_client].timeout = data_unix[number_unix].timeout;
    }

    min = find_expiring_client(difference);

    if(min < TIMEOUT + 1)
        alarm(min);
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
    trap_signal(SIGALRM, &alarm_handler);
    trap_signal(SIGINT, &exit_handler);
    if (atexit(&exitme) < 0) ERROR;
}

void create_net_socket(int port) {
    struct sockaddr_in addr_net;

    if ((socket_net = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) ERROR;

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);
    addr_net.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_net, (struct sockaddr*)&addr_net, sizeof(addr_net)) < 0) ERROR;
}

void create_unix_socket() {
    struct sockaddr_un addr_unix;

    if ((socket_unix = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) ERROR;

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, file, sizeof(addr_unix.sun_path) - 1);
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    if (bind(socket_unix, (struct sockaddr*)&addr_unix, sizeof(addr_unix)) < 0) ERROR;
}


int find_matching_client(struct sockaddr_in *addr_net, struct sockaddr_un *addr_unix, size_t size, bool net) {
    int i;
    if(net) {
        for (i = 0; i < number_net; i++)
            if(memcmp(&data_net[i].addr, addr_net, size) == 0)
                return i;
    } else {
        for (i = 0; i < number_unix; i++)
            if(memcmp(&data_unix[i].addr, addr_unix, size) == 0)
                return i;
    }

    return -1;
}


void registration(struct sockaddr_in *addr_net, struct sockaddr_un *addr_unix, size_t size, bool net) {
    int index, left, min;
    index = find_matching_client(addr_net, addr_unix, size, net);
    if(net) {
        if(index == -1) {
            memcpy(&data_net[number_net].addr, addr_net, size);
            data_net[number_net].timeout = TIMEOUT;
            number_net++;
        } else if(index != expiring_client)
            data_net[index].timeout = TIMEOUT;
        else {
            left = alarm(TIMEOUT + 1);
            min = find_expiring_client(data_net[index].timeout - left);
            data_net[index].timeout = TIMEOUT;
            if(min < TIMEOUT + 1)
                alarm(min);
        }
    } else {
        if(index == -1) {
            memcpy(&data_unix[number_unix].addr, addr_unix, size);
            data_unix[number_unix].timeout = TIMEOUT;
            number_unix++;

        } else if(index != expiring_client - CLIENTS)
            data_unix[index].timeout = TIMEOUT;
        else {
            left = alarm(TIMEOUT + 1);
            min = find_expiring_client(data_unix[index].timeout - left);
            data_unix[index].timeout = TIMEOUT;
            if(min < TIMEOUT + 1)
                alarm(min);
        }
    }
}

void dismiss(msg message, struct sockaddr_in *addr_net, struct sockaddr_un *addr_unix, size_t size_net, size_t size_unix, bool net) {
    int i, index;

    if(net) {
        index = find_matching_client(addr_net, addr_unix, size_net, net);
        for(i = 0; i < number_unix; i++)
            if (sendto(socket_unix, &message, sizeof(msg), 0, (struct sockaddr*)&data_unix[i].addr, size_unix) < 0) ERROR;

        for(i = 0; i < number_net; i++)
            if(i != index)
                if (sendto(socket_net, &message, sizeof(msg), 0, (struct sockaddr*)&data_net[i].addr, size_net) < 0) ERROR;
    } else {
        index = find_matching_client(addr_net, addr_unix, size_unix, net);
        for(i = 0; i < number_unix; i++)
            if(i != index)
                if (sendto(socket_unix, &message, sizeof(msg), 0, (struct sockaddr*)&data_unix[i].addr, size_unix) < 0) ERROR;

        for(i = 0; i < number_net; i++)
            if (sendto(socket_net, &message, sizeof(msg), 0, (struct sockaddr*)&data_net[i].addr, size_net) < 0) ERROR;
    }
}



int main(int argc, char *argv[]) {
    fd_set set;
    int max;
    msg message;

    struct sockaddr_in addr_net;
    socklen_t size_net = sizeof(addr_net);

    struct sockaddr_un addr_unix;
    socklen_t size_unix = sizeof(addr_unix);

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

        if (select(max, &set, NULL, NULL, NULL) < 0) ERROR;

        if(FD_ISSET(socket_net, &set)) {
            memset(&message, 0, sizeof(msg));
            if (recvfrom(socket_net, &message, sizeof(msg), 0, (struct sockaddr*)&addr_net, &size_net) < 0) ERROR;

            if(message.registration)
                registration(&addr_net, NULL, size_net, true);
            else
                dismiss(message, &addr_net, NULL, size_net, size_unix, true);
        }

        if(FD_ISSET(socket_unix, &set)) {
            memset(&message, 0, sizeof(msg));
            if (recvfrom(socket_unix, &message, sizeof(msg), 0, (struct sockaddr*)&addr_unix, &size_unix) < 0) ERROR;

            if(message.registration)
               registration(NULL, &addr_unix, size_unix, false);
            else
                dismiss(message, NULL, &addr_unix, size_net, size_unix, false);
        }
    }

    return 0;
}
