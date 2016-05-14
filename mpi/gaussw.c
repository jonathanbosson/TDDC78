/*
  File gaussw.c

  Implementation of get_gauss_weights function.

 */
#include <math.h>
#include "mpi.h"

#define MAX_X 1.33
#define Pi 3.14159

/* Generate an array of weights for the gaussian filter. */
/* Input: n - number of weights to generate              */
/* Output: weights_out - array of weights. The element [0] */
/*  should be used for the central pixel, elements [1..n] */
/*  should be used for the pixels on a distance [1..n]  */  
void get_gauss_weights(int n, double* weightsOut) {
	double x;
	int i;
	const int root = 0; 
	const int myid = MPI::COMM_WORLD.Get_rank(); // Rank of processes
  	const int numProc = MPI::COMM_WORLD.Get_size(); // N.o. processes

	int workPerProc = n / numProc;
	// Last process handles rest after last index as well
	if (myid == numProc - 1) {
		workPerProc += n % numProc;
	}
	
	// Work buffer of active process
	double procWeight[workPerProc];
	// Where in n the active process is in 
	const int start = myid*workPerProc;
	// Number of elements received
	int recvCount[numProc];
	// Displacement relative to where in weightsOut to place the procWeight
	int displace[numProc]; 
	
	for(i = start; i < start+workPerProc; i++) {
		x = (double)i * MAX_X/n;
		procWeight[i-start] = exp(-x*x * Pi);
	}

	// Determine amount of data to be received from each process
	for (i = 0; i < numProc; i++) {
		recvCount[i] = n / numProc;
		if (i == numProc-1)
			recvCount[i] += n % numProc;
		displace[i] = i * (n / numProc);
	}
	// Gather all weightProc and save it to weightOut
	MPI_Allgatherv(&procWeight, workPerProc, MPI_DOUBLE, weightsOut, recvCount, displace, MPI_DOUBLE, MPI_COMM_WORLD);
}