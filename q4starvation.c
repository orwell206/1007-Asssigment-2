#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>

#define RESOURCES 3 
#define PROCESSES 5 

char files[RESOURCES][6] = {"A.txt", "B.txt", "C.txt"}; // File names

sem_t *fcfs; // fcfs queuing semaphore
sem_t *counter; // incrementing counter semaphore
sem_t *priority; // global priority semaphore
sem_t *start; // control start semaphore
sem_t *resource_A; // resource access semaphore
sem_t *resource_B; // resource access semaphore
sem_t *resource_C; // resource access semaphore

void linebreak() {
    /* Line Break Printing Function */

    printf("==================================\n");
}

void generate_file() {
    /* Generate files based on file[] content */

    for (int i = 0; i < RESOURCES; i++) { // Loops pre-determined amount to create files
        FILE *fp;
        fp = fopen(files[i], "w"); // Create or overwrite existing files
        fclose(fp);
        printf("File \"%s\" Generated.\n", files[i]); 
    }
}

void append_text(const char *filename, int lprocess) {
    /* Append Process & Time onto selected files */

    printf("Writing to \"%s\"\n", filename);

    FILE *fp;
    time_t t; 
    time(&t); // set current time

    fp = fopen(filename, "a");
    fprintf(fp, "Process: %d", lprocess); // Local process if
    fprintf(fp, " | %s", ctime(&t)); // current time date year
    fprintf(fp, "%s", "\n");
    fclose(fp); 
    sleep(1); // simulated work time
}

void doing_something(const char *filename, int lprocess){
    /* General function that contains some task */

    printf("Process %d: ", lprocess); // Process identification
    append_text(filename, lprocess); // Append text task 
}

int starvation_aging(int lprocess, int lpriority) {
    /* Starvation Solution that introduce aging of priority */

    if(sem_wait(fcfs) == 0) { // Waiting to update queue
        sem_post(priority); // Updates priority
        int gpriority; // local "global priority" variable 
        sem_getvalue(priority, &gpriority); // set value to gpriority to be used as reference
        //printf("Global Priority is now %d\n", gpriority);
        //printf("Process %d: Priority increased to %d\n", lprocess, lpriority);
        sem_post(fcfs); // release queue
        return 1; // returns 1 to update "local priority"
    }
}

void clear_semaphore() {
    /* Cleanup named semaphores */

    sem_unlink("fcfs");
    sem_unlink("counter");
    sem_unlink("priority");
    sem_unlink("start");
    sem_unlink("A.txt");
    sem_unlink("B.txt");
    sem_unlink("C.txt");
}

void create_semaphore() {
    /* Create named semaphores */

    fcfs = sem_open("fcfs", O_CREAT | O_EXCL, 0777, 1); // Used to setup FCFS condition
    counter = sem_open("counter", O_CREAT | O_EXCL, 0777, 1); // Used to give priority value based on FCFS
    priority = sem_open("priority", O_CREAT | O_EXCL, 0777, 1); // Used to determine access priority for processes
    start = sem_open("start", O_CREAT | O_EXCL, 0777, 1); // Used to let processes start working as close as possible
    resource_A = sem_open("A.txt", O_CREAT | O_EXCL, 0777, 1); // Used to control access to "A.txt"
    resource_B = sem_open("B.txt", O_CREAT | O_EXCL, 0777, 1); // Used to control access to "B.txt"
    resource_C = sem_open("C.txt", O_CREAT | O_EXCL, 0777, 1); // Used to control access to "C.txt"
}

int main() {

    system("clear"); // Clear terminal

    generate_file(); 
    linebreak();

    clear_semaphore();
    create_semaphore();
    
    int id = fork(); // Child process
    if (id != 0) { // If not parent
        fork(); // Child process
        fork(); // Child process
    } // Total of 5 process

    int lpriority; // Local priority
    int lprocess; // Local process ID

    if(sem_wait(fcfs) == 0){ // FCFS queueing 
        sem_getvalue(counter, &lprocess); // Local Process ID given based on FCFS
        sem_getvalue(counter, &lpriority); // Priority given based on FCFS
        printf("Process %d: Arrived with Priority %d\n", lprocess, lpriority);
        sem_post(counter); // Increment coutner for next process
        sem_post(fcfs); // Release Semaphore
    }

    if(id == 0) { // If parent condition
        sem_wait(start); // Makes the original process hold
        sleep(2); // Sleeps for a arbitrary amount to wait for other child process
        linebreak();
        sem_post(start); // Release semaphore to start working
        printf("Process Released to Start Working.\n");
        linebreak();
    }

    if(sem_wait(start) == 0) { // If child condition to wait for parent
        sem_post(start); // Release semaphore to start work to allow a fair start
    }
    
    int gpriority; // Local "global priority" variable used to keep track of the priority semaphore
    int lcompletion = 000; // Task completion condition marker

    struct timeval begin, end; // Timer struct
    gettimeofday(&begin, 0); // Get current timeofday to start timer

    while(lcompletion != 111) { // Completion condition 

        sem_getvalue(priority, &gpriority); // Updates the gpriority variable for facilitatom

        if(lpriority <= gpriority) { // If local priority is <= to priority *artifical condition that causes starvation*

            if((lcompletion & 100) != 100) { // Completion checker by using AND operation
                if (sem_trywait(resource_A) == 0) { // Trying to get access to resource semaphore
                    doing_something("A.txt", lprocess); // Simulate working
                    lcompletion = lcompletion ^ 100; // Updates Completion maker by using XOR operation
                    lpriority += starvation_aging(lprocess, lpriority); // Updates local priority value 
                    sem_post(resource_A); // Release Semaphore
                }
            }
            if ((lcompletion & 10) != 10) { // Completion checker by using AND operation
                if (sem_trywait(resource_B) == 0) { // Trying to get access to resource semaphore
                    doing_something("B.txt", lprocess); // Simulate working
                    lcompletion = lcompletion ^ 10; // Updates Completion maker by using XOR operation
                    lpriority += starvation_aging(lprocess, lpriority); // Updates local priority value 
                    sem_post(resource_B); // Release Semaphore
                }
            }
            if ((lcompletion & 1) != 1) { // Completion checker by using AND operation
                if (sem_trywait(resource_C) == 0 ) { // Trying to get access to resource semaphore
                    doing_something("C.txt", lprocess); // Simulate working
                    lcompletion = lcompletion ^ 1; // Updates Completion maker by using XOR operation
                    lpriority += starvation_aging(lprocess, lpriority); // Updates local priority value 
                    sem_post(resource_C); // Release Semaphore
                }          
            }
        }
    }

    //sem_post(priority); // Updates priority value only when proccess X finish all task *artifical condition that causes starvation*

    gettimeofday(&end, 0); // Get current timeofday to end timer
    long seconds = end.tv_sec - begin.tv_sec; // Calculate seconds elasped
    long microseconds = end.tv_usec - begin.tv_usec; // Calculate microseconds elasped
    double elapsed = seconds + microseconds*1e-6; // Calculate total elapsed time 
    printf("Process %d: All Task Completed in %0.2f Seconds!\n", lprocess, elapsed); // Finished all task 

    return(0);
}



