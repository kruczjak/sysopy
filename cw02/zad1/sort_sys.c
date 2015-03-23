#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

static struct tms start_tms;
static struct tms a_tms;
clock_t start_time;
clock_t a_time;

void print_diff() {
  times(&a_tms);
  a_time = clock();
  printf("real\tstartdiff: %.12lf\n", (double) (a_time - start_time) / CLOCKS_PER_SEC);
  printf("user\tstartdiff: %.12lf\n", (double) (a_tms.tms_utime - start_tms.tms_utime) / CLOCKS_PER_SEC);
  printf("system\tstartdiff: %.12lf\n", (double) (a_tms.tms_stime - start_tms.tms_stime) / CLOCKS_PER_SEC);
  printf("--------------------------------\n");
}

void set_start_time() {
  times(&start_tms);
  start_time = clock();
}

void replace(int handle, int length, int first, int second, char * buff1, char * buff2) {
  lseek(handle, length*first, SEEK_SET);
  write(handle, buff2, length);

  lseek(handle, length*second, SEEK_SET);
  write(handle, buff1, length);
}



void sort(int handle, int length) {

  char buff1[length];
  char buff2[length];
  int count = lseek(handle, 0L, SEEK_END) / length;

  bool swapped = true;
  int j = 0;

  set_start_time();

  while(swapped) {
    swapped = false;
    j+=1;
      for(int i = 0; i < count - j; i += 1) {
        lseek(handle, length*i, SEEK_SET);
        read(handle, &buff1, length);
        lseek(handle, length*(i+1), SEEK_SET);
        read(handle, &buff2, length);
        if (buff1[0] > buff2[0]) {
            replace(handle, length, i, i+1, buff1, buff2);
            swapped=true;
        }
      }
  }
}



int main(int argc, char **argv) {
  if (argc!=3) {
    perror("Bad number of arguments (should be 2)");
    exit(1);
  }

  char * path = argv[1];
  const char * len = argv[2];

  for(int i = 0; i < strlen(len); i++) {
    char curr = len[i];
    if ( curr < 48 || curr > 57 ) {
      perror("Bad argument - length od record should be integer");
      exit(1);
    }
  }

  int length = atoi(len);

  int handle = open(path, O_RDWR);

  sort(handle, length);

  print_diff();
  close(handle);
  exit(EXIT_SUCCESS);
}
