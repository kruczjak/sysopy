#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <time.h>
#include <locale.h>
//server

int main(int argc, char ** argv) {
  char buffer[80];
  char buff[70];
  if (argc != 2) {
    printf("Argument mismatch. Usage: rdinfo <fifo_name>\n");
    exit(1);
  }
  int fifo = mkfifo(argv[1], 0666);
  if (fifo == -1) perror("Error in mkfifo");
  printf("Godzina odczytu -   PID procesu klienta - godzina zapisu - treść komunikatu.\n");
  while(1) {
    FILE * fd = fopen(argv[1], "r");


    fgets(buffer, 80,fd);

    time_t t = time(NULL);
    struct tm * my_time;
    my_time = localtime(&t);
    strftime(buff, sizeof buff, "%T", my_time);

    printf("%s - %s", buff, buffer);

    fclose(fd);
  }



  return 0;
}
