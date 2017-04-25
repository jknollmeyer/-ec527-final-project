#include <vector>

struct edge;

struct vertex {
    edge* firstEdge;
    int value;
};

struct edge
{
    vertex* destination;
    int weight;
    edge* nextEdge;
};

class Graph
{
    public:
        int edgeCount;
        int vertexCount;
        std::vector<vertex> adjacencyList;
    
        Graph(int numVertices);
        void addVertex(int vertexValue);
        void addEdge(int source, int destination, int weight);
        void printEdges();
        void printEdges(int vertexValue);
};