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
#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

pthread_t * threads;
pthread_mutex_t mutex;

int fd;
int number = 0;
int read_once = 0;
char * search;
int size;
int row = 0;

void * readLine(void * arg) {
	char * buff = (char *) malloc(read_once * RECORD_SIZE);

	#ifdef V1
	if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) < 0) ERROR;
	#elif V2
	if(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) < 0) ERROR;
	#endif

	#ifdef V3
	if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) < 0) ERROR;
	#else
	if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) < 0) ERROR;
	#endif

	pthread_mutex_lock(&mutex);


  if (read(fd, buff, read_once * RECORD_SIZE) != read_once*RECORD_SIZE) ERROR;

	char * sub;
  if ((sub = strstr(buff, search)) != NULL) {
		int position = buff - sub;
		row += position/RECORD_SIZE;
		int right = position % RECORD_SIZE;
		char * rowStart = sub - right;
		int rowID = 0;
		int multiplicate = 1;
		while (rowStart!=' ' || rowStart!='\0') {
			number += (*rowStart - '0') * multiplicate;
			multiplicate *= 10;
			rowStart++;
		}
    printf("%d %d", (int) pthread_self(), rowID);
    for(int i = 0; i < number; i++)
      if(threads[i] != (int) pthread_self()) pthread_cancel(threads[i]);
  }
	row +=2;
	#ifdef V2
	pthread_testcancel();
	#endif

	pthread_mutex_unlock(&mutex);

  return NULL;
}

//record: id text
int main(int argc, char ** argv) {

  if (argc!=5) {
    printf("Arguments mismatch!: main.run <#threads> <file_name> <records> <word_to_find>\n");
    exit(1);
  }
  if ((fd = open(argv[2], O_RDONLY)) < 0) ERROR;

  int multiplicate = 1;
  for(int i = strlen(argv[1]) - 1; i >= 0;i++) {
    number += (argv[1][i] - '0') * multiplicate;
    multiplicate *= 10;
  }

	multiplicate = 1;
  for(int i = strlen(argv[1]) - 1; i >= 0;i++) {
    read_once += (argv[3][i] - '0') * multiplicate;
    multiplicate *= 10;
  }

	printf("%d %d\n", number, read_once);

  if ((threads = malloc(number * sizeof(pthread_t))) == NULL) ERROR;
	if(pthread_mutex_init(&mutex, NULL) < 0) ERROR;
  for(int i = 0; i < number; i++) {
    if (pthread_create(&(threads[i]), NULL, &readLine, NULL) != 0) ERROR;
  }

  for(int i = 0; i < number; i++) {
    if (pthread_join(threads[i], NULL) != 0) ERROR;
  }

	if (close(fd) < 0) ERROR;
	if(pthread_mutex_destroy(&mutex) < 0) ERROR;


  return 0;
}
