#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"


#define MAX_RAD 1000
#define MAX_THREADS 50


//Returns the start y for given thread
int getYStart(const int threadId, const int numThreads, const int ysize) {
  return threadId * (ysize / numThreads);
}

//Returns the end y for given thread
int getYEnd(const int threadId, const int numThreads, const int ysize) {
  int yEnd = (threadId + 1) * (ysize / numThreads);

  //Add the leftoverpixels to the last thread
  if(threadId == numThreads - 1)
    yEnd += ysize % numThreads;

  return yEnd;
}




int main (int argc, char ** argv) {
	int radius;
    int xsize, ysize, colmax;
    struct timespec stime, etime;
    double w[MAX_RAD];

    pixel* src = new pixel[MAX_PIXELS];
    pixel* dst = new pixel[MAX_PIXELS];

    /* Take care of the arguments */

	if (argc != 5) {
		fprintf(stderr, "Usage: %s radius cores infile outfile\n", argv[0]);
		exit(1);
	}
	radius = atoi(argv[1]);
	if((radius > MAX_RAD) || (radius < 1)) {
		fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
		exit(1);
	}

	/* read file */
	if(read_ppm (argv[3], &xsize, &ysize, &colmax, (char *) src) != 0)
	    exit(1);

	if (colmax > 255) {
		fprintf(stderr, "Too large maximum color-component value\n");
		exit(1);
	}

	const int numThreads = atoi(argv[2]);
	if (numThreads < 1) {
		fprintf(stderr, "Number of threads needs to be higher than zero\n");
	}

	printf("Has read the image, generating coefficients\n");

    /* filter */
    get_gauss_weights(radius, w);

    printf("Calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);

    // pthreads stuff ////

    pthread_t threads[MAX_THREADS];
    struct threadDataBlurfilter threadData[MAX_THREADS];

    // Start blurfilterX
    for (int i = 0; i < numThreads; ++i) {
    	threadData[i].threadId = i;
    	threadData[i].xsize = xsize;
    	threadData[i].ysize = ysize;
    	threadData[i].src = src;
    	threadData[i].dst = dst;
    	threadData[i].radius = radius;
    	threadData[i].w = w;
    	threadData[i].yStart = getYStart(i, numThreads, ysize);
    	threadData[i].yEnd = getYEnd(i, numThreads, ysize);

    	int rc = pthread_create(&threads[i], NULL, blurfilterX, (void*)&threadData[i]);
    	if (rc) {
    		fprintf(stderr, "Error creating thread %d", rc);
    		exit(1);
    	}
    }

    // Waiting until all threads are finished
    void* status;
    for (int i = 0; i < numThreads; ++i) {
    	int rc = pthread_join(threads[i], &status);
    	if (rc) {
    		fprintf(stderr, "Error in joining thread %d", rc);
    		exit(1);
    	}
    }


    // Start blurfilterY
    for (int i = 0; i < numThreads; ++i) {
    	threadData[i].threadId = i;
    	threadData[i].xsize = xsize;
    	threadData[i].ysize = ysize;
    	threadData[i].src = src;
    	threadData[i].dst = dst;
    	threadData[i].radius = radius;
    	threadData[i].w = w;
    	threadData[i].yStart = getYStart(i, numThreads, ysize);
    	threadData[i].yEnd = getYEnd(i, numThreads, ysize);

    	int rc = pthread_create(&threads[i], NULL, blurfilterY, (void*)&threadData[i]);
    	if (rc) {
    		fprintf(stderr, "Error creating thread %d", rc);
    		exit(1);
    	}
    }

    // Waiting until all threads are finished
    for (int i = 0; i < numThreads; ++i) {
    	int rc = pthread_join(threads[i], &status);
    	if (rc) {
    		fprintf(stderr, "Error in joining thread %d", rc);
    		exit(1);
    	}
    }

    //////////////////////

    clock_gettime(CLOCK_REALTIME, &etime);
	
    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

	/* results */
	printf("Writing output file\n");
		
	if(write_ppm (argv[4], xsize, ysize, (char *)src) != 0)
		  exit(1);

	// Delete to avoid memory leak
	delete[] src;
	delete[] dst;
	pthread_exit(NULL);
    return(0);
}
