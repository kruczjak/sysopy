#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
pid_t child;
static int loop;
static int number;
static int sendingP;
void addSignals(int signo) {
  printf("P: Got\n");
  //odebrano
  if (sendingP==0) {
		number++;
		kill(child, SIGUSR1);
	} else
		loop = 0;
}

void endAdding(int signo) {
  printf("P: SIGUSR2\n");
  loop =0;
}

int main(int argc, char ** argv) {
  loop = 1;
	number = 0;

  if (argc !=2) {
    printf("Bad arguments\nmain <number_of_signals>");
		exit(1);
  }

	signal(SIGUSR1, addSignals);
	signal(SIGUSR2, endAdding);

  int n = atoi(argv[1]);
	printf("Forking\n");

  child = fork();
  if (child == -1) {
    printf("Error while creating child");
    exit(1);
  } else if (child == 0) {
      if (execl("child.run", "child.run", NULL) < 0) {
				printf("Error exec\n");
				_exit(1);
			}
  } else {
		while(loop) { pause(); }
		loop=1;
    sendingP = 1;
    printf("P: Sending\n");
    for(int i=0; i<n; i++) {
       kill(child, SIGUSR1);
       while (loop) {pause();}
       loop = 1;
     }

    printf("P: Last one\n");
    sendingP = 0;
    kill(child, SIGUSR2);


		wait(NULL);

		printf("Got %d SIGUSR1, should be %d", number, n);

  }
  return 0;
}
