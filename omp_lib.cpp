//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <queue>
#include <vector>
#include <climits>
#include <omp.h>
using namespace std;
#define INF INT_MAX //Infinity
#define NUM_THREADS 2
const int sz=10000; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly

//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator ()(pair<int, int>&p1 ,pair<int, int>&p2){
            return p1.second>p2.second;
        }
};

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
        #pragma omp parallel num_threads(NUM_THREADS){
            #pragma omp for
                for(int i=0;i<a[cv].size();i++) //Iterating through all adjacent vertices
                    if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                        pq.push(make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw))); //Set the new distance and add to priority queue
        }
    return dis;
}
 