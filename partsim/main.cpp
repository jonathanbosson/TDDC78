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

    // Declare some variables before for-loop
  	float collision;
  	int newTarget;
  	pcord_t* recBuffer = new pcord_t[PARTICLE_BUFFER_SIZE];

  	clock_gettime(CLOCK_REALTIME, &stime);
  	// Work work
    for (int t = 0; t < MAX_TIME; ++t) {
      int i = 0;
      int numParticles = particles.size();

      // Particle loop!
      while(i < numParticles) {
        collision = -1; // no hit

        VT_enter(vtCollision, VT_NOSCL);
        // Check collision with all other particles
        for (int k = i + 1; k < particles.size(); ++k) {
          collision = collide(&particles[i].pcord, &particles[k].pcord);
          // if hit call calculate momentum
          if (collision >= 0) {
            interact(&particles[i].pcord, &particles[k].pcord, collision);
            localTotalMom += wall_collide(&particles[j].pcord, wall);
            continue;
          }
        }

        VT_leave(VT_NOSCL);
        if (collision < 0) { // if no hit, translate
          feuler(&particles[i].pcord, 1);
          localTotalMom += wall_collide(&particles[i].pcord, wall);
        }

        newTarget = destination(AREA, particles[i].pcord);
        if(newTarget != myid) {
          travellers[newTarget].push_back(particles[i].pcord); // add to new vector
          particles.erase(particles.begin() + i); // remove from first vector
          numParticles = particles.size();
        } else
          i++;
      }

      int sentParticles = 0;
      pcord_t newCord;
      particle_t newParticle;
      MPI_Request req;
      VT_enter(vtComm, VT_NOSCL);

      // Send data out of order to all processes
      for (int i = 0; i < numProc; ++i) {
        if (i != myid) {
          MPI_Isend(&travellers[i][0], travellers[i].size(), pcoordType, i, 0, MPI_COMM_WORLD, &req);
          sentParticles += travellers[i].size();
        }
      }

      // Get data from everyone else
      for (int i = 0; i < numProc - 1; ++i) {
        int recSize;
        bool flag = false;
        while (!flag)
          MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
        MPI_Get_count(&status, pcoordType, &recSize);
        MPI_Recv(recBuffer, recSize, pcoordType, status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for (int j = 0; j < recSize; ++j) {
          newCoord.x = recBuffer[j].x;
          newCoord.y = recBuffer[j].y;
          newCoord.vy = recBuffer[j].vy;
          newCoord.vx = recBuffer[j].vx;

          newParticle.pcord = coord;
          newParticle.ptype = 0;
          particles.push_back(newParticle);
        }
      }
      VT_leave(VT_NOSCL);
      VT_enter(vtSync, VT_NOSCL);
      MPI_Barrier(MPI_COMM_WORLD);
      VT_leave(VT_NOSCL);

      // Empty traveller list
      for (int i = 0; i < numProc; ++i)
        travellers[i].clear();

      VT_countval(1, &pressureCounter, &localTotalMom);
    } // end of time loop

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&localTotalMom, &totalMom, numProc, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    VT_leave(VT_NOSCL);

  	if (myid == root) {
  		clock_gettime(CLOCK_REALTIME, &etime);
        printf("Simulation took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
               1e-9*(etime.tv_nsec  - stime.tv_nsec));
		printf("Pressure is %e\n", totalMom / (MAX_TIME * WALL_LENGTH));	
  	}
  	
    MPI_Finalize();
    return 0;
}