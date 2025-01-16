#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "util.h"

int num_of_records(char datafile[30]){
    FILE *fpb;
    MyRecord rec;
    long lSize;
    int numOfrecords;

    fpb = fopen (datafile,"rb+");
    if (fpb==NULL) {
      	printf(" Cannot open binary file\n");
      	return 1;
   	}

   // check number of records
   fseek (fpb , 0 , SEEK_END);
   lSize = ftell (fpb);
   rewind (fpb);
   numOfrecords = (int) lSize/sizeof(rec);
   return numOfrecords;
}
