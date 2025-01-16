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

    if ( argc != 6){
        printf("Wrong parameters\n"); exit(0);
    }

    clock_t start,end,mid;
    double cpu_time_used,mid_time;

    start = clock();

    char datafile[30];
    strcpy(datafile, argv[1]);
    int line = atoi(argv[2]);
    int value = atoi(argv[3]);
    int time = atoi(argv[4]);
    int shm_id = atoi(argv[5]);

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
    sem_wait(&(sh->lock)); // succeds only when 0 readers are active
    sem_post(&(sh->queue)); // let the next one in

    // cs

    mid = clock(); // for the stats


    sleep(time);

    // find record
    fseek(fpb, (line-1)*sizeof(MyRecord),SEEK_SET);
    fread(&rec, sizeof(rec), 1, fpb);
    
    rec.balance += value;

    // write record
    fseek(fpb, (line-1)*sizeof(MyRecord),SEEK_SET);
    fwrite(&rec,sizeof(MyRecord),1,fpb);


    printf("\nWriter with pid %d changed: %d %s %s  %d \n\n",getpid(), rec.custid, rec.LastName, rec.FirstName, rec.balance);

    // exit cs
    sem_post(&(sh->lock)); // allow readers or writers in
    // end


    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    mid_time = ((double) (mid - start)) / CLOCKS_PER_SEC;

    sem_wait(&(sh->tsem));
    sh->total_writers++;
    sh->total_all++; // just one writer everytime
    sh->writers_time += cpu_time_used;
    if ( mid_time > sh->max_time )
        sh->max_time = mid_time;
    sem_post(&(sh->tsem));


    int err = shmdt((void *) sh);
	if (err == -1) {
		perror ("Detachment.");
	}

    fclose(fpb);

    //printf("WRITERRRR\n");

    return 0;
}