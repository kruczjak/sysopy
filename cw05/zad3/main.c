#include <stdio.h>
#include <stdlib.h>

char readbuf[80];

int main(int argc, char ** argv) {
  FILE * pipeStart = popen("ls -l | grep ^d","r");
  FILE * out = fopen("file.txt","w");

  while(fgets(readbuf, 80, pipeStart))
    fputs(readbuf, out);

  pclose(pipeStart);
  pclose(out);
}
