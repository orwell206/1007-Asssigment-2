#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define RESOURCES 3 
#define PROCESSES 5

char files[RESOURCES][6] = {"A.txt", "B.txt", "C.txt"}; // File Names

sem_t *resource_A;
sem_t *resource_B;
sem_t *resource_C;
sem_t *fcfs;
sem_t *counter;
sem_t *priority;

void generate_file() {
    for (int i = 0; i < RESOURCES; i++) {
        FILE *fp;
        fp = fopen(files[i], "w");
        fclose(fp);
        printf("File \"%s\" Generated.\n", files[i]); 
    }
}

void append_text(const char *filename, pid_t pid) {
    printf("Writing to \"%s\"\n", filename);

    FILE *fp;
    time_t t; 
    time(&t);

    fp = fopen(filename, "a");
    fprintf(fp, "%d", pid);
    fprintf(fp, " %s", ctime(&t));
    fprintf(fp, "%s", "\n");
    fclose(fp); 
}

void doing_something(const char *filename, pid_t pid){
    printf("Process %d: ", pid);
    append_text(filename, pid);
}

int starvation_aging() {
    if(sem_wait(fcfs) == 0) {
        sem_post(priority);
        int gpriority;
        sem_getvalue(priority, &gpriority);
        printf("Global Priority is now %d\n", gpriority);
        sem_post(fcfs);
        return 1;
    }
}

int main() {
    system("clear");
    generate_file(); 

    sem_unlink("fcfs");
    sem_unlink("counter");
    sem_unlink("priority");
    sem_unlink("A.txt");
    sem_unlink("B.txt");
    sem_unlink("C.txt");

    fcfs = sem_open("fcfs", O_CREAT | O_EXCL, 0777, 1);
    counter = sem_open("counter", O_CREAT | O_EXCL, 0777, 1);
    priority = sem_open("priority", O_CREAT | O_EXCL, 0777, 1);
    resource_A = sem_open("A.txt", O_CREAT | O_EXCL, 0777, 1);
    resource_B = sem_open("B.txt", O_CREAT | O_EXCL, 0777, 1);
    resource_C = sem_open("C.txt", O_CREAT | O_EXCL, 0777, 1);

    int id = fork();
    if (id != 0) {
        fork();
        fork();
    }

    pid_t pid = getpid();

    int lpriority;

    if(sem_wait(fcfs) == 0){
        sem_getvalue(counter, &lpriority);
        sem_post(counter);
        sem_post(fcfs);
    }

    printf("Process %d: Arrived with Priority %d\n", pid, lpriority);

    int gpriority;
    int completion = 000;

    while(completion != 111) {

        sem_getvalue(priority, &gpriority);

        if(lpriority <= gpriority) {

            if((completion & 100) != 100) {
                if (sem_trywait(resource_A) == 0) {
                    doing_something("A.txt", pid);
                    sleep(1);
                    completion = completion ^ 100;
                    lpriority += starvation_aging();
                    printf("Process %d: Priority is now %d\n", pid, lpriority);
                    sem_post(resource_A);
                }
            }
            if ((completion & 10) != 10) {
                if (sem_trywait(resource_B) == 0) {
                    doing_something("B.txt", pid);
                    sleep(1);
                    completion = completion ^ 10;
                    lpriority += starvation_aging();
                    printf("Process %d: Priority is now %d\n", pid, lpriority);
                    sem_post(resource_B);
                }
            }
            if ((completion & 1) != 1) {
                if (sem_trywait(resource_C) == 0 ) {
                    doing_something("C.txt", pid);
                    sleep(1);
                    lpriority += starvation_aging();
                    completion = completion ^ 1;
                    printf("Process %d: Priority is now %d\n", pid, lpriority);
                    sem_post(resource_C);
                }          
            }
        }
    }
    printf("Process %d: All Task Completed!\n", pid);
    //sem_post(priority);

    return(0);
}


