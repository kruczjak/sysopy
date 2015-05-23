#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t tid[5];
int counter;
pthread_mutex_t lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_cond_t fork_cond[5] = {PTHREAD_COND_INITIALIZER};
int forks[5] = {0};                             //0 means free fork

void* doPhilosophy(void *arg)
{
    int i;
    int id = (int)arg;
    pthread_mutex_lock(&lock);
    if(counter == 4) {                          //only 4 philosophers in monitor
        pthread_cond_wait(&cond, &lock);
    }
    counter++;
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&lock);                  //taking forks
    if(forks[id] == 1) {
        pthread_cond_wait(&fork_cond[id], &lock);
    }
    forks[id] = 1;
    if(forks[(id+1)%5] == 1) {
        pthread_cond_wait(&fork_cond[(id+1)%5], &lock);
    }
    forks[(id+1)%5] = 1;
    pthread_mutex_unlock(&lock);

    printf("Consumption %d started\n", id);     //consumption
    for(i=0; i<(0xFFFFFFFF);i++);
    printf("Consumption %d finished\n", id);

    pthread_mutex_lock(&lock);                  //returning forks
    forks[id] = 0;
    forks[(id+1)%5] = 0;
    pthread_cond_signal(&fork_cond[id]);
    pthread_cond_signal(&fork_cond[(id+1)%5]);
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&lock);                  //leaving monitor
    counter--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    int i = 0;
    int err;
    counter = 0;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    while(i < 5)
    {
        err = pthread_create(&(tid[i]), NULL, &doPhilosophy, (void*) i);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        i++;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    pthread_join(tid[3], NULL);
    pthread_join(tid[4], NULL);
    pthread_mutex_destroy(&lock);

    return 0;
}
