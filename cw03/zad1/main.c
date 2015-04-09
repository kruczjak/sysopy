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

#define CLK sysconf(_SC_CLK_TCK)

int counter;
clock_t u_child = 0.0, s_child = 0.0, r_child = 0.0;

clock_t forkTime(int);
clock_t vforkTime(int);
void wrapper(clock_t (*)(int), int, char *);

clock_t forkTime(int N) {
  clock_t start, end;
  struct tms tms1, tms2;
  counter = 0;
  clock_t childTime = 0;

  for (int i=0;i<N;i++) {
    start = times(&tms1);
    pid_t child = fork();
    if (child == 0) {
      counter++;
      _exit(end-start);
    } else {
      wait(NULL);
      end = times(&tms2);
      childTime += end-start;
      u_child += (tms2.tms_utime - tms1.tms_utime);
      s_child += (tms2.tms_stime - tms1.tms_stime);
    }
  }

  return childTime;
}

clock_t vforkTime(int N) {
  clock_t start, end;
  struct tms tms1, tms2;
  counter = 0;
  clock_t childTime = 0;

  for (int i=0;i<N;i++) {
    start = times(&tms1);
    pid_t child = vfork();
    if (child == 0) {
      counter++;
      _exit(end-start);
    } else {
      wait(NULL);
      end = times(&tms2);
      childTime += end-start;
      u_child += (tms2.tms_utime - tms1.tms_utime);
      s_child += (tms2.tms_stime - tms1.tms_stime);
    }
  }

  return childTime;
}

int childFunction(void * arg) {
  counter++;
  _exit(0);
}

clock_t clone_fork(int N) {
	char *childStack = malloc(STACK_SIZE);
  struct tms tms1, tms2;
  clock_t start, end;
	childStack += STACK_SIZE;
	counter = 0;
	clock_t childTime = 0;

	for(int i = 0; i < N; i++){
    start = times(&tms1);
		clone(&childFunction, childStack, SIGCHLD, NULL);
		wait(NULL);
    end = times(&tms2);
    childTime += end-start;
    u_child += (tms2.tms_utime - tms1.tms_utime);
    s_child += (tms2.tms_stime - tms1.tms_stime);
	}

	return childTime;
}

clock_t clone_vfork(int N) {
	char *childStack = malloc(STACK_SIZE);
  struct tms tms1, tms2;
  clock_t start, end;
	childStack += STACK_SIZE;
	counter = 0;
	clock_t childTime = 0;

	for(int i = 0; i < N; i++){
    start = times(&tms1);
		clone(&childFunction, childStack,  SIGCHLD | CLONE_VM | CLONE_VFORK, NULL);
    wait(NULL);
    end = times(&tms2);
    childTime += end-start;
    u_child += (tms2.tms_utime - tms1.tms_utime);
    s_child += (tms2.tms_stime - tms1.tms_stime);
	}

	return childTime;
}

int main(int argc, char ** argv) {
  // FILE * f = fopen("data.txt");
  int N = atoi(argv[1]);
  wrapper(&forkTime, N, "fork");
  wrapper(&vforkTime, N, "vfork");
  wrapper(&clone_fork, N, "cloneFork");
  wrapper(&clone_vfork, N, "cloneVFork");
  return 0;
}

void wrapper(clock_t (*testing)(int N), int N, char * desc) {
  u_child = 0.0;
  s_child = 0.0;
  r_child = 0.0;
  clock_t start, end;
  struct tms start_tms, end_tms;
  start = times(&start_tms);
  clock_t time = (*testing)(N);
  end =  times(&end_tms);


  printf("%s\n", desc);
  printf("Counter: %d\n", counter);
  printf("Child real: %lfs\n", (double) time / CLK);
  printf("Child user: %lfs\n", (double) u_child / CLK);
  printf("Child system: %lfs\n", (double) s_child / CLK);
  printf("Parent real: %lfs\n", (double) (end-start) / CLK);
  printf("Parent user: %lfs\n", (double) (end_tms.tms_utime-start_tms.tms_utime) / CLK);
  printf("Parent system: %lfs\n", (double) (end_tms.tms_stime-start_tms.tms_stime) / CLK);
  printf("-------------------------------------\n");

}
