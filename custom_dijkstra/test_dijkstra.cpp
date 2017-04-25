#include <iostream>
#include <climits>
#include <queue>
#include <utility> // for pair
#include "dijkstra.h"

void printEdges(Graph inGraph);
int* dijkstra(Graph inGraph, int sourceValue, int destinationValue);

int main(){


    // START graph initializion
    int NUM_VERTICES = 10;
    Graph myGraph = Graph(NUM_VERTICES);

    for(int i = 0; i < NUM_VERTICES; i++){
        myGraph.addVertex(i);
    }
    for(int i = 0; i < NUM_VERTICES - 1; i++){
        myGraph.addEdge(i, i + 1, i * 2);
    }

    // Add a shortcut!
    myGraph.addEdge(0, NUM_VERTICES - 1, 1);
    // END graph initialization

    printEdges(myGraph);
    // Returns the "previous" array, composed of vertex*'s
    int* shortestPath = dijkstra(myGraph, 0, NUM_VERTICES - 1);

    int target = NUM_VERTICES - 1;
    for(int i = target; i > 0 ; i--){
        if(shortestPath[i] != -1){
            std::cout << shortestPath[i] << ", ";
            i = shortestPath[i];
        } else {
            std::cout << std::endl;
            break;
        }
    }
}

void printEdges(Graph inGraph){
    int numVertices = inGraph.vertexCount;
    for(int i = 0; i < numVertices; i++){
        vertex* currentVertex = &(inGraph.adjacencyList[i]);
        edge* currentEdge = currentVertex->firstEdge;
        while(currentEdge != 0){
            std::cout << "Source: " << currentVertex->value << "\tDestination: " << currentEdge->destination->value;
            std::cout << "\tWeight: " << currentEdge->weight << std::endl;
            currentEdge = currentEdge->nextEdge;
        }
    }
}

typedef std::pair<int, int*> distPair;

struct compareDist
{
    bool operator()(const distPair& l, const distPair& r){
        return *(l.second) > *(r.second);
    }
};

int* dijkstra(Graph inGraph, int sourceValue, int destinationValue){
    int numVertices = inGraph.vertexCount;
    int* dist = new int[numVertices];
    int* previous = new int[numVertices];
    bool* visited = new bool[numVertices];

     // priority queue containing pairs of <i, &(dist[i])>
    
    // Initialize dist to infinity and previous to undefined
    for(int i = 0; i < numVertices; i++){
        dist[i] = INT_MAX;
        previous[i] = -1; // essentially "undefined", no vertex would be position -1
        visited[i] = false;
    }
    // set source distance to 0
    dist[sourceValue] = 0;

    // load all vertices into a queue
    std::priority_queue<distPair > Q;
    Q.push(distPair(0, &(dist[0])));
    edge* currentEdge;
    vertex* currentVertex;
    int currentValue, neighborValue, newDistance;
    distPair currentPair;
    // go through each node and relax it against its neighbors
    while(!Q.empty()){
        currentPair = Q.top(); Q.pop();
        currentValue = currentPair.first;
        

        visited[currentValue] = true;
        currentVertex = &(inGraph.adjacencyList[currentValue]);
        currentEdge = currentVertex->firstEdge;

        while(currentEdge != 0){
            neighborValue = currentEdge->destination->value;
            
            newDistance = dist[neighborValue] + currentEdge->weight;
            if (newDistance < dist[currentValue]){
                dist[neighborValue] = newDistance;
                previous[neighborValue] = currentValue;
            }
            if(!visited[neighborValue]){
                Q.push(distPair(neighborValue, &(dist[neighborValue])));
            }
            currentEdge = currentEdge->nextEdge;
        }

    }
    delete dist;
    delete visited;

    return previous;
}