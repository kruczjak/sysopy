#include<stdio.h>
#include<stdlib.h>


int main(int argc, char ** argv) {
  if (argc !=2) {
    printf("Bad arguments\nmain <number_of_signals>");
  }

  int n = atoi(argv[1]);


  pid_t child = fork();
  if (child == -1) {
    printf("Error while creating child");
    exit(1);
  } else if (child == 0) {
      execlp("child.run", "child", NULL);
  } else {

    for(int i=0; i<n; i++) {
       kill(child, SIGUSR1);
    }
    kill(child, SIGUSR2);
    wait(NULL);
  }
  return 0;
}
