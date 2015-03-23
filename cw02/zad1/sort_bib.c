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
static struct tms last_tms;
static struct tms a_tms;
clock_t start_time;
clock_t last_time;
clock_t a_time;



void print_diff() {
    times(&a_tms);
    a_time = clock();
    printf("real\tlastdiff: %.12lf, startdiff: %.12lf\n",
            (double) (a_time - last_time) / CLOCKS_PER_SEC, (double) (a_time - start_time) / CLOCKS_PER_SEC);
    printf("user\tlastdiff: %.12lf, startdiff: %.12lf\n",
            (double) (a_tms.tms_utime - last_tms.tms_utime) / CLOCKS_PER_SEC, (double) (a_tms.tms_utime - start_tms.tms_utime) / CLOCKS_PER_SEC);
    printf("system\tlastdiff: %.12lf, startdiff: %.12lf\n",
            (double) (a_tms.tms_stime - last_tms.tms_stime) / CLOCKS_PER_SEC, (double) (a_tms.tms_stime - start_tms.tms_stime) / CLOCKS_PER_SEC);
    printf("--------------------------------\n");
    last_time = a_time;
    last_tms = a_tms;
}



void set_start_time() {
    times(&start_tms);
    last_tms = start_tms;
    start_time = last_time = clock();
}



void replace(FILE * handle, int length, int first, int second, char * buff1, char * buff2) {
    fseek(handle, length*first, SEEK_SET);
    fwrite(buff2,1, length, handle);

    fseek(handle, length*second, SEEK_SET);
    fwrite(buff1, 1, length, handle);
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
    set_start_time();
    
    int j = 0;
    while(swapped) {
      swapped = false;
      j+=1;
        for(int i = 0; i < count - j; i += 1) {
          fseek(handle, length*i, SEEK_SET);
          fread(buff1, 1, length, handle);
          fseek(handle, length*(i+1), SEEK_SET);
          fread(buff2, 1, length, handle);
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

    FILE * handle = fopen(path, "r+");

    sort(handle, length);
    print_diff();
    fclose(handle);
    exit(EXIT_SUCCESS);
}

void bubbleSort(int arr[], int n) {
      bool swapped = true;
      int j = 0;
      int tmp;
      while (swapped) {
            swapped = false;
            j++;
            for (int i = 0; i < n - j; i++) {
                  if (arr[i] > arr[i + 1]) {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        swapped = true;
                  }
            }
      }
}
