//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
const int sz=2000; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly

#define NUM_THREADS 2
// Barrier variable
pthread_barrier_t barr;

//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator ()(pair<int, int>&p1 ,pair<int, int>&p2){
            return p1.second>p2.second;
        }
};

pthread_mutex_t mutexPQ;   // declare a global mutex

struct thread_data{
    int threadid;
    int source;
    int n;
    vector<pair<int,int> > * adjacencyList;
    int * cv;
    int * cw;
    int * dis;
    int * nodes_this_iteration;
    bool * vis;
    bool * solved;
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> * pq;
};



void *work(void *threadarg)
{
    printf(" Hello World! from child thread %lu\n",pthread_self());
    struct thread_data * my_data = (struct thread_data *) threadarg;
    int threadid = my_data->threadid;
    int source = my_data->source;
    int n = my_data->n;
    vector<pair<int,int> > * adjacencyList = my_data->adjacencyList;
    int * cv = my_data->cv;
    int * cw = my_data->cw;
    int * nodes_this_iteration = my_data->nodes_this_iteration;
    int * dis = my_data->dis;
    bool * vis = my_data->vis;
    bool * solved = my_data->solved;
    priority_queue <pair<int,int> ,vector<pair<int,int> >, prioritize> * pq = my_data->pq;
    if(threadid == 0){
        *solved = false;
        for(int i=0;i<sz;i++){ //Set initial distances to Infinity
            dis[i]=INF;
            vis[i] = false;
        }
        (*pq).push(make_pair(source,dis[source]=0)); //Pushing the source with distance from itself as 0
        while(!(*pq).empty())
        {
            pair<int, int> curr=(*pq).top(); //Current vertex. The shortest distance for this has been found
            (*pq).pop();
            *cv=curr.first;
            *cw=curr.second; //'cw' the final shortest distance for this vertex
            if(vis[(*cv)]) //If the vertex is already visited, no point in exploring adjacent vertices
                continue;
            vis[(*cv)]=true;
            int totalNodes = adjacencyList[(*cv)].size();
            *nodes_this_iteration = totalNodes/NUM_THREADS;
            //wait at barrier until all NUM_THREAD threads arrive
            int rc = pthread_barrier_wait(&barr);
            if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
                printf("Could not wait on barrier\n");
            }
            //printf("total nodes this iterations: %d\n",totalNodes);
            //printf("thread 0 neighbors value: %d\n",neighbors_per_thread);
            int firstNeighbor = (NUM_THREADS-1)*(*nodes_this_iteration);
            //printf("thread 0 firstNeighbor: %d\n",firstNeighbor);
            pair <int,int> tempPair;
            for (int i=firstNeighbor; i < totalNodes; i++) {
                if(!vis[adjacencyList[(*cv)][i].first] && adjacencyList[(*cv)][i].second+(*cw)<dis[adjacencyList[(*cv)][i].first]){ //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                    tempPair = make_pair(adjacencyList[(*cv)][i].first,(dis[adjacencyList[(*cv)][i].first]=adjacencyList[(*cv)][i].second+(*cw)));
                    while (pthread_mutex_trylock(&mutexPQ));  // wait until released
                    (*pq).push(tempPair); //Set the new distance and add to priority queue
                    if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
                }
            }
            printf("thread 0 finished for loop\n");
        }
        *solved = true;
    }

    else{

        //wait at barrier until all NUM_THREAD threads arrive
        int rc = pthread_barrier_wait(&barr);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
        }
        //printf("other thread after waiting for barrier\n");
        //printf("other thread neighbors value: %d\n",neighbors_per_thread);
        while(!(*solved)){
            pair <int,int> tempPair;
            int firstNeighbor = (threadid-1)*(*nodes_this_iteration);
            //printf("other threads nodes this iteration: %d\n",(*nodes_this_iteration));
            //printf("firstNeighbor: %d\n",firstNeighbor);
            for (int i = firstNeighbor; i < (*nodes_this_iteration)+firstNeighbor; i++) {
                //printf("other thread looking at node %d\n",neighbors_per_thread);
                if(!vis[adjacencyList[(*cv)][i].first] && adjacencyList[(*cv)][i].second+(*cw)<dis[adjacencyList[(*cv)][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                tempPair = make_pair(adjacencyList[(*cv)][i].first,(dis[adjacencyList[(*cv)][i].first]=adjacencyList[(*cv)][i].second+(*cw)));
                while (pthread_mutex_trylock(&mutexPQ));  // wait until released
                (*pq).push(tempPair); //Set the new distance and add to priority queue
                if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
            }
            //printf("other thread finished for loop\n");
            //wait at barrier until all NUM_THREAD threads arrive
            int rc = pthread_barrier_wait(&barr);
            if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
                printf("Could not wait on barrier\n");
            }
            printf("other thread after waiting for barrier\n");
        }
    }
    //printf("other thread quit\n");

    pthread_exit(NULL);
}




int * Dijkstra(int source, int n, vector<pair<int,int> > a[],int dis[]) //Algorithm for SSSP
{
    pthread_t threads[NUM_THREADS];
    if (pthread_mutex_init(&mutexPQ, NULL)) {
        printf("ERROR\n");
    }
    if(pthread_barrier_init(&barr, NULL, NUM_THREADS)) {
        printf("Could not create a barrier\n");
    } 
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> pq; //Priority queue to store vertex,weight pairs
    struct thread_data thread_data_array[NUM_THREADS];
    int cv = 0;
    int cw=0;
    int nodes = 0;
    bool vis[sz];
    bool solved = false;
    if (pthread_mutex_lock(&mutexPQ)) printf("\nERROR on lock\n");
    for(int t = 0; t< NUM_THREADS;t++){
        thread_data_array[t].threadid = t;
        thread_data_array[t].source = source;
        thread_data_array[t].n = n;
        thread_data_array[t].adjacencyList = a;
        thread_data_array[t].cv = &cv;
        thread_data_array[t].cw = &cw;
        thread_data_array[t].nodes_this_iteration = &nodes;
        thread_data_array[t].dis = dis;
        thread_data_array[t].vis = vis;
        thread_data_array[t].pq = &pq;
        thread_data_array[t].solved = &solved;
        int rc = pthread_create(&threads[t], NULL, work,(void*) &thread_data_array[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
    for (int t = 0; t < NUM_THREADS; t++) {
        if (pthread_join(threads[t],NULL)){
            printf("\n ERROR on join\n");
        }
    }
    return dis;
}
