#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
static int loop;
static int number;

#undef SIGUSR1
#undef SIGUSR2

#define SIGUSR1 SIGRTMIN+2
#define SIGUSR2 SIGRTMIN+1

void addSignals(int signo) {
  printf("C: Got\n");
  number++;
}

void endAdding(int signo) {
  printf("C: SIGTRMIN+2\n");
  loop =0;
}
int main(int argc, char ** argv) {
  number = 0;
  loop = 1;

  if (signal(SIGUSR1, addSignals)== SIG_ERR)
    exit(1);
  if (signal(SIGUSR2, endAdding) == SIG_ERR)
    exit(1);

  //getting PID of parent
  FILE *fp = popen("pidof main.run", "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(1);
  }
  pid_t PID;
  fscanf(fp, "%d", &PID);
  printf("C: PPID %d", PID);
  //end getting

  kill(PID, SIGUSR2);
  while (loop) {
    pause();
  }

  for(int i=0; i<number; i++) {
     kill(PID, SIGUSR1);
  }
  kill(PID, SIGUSR2);

  return 0;
}
