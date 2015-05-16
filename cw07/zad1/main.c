#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#define N 100

int seg_id;
int semid;

struct Segment {
  int TAB[N];
  int tab_ptr;
};

union semun {
  int val;
  struct semid_ds * buf;
  unsigned short int * array;
  struct seminfo * __buf;
}

void * segment;

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
  shmdt(segment);
  shmctl(seg_id, IPC_RMID, 0);
  union semun ignored_argument;
  semctl(semid, 1, IPC_RMID, ignored_argument);
}


void producent() {

  printf("1");
  while(1) {
    int number = rand();
    struct Segment * curr = (struct Segment *) segment;
    //printf("%d", curr->TAB[curr->tab_ptr]);
    if (curr->TAB[curr->tab_ptr] == -1) {
      curr->TAB[curr->tab_ptr++] = number;
      if (curr->tab_ptr == N) {
        curr->tab_ptr = 0;
      }
      int counter = 0;
      for (int i = 0; i < N; ++i) {
        if (curr->TAB[i] != -1) {
          counter += 1;
        }
      }
      printf("(%d %ld)   Dodalem liczbe: %d. Liczba zadan oczekujacych: %d. \n", getpid(), time(NULL), number, counter);
    }
    else {
      break;
    }

  }

}


void customer() {

  while(1) {
    //int number = rand();
    struct Segment * curr = (struct Segment *) segment;
    while (curr->TAB[curr->tab_ptr] == -1) {
      tab_ptr += 1;
      if (curr->tab_ptr == N) {
        curr->tab_ptr = 0;
    }
    int counter = 0;
    for (int i = 0; i < N; ++i) {
      if (curr->TAB[i] != -1) {
        counter += 1;
    }
    int number = curr-> TAB[curr->tab_ptr];
    curr-> TAB[curr->tab_ptr] = -1;
    //printf("%d", curr->TAB[curr->tab_ptr]);
    // if (curr->TAB[curr->tab_ptr] == -1) {
    //   curr->TAB[curr->tab_ptr++] = number;
    //   if (curr->tab_ptr == N) {
    //     curr->tab_ptr = 0;
    //   }

    //  }
    char * isFirst = "pierwsza";
    if (!checkPrime(number)) {
      isFirst = "zlozona";
    }
      printf("(%d %ld)   Sprawdzilem liczbe: %d - %s. Pozostalo zadan oczekujacych: %d. \n", getpid(), time(NULL), number, ifFirst, counter);
    }
    else {
      break;
    }

  }


}


int main(int argc, char ** argv) {

  if (argc != 2) {

  }

  srand(time(NULL));

  key_t key;
  int shmflg = 0;

  semget(IPC_PRIVATE, 1, 0);
  union semun argument;
  unsigned short values[1];
  values[0] = 1;
  argument.array = values;
  semctl(semid, 0, SETALL, argument);

  int seg_size = sizeof(struct Segment);

  //allocate shared mem seg,
  if ((seg_id = shmget(IPC_PRIVATE, seg_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) >= 0) {
    if (*((int *)(segment = shmat(seg_id, 0, 0))) < 0) {
      //error
    }
    // struct Segment to_realloc;
    // to_realloc.tab_ptr = 0;
    //
    // memcpy(&to_realloc, segment, sizeof(to_realloc));
    ((struct Segment * ) segment)->tab_ptr = 0;
    for (int i = 0; i < N; i += 1) {
      ((struct Segment * ) segment)->TAB[i] = -1;
    }
  }
  else if ((seg_id = shmget(IPC_PRIVATE, seg_size, S_IRUSR | S_IWUSR)) < 0) {
    //error
  }
  else if (*((int *)(segment = shmat(seg_id, 0, 0))) < 0) {

  }


  atexit(clean);

  //attach sh mem seg
  //if (*((int *)(segment = shmat(seg_id, 0, 0))) < 0) {
  //
  //}



  if (strcmp(argv[1], "p") == 0) {
    producent();
  }
  else if (strcmp(argv[1], "c") == 0) {
    customer();
  }
  else {
    printf("Wchodzi!");
  }


  return 0;
}
