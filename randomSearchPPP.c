#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define M 801
#define N 817
#define K 15 // Choose K < N.
#define G 30

#define MIN(a,b) (a>b?b:a)
#define MAX(a,b) (a>b?a:b)

void generateData(int A[M][N],int Y[N],int S[M],int H[N+1]);
void matrixMul(int A[M][N],int Y[N],int S[M]);
void displayData(int A[M][N],int Y[N],int S[M],int H[N+1]);

int calculateCost(int S_x[M],int H[N+1]);
int abs(int n);

/*
	Take random e-matrix A(mxn) and a non-negative
	vector S(mx1). Generate random e-vectors Y(nx1).
	Compute the cost function. If yes, report Y else change one 
	element of Y.

 */

int main(int argc,char* argv[]) {


	int A[M][N],Y[N],S[M],H[N+1],X[N],i,j,minCost = INT_MAX,maxCost = INT_MIN,iterations = 0;

	generateData(A,Y,S,H);	

	clock_t begin,end;
	double time_spent;

	const int k = 1 << K;
	//printf("\nk = %d\n",k);
	
	begin = clock();

	for(i = 0;i < k; i++) {

		int z;
		for(z = 0;z < N;z++)
			X[z] = -1;	
		
		//For first K bits.
		for(j = 0;j < K;j++) 
			X[j] = (i&(1<<j)) ? 1:-1;
		
		//printf("\nX:");
		//for(z = 0;z < N;z++)
			//printf("%d ",X[z]);

		int S_i[M];
		matrixMul(A,X,S_i);

		for(z = K;z < N; z++,iterations++) {

			X[z] = -X[z];

			int p;
			//printf("\ni = %d,z = %d\nX:\n",i,z);
			//for(p = 0;p < N;p++)
				//printf("%d ",X[p]);

			
			// 	Calculate Sx for the neighbour of X
			// 	in O(n). s_i' = s_i - 2*a_i_k*x_k.
			// 	0 <= i < M, 0 <= k < N.
			 
			int m,S_xx[M];
			for(m = 0;m < M;m++)
				S_xx[m] = S_i[m] - 2 * A[m][z] * (-X[z]);

			// int S_x[M];
			// matrixMul(A,X,S_x);

			X[z] = -X[z];

			// printf("\nS_x:\n");
			// for(p = 0;p < M;p++)
			// 	printf("%d ",S_x[p]);
			
			// printf("\nS_xx:\n");
			// for(p = 0;p < M;p++)
			// 	printf("%d ",S_xx[p]);

			//Calculate the cost for X.
			int costX = calculateCost(S_xx,H);
			minCost = MIN(costX,minCost);
			maxCost = MAX(costX,maxCost);

			printf("\ni:%d Costx:%d MinCost:%d MaxCost:%d\n------------------\n\n",i,costX,minCost,maxCost);

		}

	}

	end = clock();
	time_spent = (double)(end - begin)/CLOCKS_PER_SEC;
	printf("\nTime taken: %0.10f\n",time_spent);
	printf("\nNumber of iterations: %d\n",iterations);

	return 0;
}

int calculateCost(int S_x[M],int H[N+1]) {

	int i,H_x[N+1];

	//Histogram for S_x
	for(i = 0;i <= N;i++)
		H_x[i] = 0;

	for(i = 0;i < M;i++) {
		if(S_x[i] >= 0)
			H_x[S_x[i]]++;
	}

	//Hamming Distance
	int hamDist = 0;
	for(i = 0;i < M;i++)
		hamDist += abs(S_x[i]) - S_x[i];

	//Histogram variation
	int histVar = 0;
	for(i = 0;i <= N;i++)
		histVar += abs(H[i] - H_x[i]);

	return G*hamDist + histVar;

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

	int i,j,k;

	for(i = 0;i < M;i++)  {
		S[i] = 0;
		for(k = 0;k < N ;k++)
			S[i] += A[i][k] * Y[k];
	}
		
}

void displayData(int A[M][N],int Y[N],int S[M],int H[N+1]) {

	int i,j;

	printf("A:\n");
	for(i = 0;i < M;i++) {
		for(j = 0;j < N ;j++)
			printf("%d  ",A[i][j]);
		printf("\n");
	}

	printf("Y:\n");
	for(i = 0;i < N;i++)
		printf("%d\n",Y[i]);


	printf("S:\n");
	for(i = 0;i < N;i++)
		printf("%d\n",S[i]);
	
	printf("H:\n");
	for(i = 0;i <= N;i++)
		printf("%d\n",H[i]);
	

}

int abs(int n) {
	return (n < 0)? -n:n;
}




