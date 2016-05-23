#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include "ppmio.h"
#include "thresfilter.h"

#define MAX_THREADS 50

//Returns the start y for given thread
int getYStart(const int threadId, const int numThreads, const int ysize) {
  return threadId * (ysize/numThreads);
}

//Returns the end y for given thread
int getYEnd(const int threadId, const int numThreads, const int ysize) {
  int yEnd = (threadId + 1) * (ysize/numThreads);
  //Add the leftoverpixels to the last thread
  if(threadId == numThreads - 1)
    yEnd += ysize % numThreads;

  return yEnd;
}


int main (int argc, char ** argv) {
    int xsize, ysize, colmax;
    pixel* src = new pixel[MAX_PIXELS];
    struct timespec stime, etime;

    /* Take care of the arguments */

    if (argc != 4) {
    	fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
    	exit(1);
    }

    /* read file */
    if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
        exit(1);

    if (colmax > 255) {
    	fprintf(stderr, "Too large maximum color-component value\n");
    	exit(1);
    }
    
    
    const int numThreads = atoi(argv[1]);
    if(numThreads < 1){
      printf("Number of threads need to be higher than zero\n");
      exit(1);
    }
    
    
    printf("Has read the image, calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);
    
    pthread_t threads[MAX_THREADS];
    struct threadDataThresfilter threadData[MAX_THREADS];
    
    //Start the threads
    for(int i = 0; i < numThreads; i++){
      threadData[i].threadId = i;
      threadData[i].xsize = xsize;


      //Set the y-interval to be processed by the thread
      const int yStart = getYStart(i, numThreads, ysize);
      threadData[i].src = src;
      threadData[i].ysize = getYEnd(i, numThreads, ysize) - yStart;
    
      int rc = pthread_create(&threads[i], NULL, thresfilter, (void*)&threadData[i]);
      if(rc){
      	printf("Error creating thread, %d", rc);
      	exit(1);
      } 
    }

    //Wait for all threads to finish
    void* status;
    for(int i = 0; i < numThreads; i++){
      int rc = pthread_join(threads[i], &status);
      if(rc) {
      	printf("Error in thread join: %d", rc);
      	exit(1);
      }
    }

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
      exit(1);

    delete[] src;
    return(0);
}
