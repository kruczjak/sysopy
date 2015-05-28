#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define RECORD_SIZE 1024

int main(int argc, char ** argv) {
  srand(time(NULL));
  if (argc != 2) {
    printf("Usage: generator.run <#lines>");
    exit(0);
  }
  int max = strtol(argv[1], NULL, 10);

  FILE * file = fopen("data.txt", "w");

  for (int i=1; i<=max; i++) {
    fprintf(file, "%d ", i);
    int length = floor (log10 (abs (i))) + 2;
    for (int j=0; j<RECORD_SIZE-length;j++) {
      fprintf(file, "%c", 'A' + (int)(random() % 26));
    }
  }

  fclose(file);
  return 0;
}
