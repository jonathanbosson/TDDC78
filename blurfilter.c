/*
  File: blurfilter.c

  Implementation of blurfilter function.
    
 */
#include <stdio.h>
#include "blurfilter.h"
#include "ppmio.h"
#include "mpi.h"


pixel* pix(pixel* image, const int xx, const int yy, const int xsize)
{
  register int off = xsize*yy + xx;

#ifdef DBG
  if(off >= MAX_PIXELS) {
    fprintf(stderr, "\n Terribly wrong: %d %d %d\n",xx,yy,xsize);
  }
#endif
  return (image + off);
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

void blurfilter(const int xsize, const int ysize, pixel* src, const int radius, const double *w){
	MPI_Status status;
	const int root = 0; //root process
  	const int myid = MPI::COMM_WORLD.Get_rank(); // Rank of processes
  	const int numProc = MPI::COMM_WORLD.Get_size(); // N.o. processes

	MPI_Datatype pixelType;
	initPixel(pixelType);
	const int pixelTypeSize = 3;

	int x,y,x2,y2, wi;
	double r,g,b,n, wc;
	
	int rowInt = ysize / numProc;
	if (myid == numProc - 1)
		rowInt += ysize % numProc;	

	const int rowStart = rowInt*myid;
	const int localSize = rowInt*xsize;
	const int localRad = radius*xsize;

	pixel* localSrc = new pixel[localSize];
	pixel* localDist = new pixel[localSize+(2*localRad)];

	MPI_Scatter(src, localSize*pixelTypeSize, pixelType, localSrc, localSize*pixelTypeSize, pixelType, root, MPI_COMM_WORLD);
	
	// last process getting the slack pixels
	const int leftoverSize = (ysize % numProc)*xsize;
	if ( (myid == root) && (ysize % numProc != 0) ) {
		const int leftoverOffset = ysize*xsize - leftoverSize;

		MPI_Send(src+leftoverOffset, leftoverSize*pixelTypeSize, pixelType, numProc-1, 0, MPI_COMM_WORLD);
	}
	else if( (myid == numProc-1) && (ysize % numProc) != 0){

		MPI_Recv(localSrc+localSize-leftoverSize, leftoverSize*pixelTypeSize,
			 pixelType, root, 0, MPI_COMM_WORLD, &status);
	} 

	// Horizontal blurring
  for (y=0; y<rowInt; y++) {
	for (x=0; x<xsize; x++) {
	  r = w[0] * pix(localSrc, x, y, xsize)->r;
	  g = w[0] * pix(localSrc, x, y, xsize)->g;
	  b = w[0] * pix(localSrc, x, y, xsize)->b;
	  n = w[0];
	  for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	x2 = x - wi;
	if(x2 >= 0) {
	  r += wc * pix(localSrc, x2, y, xsize)->r;
	  g += wc * pix(localSrc, x2, y, xsize)->g;
	  b += wc * pix(localSrc, x2, y, xsize)->b;
	  n += wc;
	}
	x2 = x + wi;
	if(x2 < xsize) {
	  r += wc * pix(localSrc, x2, y, xsize)->r;
	  g += wc * pix(localSrc, x2, y, xsize)->g;
	  b += wc * pix(localSrc, x2, y, xsize)->b;
	  n += wc;
	}
	  }
	  pix(localDist+localRad,x,y, xsize)->r = r/n;
	  pix(localDist+localRad,x,y, xsize)->g = g/n;
	  pix(localDist+localRad,x,y, xsize)->b = b/n;
	}
  }

	const int dataSize = pixelTypeSize*localRad;
	if (myid != root) {
		MPI_Send(localDist+localRad, dataSize, pixelType, myid-1, 1, MPI_COMM_WORLD);
		MPI_Recv(localDist, dataSize, pixelType, myid-1, 2, MPI_COMM_WORLD, &status);
	}
	if (myid != numProc-1) {
		MPI_Recv(localDist+localRad+(rowInt*xsize), dataSize, pixelType, myid+1, 1, MPI_COMM_WORLD, &status);
		MPI_Send(localDist+(rowInt*xsize), dataSize, pixelType, myid+1, 2, MPI_COMM_WORLD);
	}

	int min = 0;
	int max = rowInt + (2*radius);
	if (myid == root) {
		min = radius;
	}
	else if (myid == numProc-1) {
		max = rowInt+radius;
	}

	// Vertical blurring
  for (y=radius; y<rowInt+radius; y++) {
	for (x=0; x<xsize; x++) {
	  r = w[0] * pix(localDist, x, y, xsize)->r;
	  g = w[0] * pix(localDist, x, y, xsize)->g;
	  b = w[0] * pix(localDist, x, y, xsize)->b;
	  n = w[0];
	  for ( wi=1; wi <= radius; wi++) {
	wc = w[wi];
	y2 = y - wi;
	if(y2 >= min) {
	  r += wc * pix(localDist, x, y2, xsize)->r;
	  g += wc * pix(localDist, x, y2, xsize)->g;
	  b += wc * pix(localDist, x, y2, xsize)->b;
	  n += wc;
	}
	y2 = y + wi;
	if(y2 < max) {
	  r += wc * pix(localDist, x, y2, xsize)->r;
	  g += wc * pix(localDist, x, y2, xsize)->g;
	  b += wc * pix(localDist, x, y2, xsize)->b;
	  n += wc;
	}
	  }
	  pix(localSrc,x,y - radius, xsize)->r = r/n;
	  pix(localSrc,x,y - radius, xsize)->g = g/n;
	  pix(localSrc,x,y - radius, xsize)->b = b/n;
	}
  }

	const int gatherSize = (ysize % numProc)*xsize*pixelTypeSize;

	int recvCount[numProc];
	int displace[numProc];

	for ( int i = 0; i < numProc; i++) {
		recvCount[i] = (ysize/numProc)*xsize*pixelTypeSize;
		if(i == numProc-1)
		  recvCount[i] += (ysize%numProc)*xsize*pixelTypeSize;
		displace[i] = i * (ysize/numProc)*xsize*pixelTypeSize;
	}
	
	MPI_Gatherv(localSrc, localSize*pixelTypeSize, pixelType, src, recvCount, displace, pixelType, root, MPI_COMM_WORLD);
	
	delete[] localSrc;
	delete[] localDist;
}



