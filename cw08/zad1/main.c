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
	char record[RECORD_SIZE + 1];
	char * buff = (char *) malloc(read_once * RECORD_SIZE * sizeof(char) + sizeof(char));

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

	int loop = 1;
	while (loop) {
		pthread_mutex_lock(&mutex);
		int readed = 0;
	  if ((readed = read(fd, buff, read_once * RECORD_SIZE)) == 0) {
			//end of records
			loop = 0;
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
		int readed_lines = readed / RECORD_SIZE;
		for (size_t i = 0; i < readed_lines; i++) {
			strncpy(record, buff, RECORD_SIZE);
			memmove(buff, buff + RECORD_SIZE, RECORD_SIZE * readed_lines);
			record[RECORD_SIZE] = '\0';
			char * text;
			int id = strtol(record, &text, 10);


		  if (strstr(text, search) != NULL) {
		    printf("%ld %d", syscall(SYS_gettid), id);
				#ifndef V3
			    for(int i = 0; i < number; i++)
			      if(pthread_equal(threads[i], pthread_self()))
							pthread_cancel(threads[i]);
					loop = 0;
				#endif
		  }
		}

		#ifdef V2
			pthread_testcancel();
		#endif
	}


  return NULL;
}

//record: id text
int main(int argc, char ** argv) {

  if (argc!=5) {
    printf("Arguments mismatch!: main.run <#threads> <file_name> <records> <word_to_find>\n");
    exit(1);
  }
  if ((fd = open(argv[2], O_RDONLY)) < 0) ERROR;

  number = strtol(argv[1], NULL, 10);
	read_once = strtol(argv[3], NULL, 10);
	search = argv[4];

  if ((threads = calloc(number,sizeof(pthread_t))) == NULL) ERROR;
	if(pthread_mutex_init(&mutex, NULL) < 0) ERROR;
  for(int i = 0; i < number; i++) {
    if (pthread_create(&(threads[i]), NULL, readLine, NULL) != 0) ERROR;
		#ifdef V3
			pthread_detach(threads[i]);
		#endif
  }

	#ifndef V3
  for(int i = 0; i < number; i++) {
    if (pthread_join(threads[i], NULL) != 0) ERROR;
  }
	#endif

	if (close(fd) < 0) ERROR;
	if(pthread_mutex_destroy(&mutex) < 0) ERROR;


  return 0;
}
