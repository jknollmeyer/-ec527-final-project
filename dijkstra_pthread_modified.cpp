//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
const int sz = 1000; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly
#define NUM_THREADS 4

//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator() (pair<int,int> &p1, pair<int,int> &p2) {
            return p1.second > p2.second;
        }
};



pthread_mutex_t mutexPQ; // declare a global mutex



struct thread_data {
    int numNeighbor;
    int firstNeighbor;
    int cw;
    vector<pair<int,int> > neighbors;
    priority_queue<pair<int,int>, vector<pair<int,int> >, prioritize> * pq;
    bool * vis;
    int * dis;
};



void *work(void *threadarg) {
    int i, numNeighbors, firstNeighbor, cw, curNeighbor, combinedWeight, rangeNeighbors;
    struct thread_data* my_data = (struct thread_data *) threadarg;

    numNeighbors = my_data->numNeighbor;
    firstNeighbor = my_data->firstNeighbor;
    cw = my_data->cw;
    vector<pair<int,int> > neighbors = my_data->neighbors;
    priority_queue<pair<int,int>, vector<pair<int,int> >, prioritize> * pq = my_data->pq;
    bool* vis = my_data->vis;
    int* dis = my_data->dis;

    vector<pair<int,int> > updatedEdges;
    rangeNeighbors = firstNeighbor + numNeighbors;
    for (i = firstNeighbor; i < rangeNeighbors; i++) {
        curNeighbor = neighbors[i].first;//<neighbor ID, weight of edge to this neighbor>
        combinedWeight = neighbors[i].second + cw;
        if (!vis[curNeighbor] && combinedWeight < dis[curNeighbor])//If unvisited node && combined shortest distance to this node < current set-distance, update this node's distance
            updatedEdges.push_back(make_pair(curNeighbor, (dis[curNeighbor] = combinedWeight)));//Set the new distance and add to local array
    }
    //Obtain lock on writing to pq (should be passed in as reference as member variable in thread_data struct)
    //and then just push onto the queue all pairs in vector, then release lock
    while (pthread_mutex_trylock(&mutexPQ)); // wait until released
    while (!updatedEdges.empty()) {
        (*pq).push(updatedEdges.back());//not sure if I can do it like this?
        updatedEdges.pop_back();
    }
    if (pthread_mutex_unlock(&mutexPQ))
        printf("\nERROR on unlock\n"); // unlock thread

    pthread_exit(NULL);
}







int * Dijkstra(int source, int n, vector<pair<int,int> > a[], int dis[]) //Algorithm for SSSP
{
    pthread_t threads[NUM_THREADS];
    if (pthread_mutex_init(&mutexPQ, NULL)) {
        printf("ERROR\n");
    }
    struct thread_data thread_data_array[NUM_THREADS];
    bool vis[sz] = {0};

    for (int i=0;i<sz;i++) //Set initial distances to Infinity
        dis[i] = INF;
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> pq; //Priority queue to store vertex,weight pairs
    pq.push(make_pair(source, dis[source] = 0)); //Pushing the source with distance from itself as 0

    while(!pq.empty())
    {
        pair<int, int> curr = pq.top(); //Current vertex. The shortest distance for this has been found
        pq.pop();

        int cv = curr.first, cw = curr.second; //'cw' the final shortest distance for this vertex
        if (vis[cv]) //If the vertex is already visited, no point in exploring adjacent vertices
            continue;
        vis[cv] = true;

        vector<pair<int,int> > theseNeighbors = a[cv];
        int totalNeighbor = theseNeighbors.size();
        if (totalNeighbor > NUM_THREADS){
            int nodes_per_thread = totalNeighbor/(NUM_THREADS + 1); 
            if (pthread_mutex_lock(&mutexPQ)) 
                printf("\nERROR on lock\n");

            int t, i;
            for (t = 0; t < NUM_THREADS; t++){
                thread_data temp = {nodes_per_thread, t*nodes_per_thread, cw, theseNeighbors, &pq, vis, dis};
                thread_data_array[t] = temp;//David: not sure if I can do it like this...?
                /*
                thread_data_array[t].thread_id = t;
                thread_data_array[t].numNeighbor = nodes_per_thread;
                thread_data_array[t].firstNeighbor = t*nodes_per_thread;
                thread_data_array[t].cw = cw;
                thread_data_array[t].neighbors = a[cv];
                thread_data_array[t].vis = vis;
                thread_data_array[t].dis = dis;
                //*/
                int rc = pthread_create(&threads[t], NULL, work, (void*) &thread_data_array[t]);
                if (rc) {
                    printf("ERROR; return code from pthread_create() is %d\n", rc);
                }
            }
            if (pthread_mutex_unlock(&mutexPQ)) 
                printf("\nERROR on unlock\n"); // unlock thread


            int firstNeighbor = t*nodes_per_thread;//essentially totalNeighbor*NUM_THREADS/(NUM_THREADS+1)
            int aggNeighbor = totalNeighbor - (nodes_per_thread*(NUM_THREADS)) + firstNeighbor;//essentially (totalNeighbor - totalNeighbor*(NUM_THREADS)/(NUM_THREADS+1)) + firstNeighbor
            int f, s;
            for (i = firstNeighbor; i < aggNeighbor+firstNeighbor; i++) {
                f = theseNeighbors[i].first;
                s = theseNeighbors[i].second + cw;
                if(!vis[f] && s < dis[f]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                    pq.push(make_pair(f, (dis[f] = s))); //Set the new distance and add to priority queue
            }

            for (t = 0; t < NUM_THREADS; t++) {
                if (pthread_join(threads[t], NULL)){
                    printf("\n ERROR on join\n");
                }
            }
        } else {
            printf("Not enough neighbors\n");
            int i, f, s;
            for (i = 0; i < totalNeighbor; i++) //Iterating through all adjacent vertices
                f = theseNeighbors[i].first;
                s = theseNeighbors[i].second + cw;
                if (!vis[f] && s < dis[f]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                    pq.push(make_pair(f, (dis[f] = s))); //Set the new distance and add to priority queue
        }
    }
    return dis;
}
