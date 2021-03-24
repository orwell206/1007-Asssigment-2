#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PROCESSES 5
#define RESOURCES 5

//SEMAPHORE
sem_t room;
sem_t chopstick[RESOURCES];
void eat(int);
void * philosopher(void *);

void eat(int phil)
{
	printf("\nPhilosopher %d is eating",phil);
}

void * philosopher_action(void * num)
{
	int phil=*(int *)num;
    //sem_wait(&room);
	printf("\nPhilosopher %d has entered room",phil);
    //taking chopstick
    sem_wait(&chopstick[phil]);
    printf("\nPhilosopher %d has taken chopstick %d",phil,phil);
    sleep(2);
    sem_wait(&chopstick[(phil+1)%RESOURCES]);
    printf("\nPhilosopher %d has taken chopstick %d",phil,(phil+1)%RESOURCES);

	eat(phil);
	sleep(2);
	printf("\nPhilosopher %d has finished eating",phil);
    sem_post(&chopstick[phil]);
    sem_post(&chopstick[(phil+1)%RESOURCES]);
    printf("\nPhilosopher %d has put down chopstick %d and %d\n",phil,phil,(phil+1)%RESOURCES);
    //sem_post(&room);
}


int main(int argc, char const *argv[])
{
    int i, Philosopher[PROCESSES];
    pthread_t thread_id[PROCESSES];
    //sem_init(&room,0,4);
    for(i=0;i<PROCESSES;i++)
		sem_init(&chopstick[i],0,1);

    for (i = 0; i < PROCESSES; i++)
    {
        Philosopher[i] = i;
        pthread_create(&thread_id[i],NULL,philosopher_action,(void *)&Philosopher[i]);
    }
    for ( i = 0; i < PROCESSES; i++)
    {
        pthread_join(thread_id[i],NULL);
    }
    
    
    return 0;
}
