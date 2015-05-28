#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define ERROR {printf("FATAL (line %d): %s\n", __LINE__, strerror(errno)); \
				exit(errno);}

pthread_t threads[5];
int counter;
pthread_mutex_t mutex;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

pthread_cond_t forkCondition[5] = {PTHREAD_COND_INITIALIZER};
int forks[5] = {0};

void* dinner(void *arg)
{
    int i;
    int id = (int)arg;
    pthread_mutex_lock(&mutex);
    if(counter == 4) {                          //only 4 philosophers in monitor
        pthread_cond_wait(&condition, &mutex);
    }
    counter++;
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex);                  //taking forks
    if(forks[id] == 1) {
        pthread_cond_wait(&forkCondition[id], &mutex);
    }
    forks[id] = 1;
    if(forks[(id+1)%5] == 1) {
        pthread_cond_wait(&forkCondition[(id+1)%5], &mutex);
    }
    forks[(id+1)%5] = 1;
    pthread_mutex_unlock(&mutex);

    printf("Eating %d start\n", id);     //consumption
    for(i=0; i<(0xFFFFFF);i++);
    printf("Eating %d ended\n", id);

    pthread_mutex_lock(&mutex);                  //returning forks
    forks[id] = 0;
    forks[(id+1)%5] = 0;
    pthread_cond_signal(&forkCondition[id]);
    pthread_cond_signal(&forkCondition[(id+1)%5]);

    counter--;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(void)
{
    counter = 0;

    if (pthread_mutex_init(&mutex, NULL) != 0) ERROR;

    for (int i = 0; i < 5; i++)
        if (pthread_create(&(threads[i]), NULL, &dinner, (void*) i) !=0) ERROR;

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);
    pthread_join(threads[4], NULL);
    pthread_mutex_destroy(&mutex);

    return 0;
}
