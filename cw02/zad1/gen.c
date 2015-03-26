#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {

  if (argc!=3) {
    printf("Bad number of arguments (should be 2). Example: generator.run <lines_number> <line_length>\n");
    exit(EXIT_FAILURE);
  }

  const char * si = argv[1];
  const char * len = argv[2];

  for(int i = 0; i < strlen(si); i++) {
    char curr = si[i];
    if ( curr < 48 || curr > 57 ) {
      printf("Bad argument - number of records should be integer\n");
      exit(EXIT_FAILURE);
    }
  }

  for(int i = 0; i < strlen(len); i++) {
    char curr = len[i];
    if ( curr < 48 || curr > 57 ) {
      printf("Bad argument - length of record should be integer\n");
      exit(EXIT_FAILURE);
    }
  }

  int size = atoi(si);
  int length = atoi(len);

  FILE *fp;
  srand(time(NULL));
  fp = fopen("file.txt","w");
  if (fp == NULL) {
    perror("Error while opening file");
    exit(EXIT_FAILURE);
  }

  int curr_rec = 0;
  int curr_pos = 0;

  while(curr_rec < size) {
    curr_pos = 0;
    while(curr_pos < length - 1) {
      fprintf(fp, "%c", 'A' + (rand() % 26));
      curr_pos += 1;
    }
    fprintf(fp, "\n");
    curr_rec += 1;
  }
  fclose(fp);
  exit(EXIT_SUCCESS);
}
