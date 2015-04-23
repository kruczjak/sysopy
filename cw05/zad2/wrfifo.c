#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
//server

int main(int argc, char ** argv) {
  char info[80];

  FILE * fd = fopen(argv[1], "w");
//  if (fd == -1) perror("Error in open");
  printf("Enter info: ");

  fgets(info, 80, stdin);
  fprintf(fd, "godzina zapisu - %s\n", info);

  fclose(fd);

  
  
  return 0;
}
