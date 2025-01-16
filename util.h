#pragma once
#include <semaphore.h>
#define SIZEofBUFF 20

struct shared_memory{

    int total_readers;
    int total_writers;
    int total_all;
    int readers;
    sem_t read;
    sem_t queue;
    sem_t lock;
    sem_t tsem;
    double readers_time;
    double writers_time;
    double max_time;


};

typedef struct{
	int  	custid;
	char 	LastName[SIZEofBUFF];
	char 	FirstName[SIZEofBUFF];
	int	balance;
} MyRecord;

int num_of_records(char datafile[30]);