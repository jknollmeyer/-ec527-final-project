//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))

//g++ -pthread dijkstra_custom.cpp -lrt

#include <iostream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
//const int sz = 1001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly //Should always have 1 more than the specified number of nodes in the dijkstra_custom.cpp
#define NUM_THREADS 4

//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator() (pair<int,int> &p1, pair<int,int> &p2) {
            return p1.second > p2.second;
        }
};


pthread_barrier_t barr;
pthread_mutex_t mutexPQ; // declare a global mutex



struct thread_data {//APPARENTLY IN C++, YOU CAN PASS BY REFERENCE WHICH YOU CAN'T IMPLICITLY DO IN C
    int tid;
    int n;
    vector<pair<int,int> >* adjacencyList;//for 0th thread only
    vector<pair<int,int> > * neighbors;//for non-0th threads only
    priority_queue<pair<int,int>, vector<pair<int,int> >, prioritize> * pq;
    int * nodes_per_thread;
    int * cw;
    bool * vis;
    int * dis;
    bool * solved;
};



void *work(void *threadarg) {
    //int i, numNeighbors, firstNeighbor, cw, curNeighbor, combinedWeight, rangeNeighbors;
    struct thread_data* my_data = (struct thread_data *) threadarg;
    int i, tid, rc, firstNeighbor, curNeighbor, combinedWeight, * cw, * nodes_per_thread, * dis;
    tid = my_data->tid;
    cw = my_data->cw;
    nodes_per_thread = my_data->nodes_per_thread;
    dis = my_data->dis;
    //int * cw = my_data->cw, nodes_per_thread = my_data->nodes_per_thread, dis = my_data->dis;
    bool * vis = my_data->vis, * solved = my_data->solved;
    
    priority_queue<pair<int,int>, vector<pair<int,int> >, prioritize>* pq = my_data->pq;
    vector<pair<int,int> > * curSetNeighbors = my_data->neighbors;
    vector<pair<int,int> > updatedEdges;

    if (tid == 0) {
        vector<pair<int,int> > * adj = my_data->adjacencyList;//can I do this?
        int totalNeighbor, cv;
        int myFirst = NUM_THREADS-1, n = my_data->n;
        pair<int, int> curr;

        for (int x = 1; x < n; x++) { //Set initial distances to Infinity
                dis[x] = INF;
                vis[x] = false;
        }

        while (!(*pq).empty()) {//the optimization step where we check if the number of neighbors > NUM_THREADS can't be done here because of the barrier... I wonder if that impacts performance for sparser graphs?
            curr = (*pq).top();
            (*pq).pop();

            cv = curr.first;
            *cw = curr.second;
            if (vis[cv]) //If the vertex is already visited, no point in exploring adjacent vertices
                continue;
            vis[cv] = true;

            *curSetNeighbors = adj[cv];
            totalNeighbor = curSetNeighbors->size();
            *nodes_per_thread = totalNeighbor/NUM_THREADS;
            //wait at barrier until all NUM_THREAD threads arrive
            rc = pthread_barrier_wait(&barr);
            if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
                printf("Could not wait on barrier\n");

            firstNeighbor = myFirst*(*nodes_per_thread);
            for (i = firstNeighbor; i < totalNeighbor; i++) {
                curNeighbor = (*curSetNeighbors)[i].first;
                combinedWeight = (*curSetNeighbors)[i].second + *cw;
                if (!vis[curNeighbor] && combinedWeight < dis[curNeighbor])
                    updatedEdges.push_back(make_pair(curNeighbor, (dis[curNeighbor] = combinedWeight)));
            }
            //Obtain lock on writing to pq (should be passed in as reference as member variable in thread_data struct)
            //and then just push onto the queue all pairs in vector, then release lock
            while (pthread_mutex_trylock(&mutexPQ));// wait until released
            while (!updatedEdges.empty()) {
                (*pq).push(updatedEdges.back());//David: not sure if I can do it like this?
                updatedEdges.pop_back();
            }
            if (pthread_mutex_unlock(&mutexPQ))
                printf("\nERROR on unlock\n"); // unlock thread
            //printf("Thread 0 finished while-loop\n");
        }
        //printf("Solved!\n");
        *solved = true;
        rc = pthread_barrier_wait(&barr);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
            printf("Could not wait on barrier\n");
    } 
    else {
        int rangeNeighbors;
        int myFirstId = tid-1;
        rc = pthread_barrier_wait(&barr);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)//essentially wait for 0th thread to finish processing nodes_per_thread, curSetNeighbors, pq, vis[cv] and cw in first iteration of loop
            printf("Could not wait on barrier\n");

        while (!(*solved)) {
            firstNeighbor = myFirstId*(*nodes_per_thread);
            rangeNeighbors = firstNeighbor + (*nodes_per_thread);
            for (i = firstNeighbor; i < rangeNeighbors; i++) {
                curNeighbor = (*curSetNeighbors)[i].first;
                combinedWeight = (*curSetNeighbors)[i].second + *cw;
                if (!vis[curNeighbor] && combinedWeight < dis[curNeighbor])
                    updatedEdges.push_back(make_pair(curNeighbor, (dis[curNeighbor] = combinedWeight)));
            }
            //Obtain lock on writing to pq (should be passed in as reference as member variable in thread_data struct)
            //and then just push onto the queue all pairs in vector, then release lock
            while (pthread_mutex_trylock(&mutexPQ));// wait until released
            while (!updatedEdges.empty()) {
                (*pq).push(updatedEdges.back());//David: not sure if I can do it like this?
                updatedEdges.pop_back();
            }
            if (pthread_mutex_unlock(&mutexPQ))
                printf("\nERROR on unlock\n"); // unlock thread


            rc = pthread_barrier_wait(&barr);
            if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
                printf("Could not wait on barrier\n");
            //printf("other thread after waiting for barrier\n");
        }
    }
    //printf("Thread %d now exiting...\n", tid);
    pthread_exit(NULL);
}


int * Dijkstra(int source, int n, vector<pair<int,int> > adj[], int dis[]) //Algorithm for SSSP
{
    if (pthread_mutex_init(&mutexPQ, NULL)) {
        printf("ERROR\n");
    }
    if(pthread_barrier_init(&barr, NULL, NUM_THREADS)) {
        printf("Could not create a barrier\n");
    }
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> pq; //Priority queue to store vertex,weight pairs
    pq.push(make_pair(source, dis[source] = 0)); //Pushing the source with distance from itself as 0

    bool vis[n];
    vis[0] = false;
    bool solved = false;
    int t, rc, cw, nodes_per_thread;
    cw = 0;
    nodes_per_thread = 0;
    vector<pair<int,int> > curNeighbors = adj[0];

    //printf("Starting thread creation...\n");
    for (t = 0; t < NUM_THREADS; t++) {
        thread_data_array[t] = {t, n, adj, &curNeighbors, &pq, &nodes_per_thread, &cw, vis, dis, &solved};
        rc = pthread_create(&threads[t], NULL, work, (void*) &thread_data_array[t]);
        if (rc) printf("ERROR; return code from pthread_create() is %d\n", rc);
    }
    for (t = 0; t < NUM_THREADS; t++) {
        if (pthread_join(threads[t], NULL)) printf("\n ERROR on join\n");
    }

    return dis;
}
