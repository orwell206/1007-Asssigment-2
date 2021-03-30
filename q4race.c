#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h> 
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define NUM_OF_PROCESSES 5
#define NUM_OF_RESOURCES 3
#define WRITE_COUNT 5
#define SHSIZE 1024

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

void* race_c2(){
    /* ************************************************************************* */
    /* Worker thread function that calls 'openFileAndModify()' for all resources */
    /* ************************************************************************* */

    // sem_t locks1;
    pid_t x = syscall(SYS_gettid);

    // For each file, acquire the corresponding lock
    for (int i = 0; i < NUM_OF_RESOURCES; i++){
        
        // Acquire lock for this jar 1
        // sem_wait(&locks1);
        printf("[+] Bear %d acquired lock for jar %d\n", x, i+1);
        
        /********************/
        /* Critical Section */
        /********************/
        openFileAndModify(i+1);

        // Release the lock for this jar (1 since we have written to it WRITE_COUNT times
        printf("[-] Bear %d released lock for jar %d\n", x, i+1);
        // sem_post(&locks1);
    }
}

int main()
{
    // Ensure NUM_OF_RESOURCES * jar files are created
    createFiles();
    // sem_t jarlock;
    sem_t *jarlock = mmap(NULL,sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Initialise lock.
    sem_init(jarlock, 0, 1);

    // counter variable 
    int loopctr = 0;
    int resource = NUM_OF_RESOURCES;

    // Create parent and child process
    pid_t bear1, bear2, bear3;
    bear1 = fork();
    bear2 = fork();
    bear3 = fork();
    
    if(bear1 > 0 && bear2 > 0 && bear3 > 0){
        while(resource > 0){
            // sem_wait(jarlock);
            race_c2();
            loopctr++;
            resource -= 1; 
            // sem_post(jarlock);
        }
        printf("Bear 1 accessed the jar %d times.\n", loopctr);
    }
    else if(bear1 > 0 && bear2 > 0 && bear3 == 0){
        while(resource > 0){
            // sem_wait(jarlock);
            race_c2();
            loopctr++;
            resource -= 1;
            // sem_post(jarlock);
        }
        printf("Bear 2 accessed the jar %d times.\n", loopctr);
    }
    else if(bear1 > 0 && bear2 == 0 && bear3 > 0){
        while(resource > 0){
            // sem_wait(jarlock);
            race_c2();
            loopctr++;
            resource -= 1;
            // sem_post(jarlock);
        }
        printf("Bear 3 accessed the jar %d times.\n", loopctr);
    }
    else if(bear1 > 0 && bear2 == 0 && bear3 == 0){
        while(resource > 0){
            // sem_wait(jarlock);
            race_c2();
            loopctr++;
            resource -= 1;
            // sem_post(jarlock);
        }
        printf("Bear 4 accessed the jar %d times.\n", loopctr);
    }
    else if(bear1 == 0 && bear2 > 0 && bear3 > 0){
        while(resource > 0){
            // sem_wait(jarlock);
            race_c2();
            loopctr++;
            resource -= 1;
            // sem_post(jarlock);
        }
        printf("Bear 5 accessed the jar %d times.\n", loopctr);
    }
    
    return 0;
}