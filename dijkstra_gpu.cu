#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <queue>
#include <vector>
#include <climits>
#include <time.h>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/linear_congruential.hpp>

#include "dijkstra_pthread.cpp"

#define GIG 1000000000
#define CPG 3.07           // Cycles per GHz -- Adjust to your computer

#define NUM_THREADS_PER_BLOCK 	1000
#define NUM_BLOCKS 				1
#define PRINT_TIME 				1
#define SM_ARR_LEN				1000


#define numNodes 6          
#define edgeDensity 0.5     
#define maxEdgeWeight 20    

#define INF INT_MAX //Infinity
 
const int sz=10001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly


using namespace boost;

typedef boost::minstd_rand base_generator_type;


//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator ()(pair<int, int>&p1 ,pair<int, int>&p2){
            return p1.second>p2.second;
        }
};

// Assertion to check for errors
#define CUDA_SAFE_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
	if (code != cudaSuccess) 
	{
		fprintf(stderr,"CUDA_SAFE_CALL: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

#define IMUL(a, b) __mul24(a, b)

struct timespec diff(struct timespec start, struct timespec end);
int * dijkstra(int source, int n, vector<pair<int,int> > a[],int dis[]);

__global__ void kernel_dijkstra (float* d_A, float* d_B, float* d_C, int width) {
  	float sum = 0;
  	long int k;
  	int row = blockIdx.y*(width/2)+threadIdx.y;
  	int col = blockIdx.x*(width/2)+threadIdx.x;
  	for (k = 0; k < width; k++){
		float da_element = d_A[row*width+k];
		float db_element = d_B[k*width+col];
		sum+=da_element	* db_element;
  	}
  	d_C[row*width+col] = sum;
}


int * main(int source, int n, vector<pair<int,int> > a[],int dis[]){
	int arrLen = 0;
		
	// GPU Timing variables
	cudaEvent_t startFull, stopFull, startMMM, stopMMM;
	float elapsed_gpuFull, elapsed_gpuMMM;
	
	// Arrays on GPU global memoryc
	float *d_v;
	float *d_y;
	float *d_result;

	// Arrays on the host memory
	float *h_v;
	float *h_y;
	float *h_result;
	float *h_result_gold;

	int i, errCount = 0, zeroCount = 0;
	
	if (argc > 1) {
		arrLen  = atoi(argv[1]);
	}
	else {
		arrLen = SM_ARR_LEN;
	}

	printf("Length of the array = %d\n", arrLen);

    // Select GPU
    CUDA_SAFE_CALL(cudaSetDevice(0));

	// Allocate GPU memory
	size_t allocSize_float = arrLen * arrLen*sizeof(float);
	size_t allocSize_int = sizeof(int);
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_v, allocSize_float));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_y, allocSize_float));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_result, allocSize_float));
		
	// Allocate arrays on host memory
	h_v                          = (float *) malloc(allocSize_float);
	h_y                        = (float *) malloc(allocSize_float);
	h_result                   = (float *) malloc(allocSize_float);
	h_result_gold              = (float *) malloc(allocSize_float);
	
	// Initialize the host arrays
	printf("\nInitializing the arrays ...");
	// Arrays are initialized with a known seed for reproducability
	initializeArray2D(h_v, arrLen, 2453);
	initializeArray2D(h_y, arrLen, 1467);
	printf("\t... done\n\n");
	
	
#if PRINT_TIME
	// Create the cuda events
	cudaEventCreate(&startFull);
	cudaEventCreate(&stopFull);
	cudaEventCreate(&startMMM);
	cudaEventCreate(&stopMMM);
	// Record event on the default stream
	cudaEventRecord(startFull, 0);
#endif
	
	// Transfer the arrays to the GPU memory
	CUDA_SAFE_CALL(cudaMemcpy(d_v, h_v, allocSize_float, cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(d_y, h_y, allocSize_float, cudaMemcpyHostToDevice));
	//CUDA_SAFE_CALL(cudaMemcpy(d_result, h_result, allocSize_float, cudaMemcpyHostToDevice));
	cudaEventRecord(startMMM, 0);
	// Launch the kernel
	kernel_mmm<<<NUM_BLOCKS, NUM_THREADS_PER_BLOCK>>>(d_v,d_y,d_result,arrLen);
	cudaEventRecord(stopMMM,0);
	cudaEventSynchronize(stopMMM);
	// Check for errors during launch
	CUDA_SAFE_CALL(cudaPeekAtLastError());
	
	// Transfer the results back to the host
	CUDA_SAFE_CALL(cudaMemcpy(h_result, d_result, allocSize_float, cudaMemcpyDeviceToHost));

#if PRINT_TIME
	// Stop and destroy the timer
	cudaEventRecord(stopFull,0);
	cudaEventSynchronize(stopFull);
	cudaEventElapsedTime(&elapsed_gpuFull, startFull, stopFull);
	cudaEventElapsedTime(&elapsed_gpuMMM, startMMM, stopMMM);
	printf("\nGPU total time: %f (msec)\n", elapsed_gpuFull);
	printf("\nGPU MMM time: %f (msec)\n", elapsed_gpuMMM);
	cudaEventDestroy(startFull);
	cudaEventDestroy(stopFull);
	cudaEventDestroy(startMMM);
	cudaEventDestroy(stopMMM);
#endif

	struct timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    mmm(h_v,h_y,h_result_gold,arrLen);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    struct timespec time_stamp = diff(time1,time2);
    printf("CPU time: %ld\n", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp.tv_sec + time_stamp.tv_nsec)));

	// Compare the results
	for(i = 0; i < arrLen; i++) {
		if (abs(h_result_gold[i] - h_result[i]) > TOL) {
			errCount++;
		}
		if (h_result[i] == 0) {
			zeroCount++;
		}
	}
	
	
	for(i = 0; i < 10; i++) {
		printf("%d:\t%.8f\t%.8f\n", i, h_result_gold[i], h_result[i]);
	}
	
	
	if (errCount > 0 || zeroCount > 0) {
		if(errCount > 0 )
			printf("\n@ERROR: TEST FAILED: %d results did not matched\n", errCount);
		if (zeroCount > 0)
			printf("\n@ERROR: TEST FAILED: %d results (from GPU) are zero\n", zeroCount);
	}
	else {
		printf("\nTEST PASSED: All results matched\n");
	}
	//printf("\n iters = %d\n", h_iters);
	// Free-up device and host memory
	CUDA_SAFE_CALL(cudaFree(d_v));
	CUDA_SAFE_CALL(cudaFree(d_y));
	CUDA_SAFE_CALL(cudaFree(d_result));
		   
	free(h_v);
	free(h_y);
	free(h_result);
	free(h_result_gold);

	return 0;
}

int * Dijkstra(int source, int n, vector<pair<int,int> > a[],int dis[]) //Algorithm for SSSP
{
    bool vis[sz] = {0};
    for(int i=0;i<sz;i++) //Set initial distances to Infinity
        dis[i]=INF;
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> pq; //Priority queue to store vertex,weight pairs
    pq.push(make_pair(source,dis[source]=0)); //Pushing the source with distance from itself as 0
    while(!pq.empty())
    {
        pair<int, int> curr=pq.top(); //Current vertex. The shortest distance for this has been found
        pq.pop();
        int cv=curr.first,cw=curr.second; //'cw' the final shortest distance for this vertex
        if(vis[cv]) //If the vertex is already visited, no point in exploring adjacent vertices
            continue;
        vis[cv]=true;
        for(int i=0;i<a[cv].size();i++) //Iterating through all adjacent vertices
            if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                pq.push(make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw))); //Set the new distance and add to priority queue
    }
    return dis;
}

struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}