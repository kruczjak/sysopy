#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

static struct tms start_tms;
static struct tms a_tms;
clock_t start_time;
clock_t a_time;

void my_fseek(FILE * handle, int byte, int flag) {
  if (fseek(handle, byte, flag)==-1) {
    perror("Error while fseeking file");
    exit(EXIT_FAILURE);
  }
}
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

void replace(FILE * handle, int length, int first, int second, char * buff1, char * buff2) {
    my_fseek(handle, length*first, SEEK_SET);
    if (fwrite(buff2,1, length, handle)<0) {
      perror("Error while writing file");
      exit(EXIT_FAILURE);
    }

    my_fseek(handle, length*second, SEEK_SET);
    if (fwrite(buff1, 1, length, handle)<0) {
      perror("Error while writing file");
      exit(EXIT_FAILURE);
    }
}



void sort(FILE * handle, int length) {

    char * buff1;
    char * buff2;
    int count = 0;
    char ch;
    do
    {
      ch = fgetc(handle);
      if(ch == '\n')
        count++;
    } while (ch != EOF);

    buff1 = (char *) malloc(length * sizeof(char));
    buff2 = (char *) malloc(length * sizeof(char));
    bool swapped = true;
    int j = 0;

    set_start_time();

    while(swapped) {
      swapped = false;
      j+=1;
      for(int i = 0; i < count - j; i += 1) {
        my_fseek(handle, length*i, SEEK_SET);
        if (fread(buff1, 1, length, handle)<0) {
          perror("Error while reading");
          exit(EXIT_FAILURE);
        }
        my_fseek(handle, length*(i+1), SEEK_SET);
        if (fread(buff2, 1, length, handle)<0) {
          perror("Error while reading");
          exit(EXIT_FAILURE);
        }
        if (buff1[0] > buff2[0]) {
            replace(handle, length, i, i+1, buff1, buff2);
            swapped=true;
        }
      }
    }
    free(buff1);
    free(buff2);
}



int main(int argc, char **argv) {
    if (argc!=3) {
        printf("Bad number of arguments (should be 2). Example: sort_bib.run <file> <line_length>");
        exit(1);
    }

    char * path = argv[1];
    const char * len = argv[2];

    for(int i = 0; i < strlen(len); i++) {
        char curr = len[i];
        if ( curr < 48 || curr > 57 ) {
            printf("Bad argument - length od record should be integer\n");
            exit(1);
        }
    }

    int length = atoi(len);

    FILE * handle = fopen(path, "r+");
    if (handle == NULL) {
      perror("Error opening file");
      exit(EXIT_FAILURE);
    }

    sort(handle, length);
    print_diff();
    fclose(handle);
    exit(EXIT_SUCCESS);
}
