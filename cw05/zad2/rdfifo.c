#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
//server

int main(int argc, char ** argv) {
  char buffer[80];

  int fifo = mkfifo(argv[1], 0666);
  if (fifo == -1) perror("Error in mkfifo");

  while(1) { 
    FILE * fd = fopen(argv[1], "r");
   // if (fd == -1) perror("Error in open");
    
    fgets(buffer, 80,fd);
    printf("godzina teraz - %s\n", buffer);

    fclose(fd);
  }

  
  
  return 0;
}
