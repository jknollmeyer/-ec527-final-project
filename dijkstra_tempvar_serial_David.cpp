//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
//const int sz=10001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly

//Custom Comparator for Determining priority for priority queue (shortest edge comes first)
class prioritize {
    public: 
        bool operator ()(pair<int, int>&p1 ,pair<int, int>&p2){
            return p1.second>p2.second;
        }
};

int * Dijkstra(int source, int n, vector<pair<int,int> > a[],int dis[]) //Algorithm for SSSP
{
    int i, cv, cw, curNeighbor, combinedWeight, totalNeighbors;
    pair<int, int> curr;
    vector<pair<int,int> > theseNeighbors;
    bool vis[n];
    vis[0] = false;
    for(i=1;i<n;i++) { //Set initial distances to Infinity
        dis[i]=INF;
        vis[i]=false;
    }
    priority_queue<pair<int,int> ,vector<pair<int,int> >, prioritize> pq; //Priority queue to store vertex,weight pairs
    pq.push(make_pair(source,dis[source]=0)); //Pushing the source with distance from itself as 0

    while(!pq.empty())
    {
        curr=pq.top(); //Current vertex. The shortest distance for this has been found
        pq.pop();
        cv=curr.first;
        cw=curr.second; //'cw' the final shortest distance for this vertex
        if(vis[cv]) //If the vertex is already visited, no point in exploring adjacent vertices
            continue;
        vis[cv]=true;

        theseNeighbors = a[cv];
        totalNeighbors = theseNeighbors.size();
        for(i = 0; i < totalNeighbors; i++) //Iterating through all adjacent vertices
            curNeighbor = theseNeighbors[i].first;
            combinedWeight = theseNeighbors[i].second + cw;
            if(!vis[curNeighbor] && combinedWeight<dis[curNeighbor]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                pq.push(make_pair(curNeighbor,(dis[curNeighbor]=combinedWeight))); //Set the new distance and add to priority queue
    }
    return dis;
}
 