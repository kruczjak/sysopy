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
      printf("Bad argument - lines number should be integer\n");
      exit(EXIT_FAILURE);
    }
  }

  for(int i = 0; i < strlen(len); i++) {
    char curr = len[i];
    if ( curr < 48 || curr > 57 ) {
      printf("Bad argument - lines length should be integer\n");
      exit(EXIT_FAILURE);
    }
  }

  int size = atoi(si);
  int length = atoi(len);

  FILE *fp;
  srand(time(NULL));
  fp = fopen("file.txt","w");
  if (fp == NULL) {
    perror("Error while opening the file");
    exit(EXIT_FAILURE);
  }

  for(int i = 0; i < size; i++) {
    for(int j= 0;j < length - 1;j++) {
      fprintf(fp, "%c", 'A' + (rand() % 26));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  exit(EXIT_SUCCESS);
}
