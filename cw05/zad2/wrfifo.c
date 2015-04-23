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
  char info[80];
  char buff[70];

  if (argc != 2) {
    printf("Argument mismatch. Usage: rdinfo <fifo_name>\n");
    exit(1);
  }

  FILE * fd = fopen(argv[1], "w");
  printf("Enter info: ");

  fgets(info, 80, stdin);
  time_t t = time(NULL);
  struct tm * my_time;
  my_time = localtime(&t);
  strftime(buff, sizeof buff, "%T", my_time);
  fprintf(fd, "%i - %s - %s\n", getpid(), buff, info);

  fclose(fd);



  return 0;
}
