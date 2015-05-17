#define _XOPEN_SOURCE
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define N 100
#define ERROR { int error_code = errno; \
				printf(KRED"FATAL (line %d): %s\n", __LINE__, strerror(error_code)); \
				exit(error_code);}

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
  // struct seminfo * __buf;
};

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

void up(int n) {
  struct sembuf semOp;
  semOp.sem_num = n;
  semOp.sem_op = 1;
  semOp.sem_flg = 0;
  if (semop(semid, &semOp, 1) < 0) {
		ERROR;
	}
}

void down(int n) {
  struct sembuf semOp;
  semOp.sem_num = n;
  semOp.sem_op = -1;
  semOp.sem_flg = 0;
  if (semop(semid, &semOp, 1) < 0) {
		ERROR;
	}
}

void clean() {
  shmdt(segment);
  shmctl(seg_id, IPC_RMID, 0);
  union semun ignored_argument;
  semctl(semid, 1, IPC_RMID, ignored_argument);
}

void handler() {
	exit(0);
}

void producent() {
	struct Segment * curr = (struct Segment *) segment;
  while(1) {
    int number = rand() + 1;
		down(0);
    down(2);

		while (curr->TAB[curr->tab_ptr] != 0) {
      curr->tab_ptr += 1;
      if (curr->tab_ptr == N) {
        curr->tab_ptr = 0;
      }
    }
		curr->TAB[curr->tab_ptr] = number;
    int counter = 0;
    for (int i = 0; i < N; ++i) {
      if (curr->TAB[i] != 0) {
        counter += 1;
      }
    }
		struct timeval  tv;
		gettimeofday(&tv, NULL);
		double time_in_mill =
       (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
    printf("(%d %.0lf)   Dodalem liczbe: %d. Liczba zadan oczekujacych: %d. \n", getpid(), time_in_mill, number, counter);

		up(2);
		up(1);
  }

}


void customer() {
  struct Segment * curr = (struct Segment *) segment;

  while(1) {
		down(1);
    down(2);
    while (curr->TAB[curr->tab_ptr] == 0) {
      curr->tab_ptr += 1;
      if (curr->tab_ptr == N) {
        curr->tab_ptr = 0;
      }
    }

    int number = curr-> TAB[curr->tab_ptr];
    curr-> TAB[curr->tab_ptr] = 0;
		int counter = 0;
		for (int i = 0; i < N; ++i) {
			if (curr->TAB[i] != 0) {
				counter += 1;
			}
		}
    up(2);
		up(0);

    char * isFirst = "pierwsza";
    if (!checkPrime(number)) {
      isFirst = "zlozona";
    }
		struct timeval  tv;
		gettimeofday(&tv, NULL);
		double time_in_mill =
			(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
    printf("(%d %.0lf)   Sprawdzilem liczbe: %d - %s. Pozostalo zadan oczekujacych: %d. \n", getpid(), time_in_mill, number, isFirst, counter);
    }
  }



int main(int argc, char ** argv) {
	printf(KGRN);
  if (argc != 2) {
    printf(KRED"Argument mismatch!: main.run <c|p>\n");
    exit(1);
  }

	if (signal(SIGINT, handler) == SIG_ERR) {
			ERROR;
	}
  srand(time(NULL));
	atexit(clean);

	//SEMAPHORES
	key_t key;
  if ( (key = ftok(".", 's')) < 0 ) {
    ERROR;
  }
  // sem 0: prod == emptyCount = MAX
	// sem 1: cons == fillCount = 0
	// sem 2: tab == mutex = 1
  if ((semid = semget(key, 3, 0666 | IPC_CREAT | IPC_EXCL)) >= 0) {
		printf("Created new semaphores\n");
		union semun arg_empty;
		union semun arg_full;
		union semun arg_critic;
		arg_empty.val = N;
		arg_full.val = 0;
		arg_critic.val = 1;
		if (semctl(semid, 0, SETVAL, arg_empty) < 0) {
				ERROR;
		}
		if (semctl(semid, 1, SETVAL, arg_full) < 0) {
				ERROR;
		}
		if (semctl(semid, 2, SETVAL, arg_critic) < 0) {
				ERROR;
		}
		printf("Setting new semaphores\n");
	} else if ((semid = semget(key, 3, 0)) < 0) {
		ERROR;
	}
	printf("Semaphores success\n");

	//SHARED MEMORY
  int seg_size = sizeof(struct Segment);
  //allocate shared mem seg,
	if ( (key = ftok(".", 1)) < 0 ) {
    ERROR;
  }
  if ((seg_id = shmget(key, seg_size, IPC_CREAT | IPC_EXCL | 0666)) >= 0) {
    //success
		printf("Created new shared memory\n");
		if (*((int *)(segment = shmat(seg_id, 0, 0))) < 0) {
      ERROR;
    }
		printf("Allocated new shared memory\n");
    ((struct Segment * ) segment)->tab_ptr = 0;
  }
  else if ((seg_id = shmget(key, seg_size, IPC_CREAT | 0666)) < 0) {
    ERROR;
  }
  else if (*((int *)(segment = shmat(seg_id, 0, 0))) < 0) {
    ERROR;
  }
	printf("Allocated shared memory success\n"KNRM);

	//RUN
  if (strcmp(argv[1], "p") == 0) {
    producent();
  }
  else if (strcmp(argv[1], "c") == 0) {
    customer();
  }
  else {
    printf(KRED"Argument mismatch!: main.run <c|p>\n");
    exit(1);
  }


  return 0;
}
