/*
 ============================================================================
 Name        : nbodySimulation.c
 Author      : Claudia Pipino
 Version     :
 Copyright   : Your copyright notice
 Description : N-Body Parallel Computation with collective commmunication
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define SOFTENING 1e-9f


typedef struct { float x, y, z, vx, vy, vz; } Body;

MPI_Datatype BodyMPI;

Body *bodies;

MPI_Status status;

int  process; /* rank of process */
int  p;       /* number of processes */
const double dt=0.1f; //iteration time


// initialization of number of particles and numb of iteration
int particles=30000;
const int numIter=30;
const int master = 0;


int main(int argc, char* argv[]){


	/* start up MPI */

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &process);

	MPI_Comm_size(MPI_COMM_WORLD, &p);



	bodies = malloc(particles * sizeof(Body)); // particles allocation

	MPI_Type_contiguous(6, MPI_FLOAT, &BodyMPI); //this is used for communication in MPI


	MPI_Type_commit(&BodyMPI);


	//random initialization of every bodies
	randomizedBodies();



	double tstart = MPI_Wtime();

	//I need a flag
	int aFlag = particles % p;
	int partition = particles / p;
	int firstPos= partition + 1;

	//if is 0 no problem, just call bodyforce
	if(aFlag==0){

		MPI_Scatter(bodies,partition,BodyMPI,bodies,partition,BodyMPI,0,MPI_COMM_WORLD);

		bodyForce(bodies,partition,partition*process);

	}else{
		if(process==master){
			int k=1;
			int start=firstPos; //start bodyforce from pos 1 not 0

			for(int i=1;i<p;i++){

				if(k<aFlag){
					//mod is not 0...we need 0 to compute
					MPI_Send(&bodies[start],firstPos,BodyMPI,i,0,MPI_COMM_WORLD);
					k++;
					start+=firstPos;
				}else{
					//when it will be 0 send partition to compute bodyforce

					MPI_Send(&bodies[start],partition,BodyMPI,i,0,MPI_COMM_WORLD);
					start+=partition;
				}
			}
			//compute bodyforce
			bodyForce(bodies,firstPos,0);

		}else{//SLAVE

			if(process<aFlag){
				MPI_Recv(bodies,firstPos,BodyMPI,0,0,MPI_COMM_WORLD,&status);
				bodyForce(bodies,firstPos,firstPos*process);

			}
			else{

				MPI_Recv(bodies,partition,BodyMPI,0,0,MPI_COMM_WORLD,&status);
				bodyForce(bodies,partition,partition*process);
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);


	if(process==master){
		double tend = MPI_Wtime();

		printBodies(bodies,particles,tend-tstart);
		free(bodies);
	}

	MPI_Type_free(&BodyMPI);
	MPI_Finalize();
	return 0;
}



void randomizedBodies(){
	for(int i=0; i< particles; i++){
		bodies[i].x=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
		bodies[i].y=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
		bodies[i].z=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
		bodies[i].vx=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
		bodies[i].vy=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
		bodies[i].vz=2.0f * (rand() / (float) RAND_MAX)  - 1.0f;
	}

}


void bodyForce(Body *bodyPart, int lenght, int start) {

	for (int i = start; i < start + lenght; i++) {
		float Fx = 0.0f;
		float Fy = 0.0f;
		float Fz = 0.0f;
		for(int it= 0; it < numIter;it++){
			for (int j = 0; j < particles; j++) {
				float dx = bodyPart[j].x - bodyPart[i].x;
				float dy = bodyPart[j].y - bodyPart[i].y;
				float dz = bodyPart[j].z - bodyPart[i].z;
				float distSqr = dx*dx + dy*dy + dz*dz + SOFTENING;
				float invDist = 1.0f / sqrtf(distSqr);
				float invDist3 = invDist * invDist * invDist;

				Fx += dx * invDist3;
				Fy += dy * invDist3;
				Fz += dz * invDist3;
			}

			bodyPart[i].vx += dt*Fx;
			bodyPart[i].vy += dt*Fy;
			bodyPart[i].vz += dt*Fz;

		}
		for (int i = start ; i < start + lenght; i++) {
			bodyPart[i].x += bodyPart[i].vx*dt;
			bodyPart[i].y += bodyPart[i].vy*dt;
			bodyPart[i].z += bodyPart[i].vz*dt;
		}
	}

}

/**
 * stampa bodies
 */
void printBodies(Body *body, int lenght, double totTime){
	printf("%d Bodies: %0.2f seconds\n", particles, totTime);


}


