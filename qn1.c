#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function that returns a command by searching a given buffer based on specified given pid
char *getCommandByPID(int pid, int argc, char *arg_list[], int cpids[]){
    for (int i = 0; i < argc; i++){
        if (cpids[i] == pid)
            return arg_list[i];
    }
    return NULL;
}

int main(int argc, char *argv[]){
    // Init variables used to store command information
    char *argument_list[argc];
    int childpid_list[argc];

    //===========================//
    /* Loop through command list */
    //===========================//
    for (int i = 1; i < argc; i++){
        // Init variables for this iteration
        pid_t pid;          // stores return value by fork() command

        //======================//
        /* Fork a child process */
        //======================//

        // If unable to fork()
        if ((pid = fork()) == -1){
            printf("[!] Unable to spawn child process to run command...");
            return EXIT_FAILURE;
        }
        // Or if is child
        else if (pid == 0)
        {
            // Try to run the current iteration's command
            if (execl(argv[i], argv[i], (char *)NULL) == -1){
                perror(argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        // Or if is parent
        else{
            // Track the pid, and command executed
            childpid_list[i] = pid;
            argument_list[i] = argv[i];
        }
    }

    //=============================//
    /* Make parent wait for childs */
    //=============================//

    // Tracks the number of completed processes
    int counter = 0;
    // Buffer for storing return exit code of child after 'waitpid()' catches it
    int exit_code;    

    while (counter < (argc -1)){
        // Call waitpid() function to wait for ANY child to exit
        int childpid = waitpid(-1, &exit_code, 0);
        char *command = getCommandByPID(childpid, argc, argument_list, childpid_list);
        
        if (exit_code == 0)
            printf("\nCommand %s has completed successfully", command);
        else
            printf("\nCommand %s has not completed successfully", command);
        counter++;
    }
    printf("\nAll done, bye!\n");
    return EXIT_SUCCESS;
}