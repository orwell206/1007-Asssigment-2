#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define NUM_OF_PROCESSES 5
#define NUM_OF_RESOURCES 3
#define WRITE_COUNT 5

// Binary Semaphores
sem_t jar1lock, jar2lock, jar3lock;


void createFiles(){
    /* ********************************************************************** */
    /* Function that will create files if does not exist, else truncate to 0 */
    /* ********************************************************************** */
    FILE *temp;
    int initial_cookie_count = 0;

    // Ensure files are recreated from scratch
    for (int i = 0; i < NUM_OF_RESOURCES; i++){
        char filename[10];
        // Get name of file to create
        snprintf(filename, 10, "jar%d.txt", i+1);
        // Open file for writing (if exist, overwrite, if does not exist, create and write)
        temp = fopen(filename, "w+");
        // Write the cookie count (default initial value == 0) into the file
        fprintf(temp, "%d", initial_cookie_count);
        // Close the file stream
        fclose(temp);
    }
}

void openFileAndModify(int jar_count){
    /* ********************************************************* */
    /* Function that increment file contents 'WRITE_COUNT' times */
    /* ********************************************************* */

    // Declare temp file stream variable
    FILE *jar;
    
    // Init var to store number of cookies in each file
    int cookie_count = 0;
    
    // Get name of file to open on this iteration
    char file_to_open[10];
    snprintf(file_to_open, 10, "jar%d.txt", jar_count);

    // Then, write n times, where n = WRITE_COUNT constant
    for (int i = 0; i < WRITE_COUNT; i++){
        // First open the file for reading
        jar = fopen(file_to_open, "r");
        // Then get the current value of cookies in the file
        fscanf(jar, "%d", &cookie_count);
        // After reading value, close the file stream
        fclose(jar);

        // Then, open the file for writing
        jar = fopen(file_to_open, "w+");
        // Write incremented value of current cookie count into file
        cookie_count += 1;
        fprintf(jar, "%d", cookie_count);
        // After writing the new value, close the file stream
        fclose(jar);
    }
}

void* race_c(void* thread_id){
    /* ************************************************************************* */
    /* Worker thread function that calls 'openFileAndModify()' for all resources */
    /* ************************************************************************* */

    int *tid = thread_id;

    // For each file, acquire the corresponding lock
    // for (int jar_count = 0; jar_count < NUM_OF_RESOURCES; jar_count++){
    
    // Acquire lock for this jar 1
    sem_wait(&jar1lock);
    printf("[+] Bear %d acquired lock for jar 1\n", *tid);
    
    /********************/
    /* Critical Section */
    /********************/
    openFileAndModify(1);

    // Release the lock for this jar (1 since we have written to it WRITE_COUNT times
    printf("[-] Bear %d released lock for jar 1\n", *tid);
    sem_post(&jar1lock);

    // Acquire lock for jar 2
    sem_wait(&jar2lock);
    printf("[+] Bear %d acquired lock for jar 2\n", *tid);
    
    /********************/
    /* Critical Section */
    /********************/
    openFileAndModify(2);

    // Release the lock for jar 2 since we have written to it WRITE_COUNT times
    printf("[-] Bear %d released lock for jar 2\n", *tid);
    sem_post(&jar2lock);
    
    // Acquire lock for this jar 3
    sem_wait(&jar3lock);
    printf("[+] Bear %d acquired lock for jar 3\n", *tid);
    
    /********************/
    /* Critical Section */
    /********************/
    openFileAndModify(3);

    // Release the lock for this jar 3 since we have written to it WRITE_COUNT times
    printf("[-] Bear %d released lock for jar 3\n", *tid);
    sem_post(&jar3lock);

    // }
}

void* race_c2(void* locks){
    /* ************************************************************************* */
    /* Worker thread function that calls 'openFileAndModify()' for all resources */
    /* ************************************************************************* */

    sem_t *jarlocks = locks;
    pid_t x = gettid();

    // For each file, acquire the corresponding lock
    for (int i = 0; i < NUM_OF_RESOURCES; i++){
        
        // Acquire lock for this jar 1
        sem_wait(&jarlocks[i]);
        printf("[+] Bear %d acquired lock for jar %d\n", x, i+1);
        
        /********************/
        /* Critical Section */
        /********************/
        openFileAndModify(i+1);

        // Release the lock for this jar (1 since we have written to it WRITE_COUNT times
        printf("[-] Bear %d released lock for jar %d\n", x, i+1);
        sem_post(&jarlocks[i]);
    }
}

int main()
{
    // Ensure NUM_OF_RESOURCES * jar files are created
    createFiles();

    // Init all 3 locks for each file
    sem_init(&jar1lock, 0, 1);
    sem_init(&jar2lock, 0, 1);
    sem_init(&jar3lock, 0, 1);

    // Spawn worker threads
    // int *thread_ids = malloc(sizeof(int)*NUM_OF_PROCESSES);

    // for (int i = 0; i < NUM_OF_PROCESSES; i++){
    //     thread_ids[i] = i+1;
    // }

    sem_t locks[NUM_OF_RESOURCES] = {jar1lock, jar2lock, jar3lock};

    // Declare the 5 processes (worker threads)
    pthread_t bear1, bear2, bear3, bear4, bear5;
    

    // for(int i=0; i<3; i++){
    // pthread_create(&bear1, NULL, race_c2, &thread_ids[0]);
    // pthread_create(&bear2, NULL, race_c2, &thread_ids[1]);
    // pthread_create(&bear3, NULL, race_c2, &thread_ids[2]);
    // pthread_create(&bear4, NULL, race_c2, &thread_ids[3]);
    // pthread_create(&bear5, NULL, race_c2, &thread_ids[4]);
    pthread_create(&bear1, NULL, race_c2, &locks);
    pthread_create(&bear2, NULL, race_c2, &locks);
    pthread_create(&bear3, NULL, race_c2, &locks);
    pthread_create(&bear4, NULL, race_c2, &locks);
    pthread_create(&bear5, NULL, race_c2, &locks);
    pthread_join(bear1, NULL);
    pthread_join(bear2, NULL);
    pthread_join(bear3, NULL);
    pthread_join(bear4, NULL);
    pthread_join(bear5, NULL);
    // }
    sem_destroy(&jar1lock);
    sem_destroy(&jar2lock);
    sem_destroy(&jar3lock);
    // free(thread_ids);

    // for(int i=0; i<3; i++){
    //     pthread_create(&bear1, NULL, sol_rc, NULL);
    //     pthread_create(&bear2, NULL, sol_rc, NULL);
    //     pthread_create(&bear3, NULL, sol_rc, NULL);
    //     pthread_create(&bear4, NULL, sol_rc, NULL);
    //     pthread_create(&bear5, NULL, sol_rc, NULL);
    //     pthread_join(bear1, NULL);
    //     pthread_join(bear2, NULL);
    //     pthread_join(bear3, NULL);
    //     pthread_join(bear4, NULL);
    //     pthread_join(bear5, NULL);
    // }
    
    return 0;
}

