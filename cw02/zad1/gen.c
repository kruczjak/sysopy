#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {

  if (argc!=3) {
    perror("Bad number of arguments (should be 2)");
    exit(1);
  }

  const char * si = argv[1];
  const char * len = argv[2];

  for(int i = 0; i < strlen(si); i++) {
    char curr = si[i];
    if ( curr < 48 || curr > 57 ) {
      perror("Bad argument - number of records should be integer");
      exit(1);
    }
  }

  for(int i = 0; i < strlen(len); i++) {
    char curr = len[i];
    if ( curr < 48 || curr > 57 ) {
      perror("Bad argument - length od record should be integer");
      exit(1);
    }
  }

  int size = atoi(si);
  int length = atoi(len);

  FILE *fp;
  srand(time(NULL));
  fp = fopen("file.txt","w");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }
  int curr_rec = 0;
  int curr_pos = 0;
  while(curr_rec < size) {
    while(curr_pos < length) {

      char randomletter =   rand() % 255;
      fprintf(fp, "%c", randomletter);
      curr_pos += 1;
    }
    fprintf(fp, "\n");
    curr_rec += 1;
  }
  fclose(fp);
  exit(EXIT_SUCCESS);

}
