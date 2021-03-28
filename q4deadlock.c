#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PROCESSES 5
#define RESOURCES 5

//SEMAPHORE
sem_t lock[RESOURCES];
void *writing_process(void *num)
{
    int process_num = *(int *)num;
    int filenum, filenum2;
    char filename[20], filename2[20];
    int sem_value, sem_value2;
    FILE *fp;
    for (int i = 0; i < 3; i++)
    {
        sleep(2);
        filenum = rand() % RESOURCES;
        filenum2 = rand() % RESOURCES;
        while (1)
        {
            if (filenum2 != filenum)
            {
                break;
            }
            filenum2 = rand() % RESOURCES;
        }

        while (1)
        {
            if (sem_trywait(&lock[filenum]) == 0)
            {
                if (sem_trywait(&lock[filenum2]) == 0)
                {
                    break;
                }
                else
                {
                    sem_post(&lock[filenum]);
                    sleep(2);
                }
            }
            else
            {
                sleep(2);
            }
        }

        snprintf(filename, 20, "test_%d.txt", filenum);
        snprintf(filename2, 20, "test_%d.txt", filenum2);
        //sem_wait(&lock[filename]);
        printf("Process %d accessing %s \n", process_num, filename);
        fp = fopen(filename, "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);
        //sem_wait(&lock[filename2]);
        printf("Process %d accessing %s \n", process_num, filename2);
        fp = fopen(filename2, "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);
        sem_post(&lock[filenum]);
        sem_post(&lock[filenum2]);
        printf("Process %d released %s and %s \n", process_num, filename,filename2);
    }
}

int main()
{
    int i, process[PROCESSES];
    char filename[20];
    pthread_t thread_id[PROCESSES];
    FILE *fp;
    for (i = 0; i < RESOURCES; i++)
    {
        process[i] = i + 1;
        sem_init(&lock[i], 0, 1);
        snprintf(filename, 20, "test_%d.txt", i);
        fp = fopen(filename, "w");
        fclose(fp);
    }
    for (i = 0; i < PROCESSES; i++)
    {
        pthread_create(&thread_id[i], NULL, writing_process, &process[i]);
    }

    for (i = 0; i < PROCESSES; i++)
    {
        pthread_join(thread_id[i], NULL);
    }

    return 0;
}
