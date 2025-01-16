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

int main(int argc, char **argv){

    if ( argc != 5){
        printf("Wrong parameters\n"); exit(0);
    }

    clock_t start,end,mid;
    double cpu_time_used,mid_time;

    start = clock();

    char datafile[30];
    strcpy(datafile, argv[1]);

    int from,to,i;
    to = -1;
    char* token;
    token = strtok(argv[2],",");
    if ( token != NULL){
        from = atoi(token);
        token = strtok(NULL,",");
        if ( token != NULL ){
            to = atoi(token);
        }
    }

    if ( to == -1 ) // just one record
        to = from;


    int time = atoi(argv[3]);
    int shm_id = atoi(argv[4]);

    FILE *fpb;
    MyRecord rec;
    fpb = fopen (argv[1],"rb+");
    if (fpb==NULL) {
      	printf("Cannot open binary file\n");
      	return 1;
   	}

    struct shared_memory* sh;
    sh = (struct shared_memory *) shmat(shm_id, NULL, 0);
    if (sh == NULL) {
        perror("Shared memory attach "); exit(EXIT_FAILURE);
    }


    sem_wait(&(sh->queue)); // go or wait for others to finish, this ensures that there is no starvation
    sem_wait(&(sh->read)); // lock readers to change counter
    sh->readers++;
    if ( sh->readers == 1) // first reader
        sem_wait(&(sh->lock)); // no writers
    sem_post(&(sh->queue));
    sem_post(&(sh->read));


    // cs
    mid = clock(); // for the stats
    sleep(time);

    for (i=from; i<=to; i++){
        fseek(fpb, (i-1)*sizeof(MyRecord),SEEK_SET);
        fread(&rec, sizeof(rec), 1, fpb);
        printf("Reader with pid %d read: %d %s %s  %d \n",getpid(), rec.custid, rec.LastName, rec.FirstName, rec.balance);
    }


    sem_wait(&(sh->read)); // lock readers to change counter
    sh->readers--;
    if ( sh->readers == 0) // last reader
        sem_post(&(sh->lock)); // allow writers
    sem_post(&(sh->read));
    // end


    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    mid_time = ((double) (mid - start)) / CLOCKS_PER_SEC;


    sem_wait(&(sh->tsem)); // only one reader can change these at any time
    sh->total_readers++;
    sh->total_all += to - from;
    sh->readers_time += cpu_time_used;
    if ( mid_time > sh->max_time )
         sh->max_time = mid_time; // max delay
    sem_post(&(sh->tsem));


    int err = shmdt((void *) sh);
	if (err == -1) {
		perror ("Detachment.");
	}

    fclose(fpb);

    //printf("READERRR\n");

    return 0;
}