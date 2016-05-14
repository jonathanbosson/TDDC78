#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"


int main (int argc, char ** argv) {
	MPI_Status status;
	MPI_Init(&argc, &argv);
	const int root = 0; //root process
  	const int myid = MPI::COMM_WORLD.Get_rank(); // Rank of processes
  	const int numProc = MPI::COMM_WORLD.Get_size(); // N.o. processes

	#define MAX_RAD 1000
	int radius;
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;

    double w[MAX_RAD];

    /* Take care of the arguments */
	if( myid == root) {
		if (argc != 4) {
			fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
			exit(1);
		}
		radius = atoi(argv[1]);
		if((radius > MAX_RAD) || (radius < 1)) {
			fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
			exit(1);
		}

		/* read file */
		if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
		    exit(1);

		if (colmax > 255) {
			fprintf(stderr, "Too large maximum color-component value\n");
			exit(1);
		}

		printf("Has read the image, generating coefficients\n");
	}

	// Broadcast radius to all processes
	MPI_Bcast(&radius, 1, MPI_INT, root, MPI_COMM_WORLD);

    /* filter */
    get_gauss_weights(radius, w);

	// Broadcast dimension of image
	MPI_Bcast(&xsize, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&ysize, 1, MPI_INT, root, MPI_COMM_WORLD);

    printf("Calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);

    blurfilter(xsize, ysize, src, radius, w);

    clock_gettime(CLOCK_REALTIME, &etime);
	if (myid == root){
		printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
		   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

		/* write result */
		printf("Writing output file\n");
		
		if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
		  exit(1);
	}

	MPI_Finalize();
    return(0);
}
