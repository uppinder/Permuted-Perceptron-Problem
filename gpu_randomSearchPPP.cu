#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "HandleError.h"

#define M 801 
#define N 817
#define K 15 // Choose K <= 16 < N.
#define G 30

void generateData(int A[M][N],int Y[N],int S[M],int H[N+1]);
void matrixMul(int A[M][N],int Y[N],int S[M]);

__global__ void computeMinCost(int *dev_A,int *dev_H);


int main(int argc,char* argv[]) {

	int A[M][N],Y[N],S[M],H[N+1];
	int *dev_A,*dev_H;

	int size_A = M * N * sizeof(int);
	int size_H = (N+1) * sizeof(int);

	
	/*
		Randomly generate A,Y and S. Compute H.
	 */
	generateData(A,Y,S,H);


	cudaMalloc((void**)&dev_A,size_A);
	cudaMalloc((void**)&dev_H,size_H);

	cudaMemcpy(dev_A,A,size_A,cudaMemcpyHostToDevice);
	cudaMemcpy(dev_H,H,size_H,cudaMemcpyHostToDevice);

	/*
		Launch kernel ... for calculating A*X for a
		particular block. In total 2^K blocks required.
	 */
	// Number of blocks required.

	float timeTaken;
	cudaEvent_t start,stop;

	HANDLE_ERROR( cudaEventCreate(&start));
	HANDLE_ERROR( cudaEventCreate(&stop));
	HANDLE_ERROR( cudaEventRecord(start, 0));

	const int numOfBlocks = 1 << K;
	computeMinCost<<< numOfBlocks, N >>>(dev_A,dev_H);
	
	HANDLE_ERROR( cudaEventRecord(stop, 0));
	HANDLE_ERROR( cudaEventSynchronize(stop));
	HANDLE_ERROR( cudaEventElapsedTime(&timeTaken, start, stop));
	
	printf("\nTime taken: %0.10fs\n\n",timeTaken/1000);	

	cudaFree(dev_A);
	cudaFree(dev_H);

	return 0;
}


__global__ void computeMinCost(int *A,int *H) {

	/*
		Get block number and calulate X. Compute S_x = AX. After
		that, compute S_xx for a particular thread in O(N). 
		( By flipping one bit(acc to thread no.)). Compute cost.  
	 */
	
	// Calculate X for a particular block.
		__shared__ int X[N],blockNumber;

		int idx = threadIdx.x;
		blockNumber = blockIdx.x;
		
		if(idx < N) {

			X[idx] = -1;
			if(idx < K)
				X[idx] = ( blockNumber & (1<<idx)) ? 1:-1; 	
		
		}

	__syncthreads();
	
	//Calculate S = AX for a particular block.
		__shared__ int S[M];

		int k,sum = 0;

		if(idx < M) {
			
			for(k = 0;k < N;k++)
				sum += A[idx * N + k] * X[k];

			S[idx] = sum; 
		}

	__syncthreads();

	// Now, on a thread basis, calculate cost.
	if(idx >= K && idx < N) {

		//Compute S_idx for a thread.
			int i,S_idx[M];
			for(i = 0;i < M;i++)
				S_idx[i] = S[i] + 2 * A[i * N + idx] * X[idx];

		//Calculate cost.
			int H_idx[N+1];
			
			for(i = 0; i <= N; i++)
				H_idx[i] = 0;

			for(i = 0;i < M;i++) {
				if(S_idx[i] >= 0)
					H_idx[S_idx[i]]++;
			}

			//1-Hamming Distance
			int hamDist = 0;
			for(i = 0;i < M;i++)
				hamDist += abs(S_idx[i]) - S_idx[i];

			//Histogram variation
			int histVar = 0;
			for(i = 0;i <= N;i++)
				histVar += abs(H[i] - H_idx[i]);

			int cost = G*hamDist + histVar;

			//printf("%d\n",cost);

	}

}


/*
	Generate random A and Y. Compute AY.
	If ith entry of AY is -ve, flip all 
	entries of ith row of A. When all 
	entries of AY are +ve, calculated A,Y,S
	are a data set.
 */
void generateData(int A[M][N],int Y[N],int S[M],int H[N+1]) {

	srand(time(NULL));
	int i,j;

	for(i = 0;i < M;i++)
		for(j = 0;j < N;j++) {

			A[i][j] = (rand()%2 == 0) ? -1:1; 
		}

	for(i = 0;i < N;i++) {
		Y[i] = (rand()%2 == 0) ? -1:1;
	}

	matrixMul(A,Y,S);

	for(i = 0;i < M;i++) {
		if(S[i] < 0) { 
			for(j = 0;j < N;j++) {
				A[i][j] = -A[i][j];
			}
			S[i] = -S[i];
		}
	}

	for(i = 0;i <= N;i++)
		H[i] = 0;

	for(i = 0;i < M;i++)
		H[S[i]]++;

}

void matrixMul(int A[M][N],int Y[N],int S[M]) {

	int i,k;

	for(i = 0;i < M;i++)  {
		S[i] = 0;
		for(k = 0;k < N ;k++)
			S[i] += A[i][k] * Y[k];
	}
		
}

