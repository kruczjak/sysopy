#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>
#include <sched.h>

#define STACK_SIZE (1024 * 1024)
int counter;

clock_t forkTime(int);
clock_t vforkTime(int);
void wrapper(clock_t (*)(int), int);

clock_t forkTime(int N) {
  int status;
  clock_t start, end;
  counter = 0;
  clock_t childTime = 0;

  for (int i=0;i<N;i++) {
    pid_t child = fork();
    if (child == 0) {
      start = clock();
      counter++;
      end = clock();
      _exit(end-start);
    } else {
      wait(&status);
      childTime += WEXITSTATUS(status);
    }
  }

  return childTime;
}

clock_t vforkTime(int N) {
  int status;
  clock_t start, end;
  counter = 0;
  clock_t childTime = 0;

  for (int i=0;i<N;i++) {
    pid_t child = vfork();
    if (child == 0) {
      start = clock();
      counter++;
      end = clock();
      _exit(end-start);
    } else {
      wait(&status);
      childTime += WEXITSTATUS(status);
    }
  }

  return childTime;
}

int childFunction(void * arg) {
  clock_t start = clock();
  counter++;
  clock_t end = clock();
  _exit(end-start);
}

clock_t clone_fork(int N) {
	int status;
	char *childStack = malloc(STACK_SIZE);
	childStack += STACK_SIZE;
	counter = 0;
	clock_t childTime = 0;

	for(int i = 0; i < N; i++){
		clone(&childFunction, childStack, SIGCHLD, NULL);
		wait(&status);
    childTime += WEXITSTATUS(status);
	}

	return childTime;
}

clock_t clone_vfork(int N) {
	int status;
	char *childStack = malloc(STACK_SIZE);
	childStack += STACK_SIZE;
	counter = 0;
	clock_t childTime = 0;

	for(int i = 0; i < N; i++){
		clone(&childFunction, childStack,  SIGCHLD | CLONE_VM | CLONE_VFORK, NULL);
		wait(&status);
    childTime += WEXITSTATUS(status);
	}

	return childTime;
}

int main(int argc, char ** argv) {
  int N = 99999;
  wrapper(&forkTime, N);
  wrapper(&vforkTime, N);
  wrapper(&clone_fork, N);
  wrapper(&clone_vfork, N);
  return 0;
}

void wrapper(clock_t (*testing)(int N), int N) {
  clock_t start, end;
  struct tms start_tms, end_tms;
  start = clock();
  times(&start_tms);
  clock_t time = (*testing)(N);
  end = clock();
  times(&end_tms);


  printf("Counter: %d\n", counter);
  printf("Child real: %lfs\n", (double) time / CLOCKS_PER_SEC);
  printf("Child user: %lfs\n", (double) (end_tms.tms_cutime-start_tms.tms_cutime) / CLOCKS_PER_SEC);
  printf("Child system: %lfs\n", (double) (end_tms.tms_cstime-start_tms.tms_cstime) / CLOCKS_PER_SEC);
  printf("Parent real: %lfs\n", (double) (end-start) / CLOCKS_PER_SEC);
  printf("Parent user: %lfs\n", (double) (end_tms.tms_utime-start_tms.tms_utime) / CLOCKS_PER_SEC);
  printf("Parent system: %lfs\n", (double) (end_tms.tms_stime-start_tms.tms_stime) / CLOCKS_PER_SEC);
  printf("-------------------------------------\n");

}
