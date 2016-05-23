#include <vector>
#include <time.h>
#include <ctime>
#include <iostream>
#include <cstlib>
#include <math.h>
#include <VT.h>
#include <mpi.h>

#include "definitions.h"
#include "physics.h"

using namespace std;

int target(const int area, pcord_t c) {
	return (int)c.x / area;
}

int main (int argc, char *argv[]) {
	struct timespec stime, etime;
	// MPI Stuff
	MPI_Status status;
	MPI_Init(&argc, &argv);
	const int root = 0; //root process
  	const int myid = MPI::COMM_WORLD.Get_rank(); // Rank of processes
  	const int numProc = MPI::COMM_WORLD.Get_size(); // N.o. processes
  	// com = MPI_COMM_WORLD
  	MPI_Datatype pcoordType;

  	// ITAC stuff
  	int vtClass, vtCollision, vtComm, vtSync;
  	VT_classdef("partsim", &vtClass);

  	VT_funcdef("collision", vtClass, &vtCollision);
  	VT_funcdef("communication", vtClass, &vtComm);
  	VT_funcdef("synchronization", vtClass, &vtSync);

  	int pressureCounter;
  	double bounds[2];

  	VT_countdef("pressure", vtClass, VT_COUNT_FLOAT, VT_GROUP_PROCESS, bounds, "p", &pressureCounter);
  	MPI_Type_contiguous(4, MPI_FLOAT, &pcoordType);
  	MPI_Type_commit(&pcoordType);

  	// Setup
  	const int AREA = BOX_HORIZ_SIZE / numProc;
  	vector<<pcord_t>* travellers = new vector<pcord_t>[numProc];
  	vector<particle_t> particles;
  	double totalMom, localTotalMom = 0.0;
  	cord_t wall;
  	wall.x0 = 0, wall.y0 = 0, wall.x1 = BOX_HORIZ_SIZE, wall.y1 = BOX_VERT_SIZE;

  	srand(time(0));

  	// Make particles
  	for (int i = 0; i < INIT_NO_PARTICLES; ++i) {
  		pcord_t c; particle_t p;
  		float r = rand() % MAX_INITIAL_VELOCITY;
  		float angle = rand() * M_PI * 2;
  		const float LOW = AREA * myid;
  		const float HIGH = AREA * (myid + 1);

  		c.x = LOW + (float)rand()/((float)RAND_MAX/(HIGH - LOW));
  		c.y = rand() % (int)BOX_VERT_SIZE;
  		c.vx = r * cos(angle);
  		c.vy = r * sin(angle);
  		p.pcord = c; p.ptype = 0;

  		particles.push_back(p);
  	}

  	float collision;
  	int newTarget;
  	pcord_t* rec;
  	rec = new pcord_t[PARTICLE_BUFFER_SIZE];

  	clock_gettime(CLOCK_REALTIME, &stime);
  	// Work work


  	if (myid == root) {
  		clock_gettime(CLOCK_REALTIME, &etime);
        printf("Simulation took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
               1e-9*(etime.tv_nsec  - stime.tv_nsec));
		printf("Pressure is %e\n", totalMom / (MAX_TIME * WALL_LENGTH));	
  	}
  	






}