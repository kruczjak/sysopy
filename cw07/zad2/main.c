#define _XOPEN_SOURCE
#define _POSIX_SOURCE 2
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
#include <semaphore.h>
#include <fcntl.h>

#define EMPTYCOUNT "/podobne1"
#define FILLCOUNT "/podobne2"
#define MUTEX "/podobne3"
#define SHM_PATH "/nibysciezka"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define N 100
#define ERROR { int error_code = errno; \
				printf(KRED"FATAL (line %d): %s\n", __LINE__, strerror(error_code)); \
				exit(error_code);}


int seg_id;
sem_t * emptyCount, * fillCount, * mutex;


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

void up(sem_t * s) {
  if (sem_post(s) < 0) {
		ERROR;
	}
}

void down(sem_t * s) {
  if (sem_wait(s) < 0) {
		ERROR;
	}
}

void clean() {
  munmap(segment, sizeof(struct Segment));
  shm_unlink(SHM_PATH);
  sem_close(emptyCount);
	sem_unlink(EMPTYCOUNT);
	sem_close(fillCount);
	sem_unlink(FILLCOUNT);
	sem_close(mutex);
	sem_unlink(MUTEX);
}

void handler() {
	exit(0);
}

void producent() {
	struct Segment * curr = (struct Segment *) segment;
  while(1) {
    int number = rand() + 1;
		down(emptyCount);
    down(mutex);

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

		up(mutex);
		up(fillCount);
  }

}


void customer() {
  struct Segment * curr = (struct Segment *) segment;

  while(1) {
		down(fillCount);
    down(mutex);
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
    up(mutex);
		up(emptyCount);

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

  // sem 0: prod == emptyCount = MAX
	// sem 1: cons == fillCount = 0
	// sem 2: tab == mutex = 1
  if ((emptyCount = sem_open(EMPTYCOUNT, O_CREAT, S_IRUSR | S_IWUSR, N)) == SEM_FAILED) {
		ERROR;
	}

	if ((fillCount = sem_open(FILLCOUNT, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		ERROR;
	}

	if ((mutex = sem_open(MUTEX, O_CREAT, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		ERROR;
	}

	printf("Semaphores success\n");

	//SHARED MEMORY
  int seg_size = sizeof(struct Segment);
  //allocate shared mem seg,

  if ((seg_id = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG)) >= 0) {
    //success
		printf("Created new shared memory\n");
		ftruncate(seg_id, seg_size);
		if ((segment = mmap(NULL, seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, seg_id, 0)) == NULL) {
      ERROR;
    }
		printf("Allocated new shared memory\n");
    ((struct Segment * ) segment)->tab_ptr = 0;
  }
  else if ((seg_id = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG)) >= 0) {
    ftruncate(seg_id, seg_size);
		if ((segment = mmap(NULL, seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, seg_id, 0)) == NULL) {
      ERROR;
    }
  }
  else {
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
