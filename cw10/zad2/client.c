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

typedef struct msg
{
    char from[32];
    char content[480];
} msg;

int sock, curr = 0;
char *sender, buf[480];


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

void exception2 (void* ret, char* msg)
{
    if(ret == NULL)
    {   
        printf("%s\n", msg);
        perror("");
        exit(-1);
    }
}

void exit_handler()
{
    exit(0);
}

void exitme()
{
    shutdown(sock, SHUT_RDWR);
    exception(close(sock), "Close error.");
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

void init(char *snd)
{
    struct termios term;
    tcgetattr( STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &term);

    memset(buf, 0, sizeof(buf));
    sender = snd;
    printf("\n\n%s: ", sender);
     
    trap_signal(SIGINT, &exit_handler);
    exception(atexit(&exitme), "Couldn't set cleaning function."); 
}

void* writer(void* arg)
{
    msg message;
    fd_set set;

    while(true)
    {
        FD_ZERO(&set);
        FD_SET(sock, &set);

        exception(select(sock + 1, &set, NULL, NULL, NULL), "Select error.");


        if(FD_ISSET(sock, &set))
        {
            memset(&message, 0, sizeof(msg));
            if(read(sock, &message, sizeof(msg)) > 0)
            {
                fflush(stdout);
                printf("%s", "\n\n\e[A\e[K");
                printf("%s", "\e[A\e[K\e[A");

                printf("%s: %s\n", message.from, message.content);

                printf("\n%s: ", sender);
                buf[curr] = 0;
                printf("%.*s", curr, buf);
                fflush(stdout);
            }
        }
    }
}

void send_message()
{
    msg message;
    memset(&message, 0, sizeof(msg));
    exception2(strcpy(message.from, sender), "String copy error.");
    exception2(strncpy(message.content, buf, curr-1), "Strncpy error.");
    exception(write(sock, &message, sizeof(msg)), "Message couldn't be sent");
}

void* reader(void* arg)
{
    while(true)
    {
        buf[curr++] = getchar();

        if(buf[curr - 1] == '\x7f')
        {
            if(--curr)
                curr--;
            printf("%s", "\n\e[A\e[K");
            printf("%s: ", sender);
            printf("%.*s", curr, buf);
        }
        else
        if(curr > 2 && buf[curr - 3] == '\x1b' && buf[curr - 2] == '[')
        {
            curr -= 3;
            printf("%s", "\n\e[A\e[K");
            printf("%s: ", sender);
            printf("%.*s", curr, buf);
        }
        else
        if(buf[curr - 1] == '\n')
        {
            printf("%s", "\n\e[A\e[K");
            printf("%s", "\e[A\e[K\e[A");  
            
            if(curr != 1)
            {    
                send_message();
                printf("%s: %.*s\n", sender, curr-1, buf); 
                memset(buf, 0, sizeof(buf));         
            }

            curr = 0;
            printf("\n%s: ", sender);
        }
    }
}

void create_net_socket(char *nick, char *address, int port)
{
    struct sockaddr_in addr_net;
    int res;

    sock = exception(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP), "Cannot create TCP socket.");
 
    init(nick);

    memset(&addr_net, 0, sizeof(addr_net));
    addr_net.sin_family = AF_INET;
    addr_net.sin_port = htons(port);

    if ((res = inet_pton(AF_INET, address, &addr_net.sin_addr)) < 0)
        exception(-1, "error: first parameter is not a valid address family");
    else 
    if (res == 0)
        exception(-1, "char string (second parameter does not contain valid ipaddress)");
 
    exception(connect(sock, (struct sockaddr *)&addr_net, sizeof(addr_net)), "Connection TCP failed.");
}

void create_unix_socket(char *nick, char *fileaddr_unix)
{
    struct sockaddr_un addr_unix;

    sock = exception(socket(AF_UNIX, SOCK_STREAM, 0), "Cannot create UNIX domain socket.");

    init(nick);

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, fileaddr_unix, sizeof(addr_unix.sun_path) - 1);
    addr_unix.sun_path[sizeof(addr_unix.sun_path) - 1] = '\0';

    exception(connect(sock, (struct sockaddr *)&addr_unix, sizeof(addr_unix)), "Connection UNIX domain failed.");
}


int main(int argc, char *argv[])
{
    pthread_t reader_t, writer_t;

    if(argc == 4 && strcmp(argv[2], "local") == 0)
        create_unix_socket(argv[1], argv[3]);
    else
    if(argc == 5 && strcmp(argv[2], "remote") == 0)
        create_net_socket(argv[1], argv[3], atoi(argv[4]));
    else
        exception(-1, "Arguments needed: your chat addr_unix, type of socket: \'local\' or \'remote\'.\n\tIf \'local\' then also a file path.\n\tIf \'remote\' - IP address and port number(greater than 1000).");

    exception(pthread_create(&writer_t, NULL, writer, NULL), "Thread creation error.");
    exception(pthread_create(&reader_t, NULL, reader, NULL), "Thread creation error.");
    exception(pthread_join(writer_t, NULL), "Thread join error.");
    exception(pthread_join(reader_t, NULL), "Thread join error.");
    
    exit(0);
}