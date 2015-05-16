#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define N 100

int * TAB;
int seg_id;

int main(int argc, char ** argv) {

  if (argc != 2) {

  }


  key_t key;
  int shmflg = 0;

  int seg_size = sizeof(TAB);

  //allocate shared mem seg,
  if ((seg_id = shmget(IPC_PRIVATE, seg_size, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) {

  }

  atexit(clean);

  //attach sh mem seg
  if (*(TAB = (int *) shmat(seg_id, 0, 0)) < 0) {

  }



  if (strcmp(argv[1], "p") == 0) {
    producent();
  }
  else if (strcmp(argv[1], "c") == 0) {
    customer();
  }
  else {
    //error
  }


  return 0;
}

bool checkPrime(int x) {
  if (x == 0 || x == 1) {
    return false;
  }
  if (x == 2) {
    return true;
  }
  for (int i = 2; i*i <= x; i++ ) {
    if (x % i == 0) {
      return false;
    }
  }
  return true;
}

void clean() {
  shmdt(TAB);
  shmctl(seg_id, IPC_RMID, 0);
}
