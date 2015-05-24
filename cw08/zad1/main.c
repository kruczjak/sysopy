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

#define RECORD_SIZE 1024
#define ERROR { int error_code = errno; \
				printf("FATAL (line %d): %s\n", __LINE__, strerror(error_code)); \
				exit(error_code);}

pthread_t * threads;

int fd;
int number;
int read_once;
char * search;

void * readLine(void * arg) {

  char buff[RECORD_SIZE * read_once];
  if (read(fd, buff, RECORD_SIZE * read_once) != RECORD_SIZE * read_once) ERROR;

  if (strstr(buff, search) != NULL) {
    printf("%d %d", (int) pthread_self(), 20); //TODO
    for(int i = 0; i < number; i++)
      if(threads[i] != (int) pthread_self())
        pthread_cancel(threads[i]);
  }

  return NULL;
}

//record: id text
int main(int argc, char ** argv) {
  int number = 0;
  int fd;
  if (argc!=5) {
    printf("Arguments mismatch!: main.run <#threads> <file_name> <records> <word_to_find>\n");
    exit(1);
  }
  if ((fd = open(argv[2], O_RDONLY)) < 0) ERROR;

  int multiplicate = 1;
  for(int i = strlen(argv[1]) - 1; i >= 0;i++) {
    multiplicate *= 10;
    number += (argv[1][i] - '0') * multiplicate;
  }

  if ((threads = malloc(number * sizeof(pthread_t))) == NULL) ERROR;

  for(int i = 0; i < number; i++) {
    if (pthread_create(&(threads[i]), NULL, &readLine, NULL) != 0) ERROR;
  }



  for(int i = 0; i < number; i++) {
    if (pthread_join(threads[i], NULL) != 0) ERROR;
  }

  return 0;
}
