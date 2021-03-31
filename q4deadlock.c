#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PROCESSES 5
#define RESOURCES 5

//SEMAPHORE
//sem_t lock[RESOURCES];

void writing_process(int num)
{
    sleep(2);
    int process_num = num;
    int filenum, filenum2;
    char filename[20], filename2[20];
    char semName[20];
    int sem_value, sem_value2;
    sem_t *sem1, *sem2;
    FILE *fp;
    printf("process %d create \n", process_num);
    for (int i = 0; i < 3; i++)
    {

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
        snprintf(semName, 20, "test_%d", filenum);
        sem1 = sem_open(semName, O_EXCL);
        snprintf(semName, 20, "test_%d", filenum2);
        sem2 = sem_open(semName, O_EXCL);

        while (1)
        {
            if (sem_trywait(sem1) == 0)
            {
                if (sem_trywait(sem2) == 0)
                {
                    break;
                }
                else
                {
                    sem_post(sem1);
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
        printf("Process %d accessing %s \n", process_num, filename);
        fp = fopen(filename, "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);
        printf("Process %d accessing %s \n", process_num, filename2);
        fp = fopen(filename2, "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);
        sem_post(sem1);
        sem_post(sem2);
        printf("Process %d released %s and %s \n", process_num, filename, filename2);

        sleep(2);
    }
    printf("Process %d finished running. Exiting program.\n", process_num);
}

int main()
{
    pid_t pid[PROCESSES], cpid;
    int i, process[PROCESSES], stats = 0;
    char filename[20];
    FILE *fp;
    char semName[20];
    for (i = 0; i < RESOURCES; i++)
    {
        /*
        this is to create/open the file we are using for this program. 
        fopen(filename, "w"); -> to create/erase the file content.
        */
        snprintf(semName, 20, "test_%d", i);
        sem_open(semName,O_CREAT, 0777, 1);
        process[i] = i + 1;
        snprintf(filename, 20, "test_%d.txt", i);
        fp = fopen(filename, "w");
        fclose(fp);
    }

    for (i = 0; i < 5; i++)
    {
        /* 
        multiprocessing with fork. 
        pid[i] = fork() will create the child process
        if (pid[i] == 0) will only allow child process to execute the function then exiting the process
         */
        pid[i] = fork();
        if (pid[i] == 0)
        {
            writing_process(i);
            exit(100 + i);
        }
    }

    for (i = 0; i < 5; i++)
    /* 
    Parent waiting for child program to finish before terminating
     */
    {
        cpid = waitpid(pid[i], &stats, 0);
        printf("Child %d terminated with status: %d\n",
               cpid, WEXITSTATUS(stats));
    }
    return 0;
}
