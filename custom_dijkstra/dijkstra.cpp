#include "dijkstra.h"

Graph::Graph(int numVertices = 0){
    this->vertexCount = numVertices;
    this->adjacencyList.reserve(numVertices);
}

void Graph::addVertex(int vertexValue){
    vertex newVertex;
    newVertex.firstEdge = 0;
    newVertex.value = vertexValue;
    this->adjacencyList.push_back(newVertex);

    return;
}
void Graph::addEdge(int source, int destination, int weight){
    edge* newEdge = new edge;
    vertex* currentVertex = &(this->adjacencyList[source]);

    newEdge->destination = &(this->adjacencyList[destination]);
    newEdge->weight = weight;
    newEdge->nextEdge = currentVertex->firstEdge;
    currentVertex->firstEdge = newEdge;

    return;
}
void Graph::printEdges(){
    return;
}
void Graph::printEdges(int vertex){
    return;
}