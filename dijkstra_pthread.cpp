//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
const int sz=3001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly
#define NUM_THREADS 4


//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator ()(pair<int, int>&p1 ,pair<int, int>&p2){
            return p1.second>p2.second;
        }
};

pthread_mutex_t mutexPQ;   // declare a global mutex

struct thread_data{
    int numNeighbor;
    int firstNeighbor;
    int cw;
    bool * vis;
    vector<pair<int,int> > neighbors;
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> * pq;
    int * dis;
    };



void *work(void *threadarg)
  {

    //printf(" Hello World! from child thread %lu\n",pthread_self());
    struct thread_data * my_data = (struct thread_data *) threadarg;
    int numNeighbor = my_data->numNeighbor;
    int firstNeighbor = my_data->firstNeighbor;
    int cw = my_data->cw;
    vector<pair<int,int> > neighbors = my_data->neighbors;
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> * pq = my_data->pq;
    bool * vis = my_data->vis;
    int * dis = my_data->dis;
    pair <int,int> tempPair;
    for (int i =firstNeighbor; i < numNeighbor+firstNeighbor; i++) {
        //printf("child thread %lu looking at node %d\n",pthread_self(),numNeighbor);
        if(!vis[neighbors[i].first] && neighbors[i].second+cw<dis[neighbors[i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
            tempPair = make_pair(neighbors[i].first,(dis[neighbors[i].first]=neighbors[i].second+cw));
            while (pthread_mutex_trylock(&mutexPQ));  // wait until released
            (*pq).push(tempPair); //Set the new distance and add to priority queue
            if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
    }
    pthread_exit(NULL);
  }




int * Dijkstra(int source, int n, vector<pair<int,int> > a[],int dis[]) //Algorithm for SSSP
{
    pthread_t threads[NUM_THREADS];
    if (pthread_mutex_init(&mutexPQ, NULL)) {
        printf("ERROR\n");
    }
    struct thread_data thread_data_array[NUM_THREADS];
    bool vis[sz];
    for(int i=0;i<sz;i++){ //Set initial distances to Infinity
        dis[i]=INF;
        vis[i] = false;
    }
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
        int totalNeighbor = a[cv].size();
        if(totalNeighbor > NUM_THREADS){
            //printf("total neighbors: %d\n",totalNeighbor);
            int nodes_per_thread = totalNeighbor/(NUM_THREADS+1); 
            int nodes_last_thread = totalNeighbor - nodes_per_thread*(NUM_THREADS);
            //printf("nodes per thread: %d\n",nodes_per_thread);
            //printf("nodes lasst thread: %d\n",nodes_last_thread);
            
            if (pthread_mutex_lock(&mutexPQ)) printf("\nERROR on lock\n");
            int t;
            for(t=0; t<NUM_THREADS;t++){
                thread_data_array[t].numNeighbor = nodes_per_thread;
                thread_data_array[t].firstNeighbor = t*nodes_per_thread;
                thread_data_array[t].cw = cw;
                thread_data_array[t].neighbors = a[cv];
                thread_data_array[t].vis = vis;
                thread_data_array[t].dis = dis;
                thread_data_array[t].pq = &pq;
                int rc = pthread_create(&threads[t], NULL, work,(void*) &thread_data_array[t]);
                if (rc) {
                    printf("ERROR; return code from pthread_create() is %d\n", rc);
                }
            }
            if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
            //printf("Before parent loop\n");
            int firstNeighbor = t*nodes_per_thread;
            int numNeighbor = nodes_last_thread;
            pair <int,int> tempPair;
            for (int i =firstNeighbor; i < numNeighbor+firstNeighbor; i++) {
                //printf("parent thread %lu looking at node %d\n",pthread_self(),numNeighbor);
                if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                    tempPair = make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw));
                    while (pthread_mutex_trylock(&mutexPQ));  // wait until released
                    pq.push(tempPair); //Set the new distance and add to priority queue
                    if (pthread_mutex_unlock(&mutexPQ)) printf("\nERROR on unlock\n");  // unlock thread
            }
            //printf("After parent loop\n");
            for (t = 0; t < NUM_THREADS; t++) {
                if (pthread_join(threads[t],NULL)){
                    printf("\n ERROR on join\n");
                }
            }
            //printf("\n main() after creating the thread.  My id is %lu\n",pthread_self());
        }
        else{
            //printf("Not enough neighbors\n");
            for(int i=0;i<a[cv].size();i++) //Iterating through all adjacent vertices
                if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                    pq.push(make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw))); //Set the new distance and add to priority queue
        }
    }
    return dis;
}