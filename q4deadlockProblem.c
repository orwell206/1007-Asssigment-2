#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define PROCESSES 5
#define RESOURCES 5

//SEMAPHORE
sem_t *SEM_ARRAY[RESOURCES];
char FILENAME[RESOURCES][20];


void writing_process(int num)
{
    /* 
    num -> prcoess number
    function to write to 2 different files

     */
    sleep(2);
    int process_num = num;            /* reference process number */
    int filenum, filenum2;            /* to reference the file number to be open */
    char filename[20], filename2[20]; /* to reference the named file to be open with *fp */
    sem_t *sem1, *sem2;               /*      use for referencing named semaphore by combining   test_ + sem_value      */
    FILE *fp;                         /* file pointer */
    printf("process %d create \n", process_num);
    for (int i = 0; i < 3; i++)
    {

        switch (process_num)
        /* to assure there will always be a deadlock: 
        process 0 and process 1 will cause a deadlock for accessing each other resources 
        file 1 -> process 0 -> file 2 -> process 1 -> file 1
        process 2, process 3 and process 4 will cause a deadlock for accessing each other resources 
        file 0 -> process 2 -> file 3 -> process 4 -> file 4 -> process 3 -> file 0 
        */
        {
        case 0:
            filenum = 1;
            filenum2 = 2;
            break;

        case 1:
            filenum = 2;
            filenum2 = 1;
            break;

        case 2:
            filenum = 0;
            filenum2 = 3;
            break;

        case 3:
            filenum = 4;
            filenum2 = 0;
            break;

        case 4:
            filenum = 3;
            filenum2 = 4;
            break;

        default:
            exit;
        }

        sem1 = SEM_ARRAY[filenum];
        sem2 = SEM_ARRAY[filenum2];

        printf("Process %d accessing %s \n", process_num, FILENAME[filenum]);
        sem_wait(sem1);
        fp = fopen(FILENAME[filenum], "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);

        printf("Process %d accessing %s \n", process_num, FILENAME[filenum2]);
        if (process_num ==0)
        {
            sleep(1);
        }
        
        sem_wait(sem2);
        fp = fopen(FILENAME[filenum2], "a");
        fprintf(fp, "Process %d was here during loop %d\n", process_num, i);
        fclose(fp);

        sem_post(sem1);
        sem_post(sem2);
        printf("Process %d released %s and %s \n", process_num, FILENAME[filenum], FILENAME[filenum2]);

        sleep(2);
    }
    printf("Process %d finished running. Exiting program.\n", process_num);
}

int main()
{
    pid_t pid[PROCESSES], cpid;
    char semName[20];
    int i, process[PROCESSES], stats = 0;
    FILE *fp;
    for (i = 0; i < RESOURCES; i++)
    {
        /*
        print the filename and named semapohre into their respective arrays.
        this is to creat/open the file we are using for this program. 
        fopen(filename, "w"); -> to create/erase the file content.
        */
        process[i] = i + 1;
        snprintf(semName, 20, "test_deadd%d", i);
        SEM_ARRAY[i] = sem_open(semName, O_CREAT, 0777, 1);
        snprintf(FILENAME[i], 20, "test_%d.txt", i);
        fp = fopen(FILENAME[i], "w");
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
            exit(i);
        }
    }

    for (i = 0; i < 5; i++)
    /* 
    Parent waiting for child program to finish before terminating
     */
    {
        cpid = waitpid(pid[i], &stats, 0);
        printf("Child %d terminated with Process: %d\n",
               cpid, WEXITSTATUS(stats));
    }

    for (i = 0; i < 5; i++)
    {

        //sem_unlink(SEM_ARRAY[i]);
        sem_close(SEM_ARRAY[i]);
    }

    printf("closed all semaphore \n");

    return 0;
}
