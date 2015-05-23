#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define RECORD_SIZE 1024
#define ERROR { int error_code = errno; \
				printf("FATAL (line %d): %s\n", __LINE__, strerror(error_code)); \
				exit(error_code);}

struct thread_arg {
  int fd;
  pthread_t * threads;
} data;

void * readLine(void * arg) {
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

  pthread_t * threads;
  if ((threads = malloc(number * sizeof(pthread_t))) == NULL) ERROR;

  data.threads = threads;

  for(int i = 0; i < number; i++) {
    if (pthread_create(&(threads[i]), NULL, &readLine, (void *) &data) != 0) ERROR;
  }



  for(int i = 0; i < number; i++) {
    if (pthread_join(threads[i], NULL) != 0) ERROR;
  }

  return 0;
}
