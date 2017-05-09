//Implementation for Dijkstra's SSSP(Single source shortest path) algorithm
//This is an optimized algorithm running in O(E*log(V))
 
#include <iostream>
#include <queue>
#include <vector>
#include <climits>
using namespace std;
#define INF INT_MAX //Infinity
 
const int sz=10001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly

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
        int i = 0;
        for(i=0;i<a[cv].size() - 2;i += 2){ //Iterating through all adjacent vertices
            if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                pq.push(make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw))); //Set the new distance and add to priority queue
            if(!vis[a[cv][i+1].first] && a[cv][i+1].second+cw<dis[a[cv][i+1].first])
                pq.push(make_pair(a[cv][i+1].first,(dis[a[cv][i+1].first]=a[cv][i+1].second+cw)));
            /*if(!vis[a[cv][i+2].first] && a[cv][i+2].second+cw<dis[a[cv][i+2].first])
                pq.push(make_pair(a[cv][i+2].first,(dis[a[cv][i+2].first]=a[cv][i+2].second+cw)));
            if(!vis[a[cv][i+3].first] && a[cv][i+3].second+cw<dis[a[cv][i+3].first])
                pq.push(make_pair(a[cv][i+3].first,(dis[a[cv][i+3].first]=a[cv][i+3].second+cw)));
            if(!vis[a[cv][i+4].first] && a[cv][i+4].second+cw<dis[a[cv][i+4].first])
                pq.push(make_pair(a[cv][i+4].first,(dis[a[cv][i+4].first]=a[cv][i+4].second+cw)));
            if(!vis[a[cv][i+5].first] && a[cv][i+5].second+cw<dis[a[cv][i+5].first])
                pq.push(make_pair(a[cv][i+5].first,(dis[a[cv][i+5].first]=a[cv][i+5].second+cw)));
            if(!vis[a[cv][i+6].first] && a[cv][i+6].second+cw<dis[a[cv][i+6].first])
                pq.push(make_pair(a[cv][i+6].first,(dis[a[cv][i+6].first]=a[cv][i+6].second+cw)));
            if(!vis[a[cv][i+7].first] && a[cv][i+7].second+cw<dis[a[cv][i+7].first])
                pq.push(make_pair(a[cv][i+7].first,(dis[a[cv][i+7].first]=a[cv][i+7].second+cw)));
            if(!vis[a[cv][i+8].first] && a[cv][i+8].second+cw<dis[a[cv][i+8].first])
                pq.push(make_pair(a[cv][i+8].first,(dis[a[cv][i+8].first]=a[cv][i+8].second+cw)));
            if(!vis[a[cv][i+9].first] && a[cv][i+9].second+cw<dis[a[cv][i+9].first])
                pq.push(make_pair(a[cv][i+9].first,(dis[a[cv][i+9].first]=a[cv][i+9].second+cw)));*/
        }
        for(i;i<a[cv].size();i++) {
            if(!vis[a[cv][i].first] && a[cv][i].second+cw<dis[a[cv][i].first])
                pq.push(make_pair(a[cv][i].first,(dis[a[cv][i].first]=a[cv][i].second+cw)));
        }
    }
    return dis;
}
 