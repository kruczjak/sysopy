#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <signal.h>

#define RECORD_SIZE 1024
#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}
#define number 10


pthread_t * threads;

void * readLine(void * arg) {
  int a = (int) arg;

  for(int i=0; i<(0xFFFFF);i++);
  if(a == number-1) {
      int x = 9/0; //w ostatnim
  }
  for(int i=0; i<(0xFFFFF);i++);
  return NULL;
}

//record: id text
int main(int argc, char ** argv) {

  if ((threads = calloc(number,sizeof(pthread_t))) == NULL) ERROR;
  for(int i = 0; i < number; i++) {
    if (pthread_create(&(threads[i]), NULL, readLine, (void*)i) != 0) ERROR;
  }
  printf("PID: %d\n", getpid());

  for(int i = 0; i < number; i++) {
    if (pthread_join(threads[i], NULL) != 0) ERROR;
  }

  return 0;
}
