#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

//Semaphore
sem_t cookie_lock;

void* race_c(void* in)
{
    FILE*jar1, *jar2, *jar3;
    int cookie_count = 0;
    sem_wait(&cookie_lock);
    for (int i = 0; i <5; i++ ){
        printf("Open Cookie Jar 1.\n");
        //If File for Jar 1 does not exist, create it.
        jar1 = fopen("jar1.txt","r");
        if(jar1 == NULL){
            jar1 = fopen("jar1.txt", "w");
        }
        else{
            //Scan current value in Jar 1, store current cookie count.
            fscanf(jar1,"%d",&cookie_count);
            fclose(jar1);
            //Add to cookie count.
            cookie_count++;
            //Write new cookie count to file.
            jar1 = fopen("jar1.txt","w");
            fprintf(jar1,"%d",cookie_count);
            //Close cookie jar 1.
            fclose(jar1);
        }
    }
    for (int i = 0; i <5; i++ ){
        printf("Open Cookie Jar 2.\n");
        //If File for Jar 2 does not exist, create it.
        jar2 = fopen("jar2.txt","r");
        if(jar2 == NULL){
            jar2 = fopen("jar2.txt", "w");
        }
        else{
            //Scan current value in Jar 2, store current cookie count.
            fscanf(jar2,"%d",&cookie_count);
            fclose(jar2);
            //Add to cookie count.
            cookie_count++;
            //Write new cookie count to file.
            jar2 = fopen("jar2.txt","w");
            fprintf(jar2,"%d",cookie_count);
            //Close cookie jar 2.
            fclose(jar2);
        }
    }
    for (int i = 0; i <5; i++ ){
        printf("Open Cookie Jar 3.\n");
        //If File for Jar 3 does not exist, create it.
        jar3 = fopen("jar3.txt","r");
        if(jar3 == NULL){
            jar3 = fopen("jar3.txt", "w");
        }
        else{
            //Scan current value in Jar 3, store current cookie count.
            fscanf(jar3,"%d",&cookie_count);
            fclose(jar3);
            //Add to cookie count.
            cookie_count++;
            //Write new cookie count to file.
            jar3 = fopen("jar3.txt","w");
            fprintf(jar3,"%d",cookie_count);
            //Close cookie jar 3.
            fclose(jar3);
        }
    }
    sem_post(&cookie_lock);
    sem_destroy(&cookie_lock);
}

int main()
{
    pthread_t bear1, bear2, bear3, bear4, bear5;
    sem_init(&cookie_lock, 0,1);
    for(int i=0; i<3; i++){
        pthread_create(&bear1, NULL, race_c, NULL);
        pthread_create(&bear2, NULL, race_c, NULL);
        pthread_create(&bear3, NULL, race_c, NULL);
        pthread_create(&bear4, NULL, race_c, NULL);
        pthread_create(&bear5, NULL, race_c, NULL);
        pthread_join(bear1, NULL);
        pthread_join(bear2, NULL);
        pthread_join(bear3, NULL);
        pthread_join(bear4, NULL);
        pthread_join(bear5, NULL);
    }

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

