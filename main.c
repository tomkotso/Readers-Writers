#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/times.h> 
#include <semaphore.h>
#include <time.h>
#include "util.h"
#define TOTAL_PROCESSES 50

int main(int argc, char** argv){

    if ( argc != 2){
        printf("Wrong parameters\n"); exit(0);
    }

    int i,pid,shm_id,recid;

    char datafile[30];
    strcpy(datafile,argv[1]);
    int total_records = num_of_records(datafile);
    struct shared_memory* sh;

    // create shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), IPC_CREAT | 0660);
    if (shm_id == -1) {
        perror("Shared memory creation"); exit(EXIT_FAILURE);
    }
    // attach shared memory
    sh = (struct shared_memory *) shmat(shm_id, NULL, 0);
    if (sh == NULL) {
        perror("Shared memory attach "); exit(EXIT_FAILURE);
    }


    int retval = sem_init ( &(sh->tsem), 1, 1);
    if ( retval != 0) {
        perror (" Couldn 't␣ initialize ." ); exit(3);
    }

    sh->readers = 0;
    sh->total_readers = 0;
    sh->total_writers = 0;
    sh->total_all = 0;
    sem_init ( &(sh->read), 1, 1);
    sem_init ( &(sh->queue), 1, 1);
    sem_init ( &(sh->lock), 1, 1);
    sh->readers_time = 0;
    sh->writers_time = 0;
    sh->max_time = 0;

    int sleep_time,to;
    srand(time(NULL));
    char id[10],line[10],sleeptime[5];

    for(i=0; i<TOTAL_PROCESSES; i++){

        if ( (pid = fork()) == -1 ){
            perror("fork\n"); exit(1);
        }

        recid = rand() % total_records;
        to = recid + 15;
        if (to > total_records)
            to = total_records; // do not exceed 
        sleep_time = rand() % 2; // max 5 sec
        recid++;

        if ( pid == 0 ){ // child

            sprintf(id,"%d",shm_id);
            sprintf(sleeptime,"%d",sleep_time);

            if ( (i+1) % 4 == 0 ){ // one writer for three readers

                sprintf(line,"%d",recid);
                execlp("./writer", "writer", datafile, line, "25", sleeptime, id, NULL);
            }
            else{
                sprintf(line,"%d,%d",recid,to);
                execlp("./reader", "reader", datafile, line, sleeptime, id, NULL);
            }
            printf("rei\n");
            exit(0);
        }


    }


    for(i=0; i<TOTAL_PROCESSES; i++)
        wait(NULL);


    printf("Total readers %d\n",sh->total_readers);
    printf("Total writers %d\n",sh->total_writers);
    printf("Total records %d\n",sh->total_all);
    printf("Average readers time %f\n",sh->readers_time/sh->total_readers);
    printf("Average writers time %f\n",sh->writers_time/sh->total_writers);
    printf("Max delay %f\n",sh->max_time);


    // detach and delete shared memory
    int err = shmdt((void *) sh);
	if (err == -1) {
		perror ("Detachment.");
	}

    if (shmctl(shm_id, IPC_RMID, (struct shmid_ds *)0 ) <0) {
		perror("semctl"); exit(1);
	}

    sem_destroy(&(sh->tsem));
    sem_destroy(&(sh->read));
    sem_destroy(&(sh->queue));
    sem_destroy(&(sh->lock));

    return 0;
}