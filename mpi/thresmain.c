#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <mpi.h>
#include "ppmio.h"


using namespace std;

#define uint unsigned int

struct pixel
{
  unsigned char r,g,b;
};

//Calculate the sum of all pixels in src
uint getSum(const int xsize, const int ysize, pixel* src){
  uint sum, i, nump;
  nump = xsize * ysize;
  
  for(i = 0, sum = 0; i < nump; i++)
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
   
  return sum;
}


//Init the custom MPI_Type for sending
void initPixel(MPI_Datatype& pixelType)
{
  const int structSize = 3;
  int blockcounts[] = {1, 1, 1};
  MPI_Aint disp[] = {0, 0, 0};
  MPI_Datatype oldTypes[structSize] = { MPI::UNSIGNED_CHAR,
                    MPI::UNSIGNED_CHAR,
                    MPI::UNSIGNED_CHAR };

  MPI_Type_struct(structSize, blockcounts, disp, oldTypes, &pixelType);
  MPI_Type_commit(&pixelType);
}


void thresfilter(const int xsize, const int ysize, pixel* src, const int sum){  
  uint pSum, numP, i;
  numP = xsize * ysize;

  for(i = 0; i < numP; i++) {
    pSum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
    if(sum > pSum) {
      src[i].r = src[i].g = src[i].b = 0;
    }
    else {
      src[i].r = src[i].g = src[i].b = 255;
    }

  }
}


int main (int argc, char ** argv) {
    MPI_Status status;
    MPI_Init (&argc, &argv);
    const int root = 0;
    const int myid = MPI::COMM_WORLD.Get_rank(); // Rank of processes
    const int numProc = MPI::COMM_WORLD.Get_size(); // N.o. processes

    MPI_Datatype pixelType;
    initPixel(pixelType);
    const int pixelTypeSize = 3;

    int xsize, ysize, colmax;
    pixel *src;
    struct timespec stime, etime;

    /* Take care of the arguments */
    if (myid == root) {
        src = new pixel[MAX_PIXELS];

        if (argc != 3) {
     fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
     exit(1);
    }

    /* read file */
    if(read_ppm (argv[1], &xsize, &ysize, &colmax, (char *) src) != 0)
        exit(1);

    if (colmax > 255) {
     fprintf(stderr, "Too large maximum color-component value\n");
     exit(1);
    }
    
    
    const int numThreads = 2;
    if(numThreads < 1){
      printf("Number of threads need to be higher than zero\n");
      return 4;
    }
    
    
    printf("Has read the image, calling filter\n");

    }
    
    if (myid == root)
      clock_gettime(CLOCK_REALTIME, &stime);
    
    // FILTERING
    //Broadcast the variables from root to the other processes
    MPI_Bcast(&xsize, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&ysize, 1, MPI_INT, root, MPI_COMM_WORLD);

    int localY = ysize/numProc;
    //The last process gets the leftover rows
    if(myid == numProc - 1)
      localY += ysize % numProc;

    pixel *localSrc = new pixel[localY*xsize];

    //Calculate the send information needed by MPI_Scatterv
    int sendCount[numProc];
    int disp[numProc];
    int dispSize = 0;
    for (int i=0; i<numProc; i++){
      if(i == numProc - 1)
        sendCount[i] = ((ysize/numProc) + (ysize%numProc))*(xsize*pixelTypeSize);
      else
        sendCount[i] = (ysize/numProc)*xsize*pixelTypeSize;
      
      disp[i] = dispSize;
      disp_size += sendCount[i];
    }

    //Distribute the work
    MPI_Scatterv(src, sendCount, disp, pixelType,
           localSrc, sendCount[myid], pixelType, root, MPI_COMM_WORLD);
      
    //Calculate the sum
    double localSum = (double)getSum(xsize, localY, localSrc);
    double sum = 0;

    //Accumulate all the local sums
    MPI_Allreduce(&localSum, &sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    sum = sum/(xsize*ysize);

    thresfilter(xsize, localY, localSrc, sum);

    MPI_Gatherv(localSrc, sendCount[myid], pixelType, src,
          sendCount, disp, pixelType, root, MPI_COMM_WORLD);

//////////////////////////////////////////

    if (myid == root) {
      clock_gettime(CLOCK_REALTIME, &etime);

      printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
  	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

      /* write result */
      printf("Writing output file\n");
      
      if(write_ppm (argv[2], xsize, ysize, (char *)src) != 0)
        exit(1);

      delete[] src;
    }

    delete[] localSrc
    MPI_Finalize();
    return(0);
}
