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

void addSignals(int signo, siginfo_t * inf, void * ptr) {
	number++;
	kill(inf->si_pid, SIGUSR1);
}

void endAdding(int signo) {
  printf("C: SIGUSR2\n");
  loop =0;
}
int main(int argc, char ** argv) {
  number = 0;
  loop = 1;

  struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	sigfillset(&action.sa_mask);
	action.sa_sigaction = addSignals;


  if (sigaction(SIGUSR1, &action, NULL))
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
     while(loop) {
       pause();
     }
     loop = 1;
  }
  kill(PID, SIGUSR2);

  return 0;
}
