#include <iostream>
#include <queue>
#include <vector>
#include <climits>
#include <time.h>
#include "dijkstra_pthread.cpp"
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/linear_congruential.hpp>

using namespace boost;

typedef boost::minstd_rand base_generator_type;

#define INF INT_MAX //Infinity

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define numNodes 6          
#define edgeDensity 0.5     
#define maxEdgeWeight 20    

 
int main() //Driver Function for Dijkstra SSSP
{

    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp;
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    long long int time_sec, time_ns;

    base_generator_type generator(42);
    boost::uniform_real<> uni_dist(0,1);
    boost::variate_generator<base_generator_type&, boost::uniform_real<> > probability(generator, uni_dist);
    base_generator_type generatorInt(43);
    boost::uniform_int<> dist(1,maxEdgeWeight);//cannot have a lower bound of 0 on an edge weight
    boost::variate_generator<base_generator_type&, boost::uniform_int<> > weightRange(generatorInt, dist);
  
    int numTotalEdges = (numNodes * (numNodes - 1)) / 2;

    int dis[sz];
    vector<pair<int,int> > a[sz]; //Adjacency list
    int edges = 0;
    for (int uNode = 1; uNode < numNodes; uNode++) {
        //cout << "from Vertex " << uNode << ":" << endl;
        for (int vNode = uNode+1; vNode < numNodes+1; vNode++) {
            if (probability() <= edgeDensity) {
                edges++;
                int weight = weightRange();
                //cout << "\tVertex " << vNode << ", Weight: " << weight << endl;
                a[uNode].push_back(make_pair(vNode,weight));
                a[vNode].push_back(make_pair(uNode,weight));
            }
        }
    }

    int source = 1;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    Dijkstra(source,numNodes,a,dis);//SSSP from source (Also passing number of vertices as parameter)
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //cout<<"Source is: "<<source<<". The shortest distance to every other vertex from here is: \n";
    /*
    for(int i=1;i<=numNodes;i++)//Printing final shortest distances from source
    {
        cout<<"Vertex: "<<i<<" , Distance: ";
        dis[i]!=INF? cout<<dis[i]<<"\n" : cout<<"-1\n";
    }
    */
    time_stamp = diff(time1,time2);
    std::cout << numNodes << ", " << edges << ", ";
    std::cout << (long int)((double)(CPG)*(double)(GIG * time_stamp.tv_sec + time_stamp.tv_nsec));
    std::cout << '\n';
    return 0;
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

