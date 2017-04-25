//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee,
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <time.h>


//http://www.boost.org/doc/libs/1_55_0/libs/random/example/random_demo.cpp
#include <boost/random/uniform_real.hpp>//ADDED BY DAVID
#include <boost/random/variate_generator.hpp>//ADDED BY DAVID
#include <boost/random/uniform_int.hpp>//ADDED BY DAVID
#include <boost/random/linear_congruential.hpp>//ADDED BY DAVID
#include <boost/lexical_cast.hpp>//ADDED BY DAVID
#include <string>//ADDED BY DAVID
#include <sstream>//ADDED BY DAVID
#include <algorithm>//ADDED BY DAVID

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "dijkstra_speedup.cpp"


#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

//Following Erdos-Renyi random graph model -- essentially just "flip a coin" with probability p == edgeDensity for each edge in the graph to see if it's in the graph
//If I want a distribution, check out this: http://strategic.mit.edu/docs/matlab_networks/random_graph.m from http://strategic.mit.edu/downloads.php?page=matlab_networks
#define numNodes 20000			//ADDED BY DAVID
#define edgeDensity 0.3		//ADDED BY DAVID
#define maxEdgeWeight 20	//ADDED BY DAVID
//Valuable for later: http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/VertexListGraph.html
//	http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/graph_concepts.html
//	http://www.boost.org/doc/libs/1_63_0/libs/graph/doc/adjacency_list.html


using namespace boost;
typedef boost::minstd_rand base_generator_type;//ADDED BY DAVID

int
main(int, char *[])
{

    struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
    struct timespec time_stamp;
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
    long long int time_sec, time_ns;

  typedef adjacency_list < listS, vecS, directedS,
    no_property, property < edge_weight_t, int > > graph_t;
  typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
  typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
  typedef std::pair<int, int> Edge;
    std::cout <<"Nodes,   Edges,   nanoseconds\n";
  std::cout << numNodes;//ADDED BY DAVID
  //std::cout << "Edge Density: " << edgeDensity << std::endl;//ADDED BY DAVID
  //std::cout << "Range of Edge Weights: [1, " << maxEdgeWeight << "]" << std::endl;//ADDED BY DAVID
  
  
  base_generator_type generator(42);//ADDED BY DAVID
  boost::uniform_real<> uni_dist(0,1);//ADDED BY DAVID
  boost::variate_generator<base_generator_type&, boost::uniform_real<> > probability(generator, uni_dist);//ADDED BY DAVID
  base_generator_type generatorInt(43);//ADDED BY DAVID
  boost::uniform_int<> dist(1,maxEdgeWeight);//cannot have a lower bound of 0 on an edge weight//ADDED BY DAVID
  boost::variate_generator<base_generator_type&, boost::uniform_int<> > weightRange(generatorInt, dist);//ADDED BY DAVID
  
  int numTotalEdges = (numNodes * (numNodes - 1)) / 2;//ADDED BY DAVID

  /*//COMMENT-BLOCK ADDED BY DAVID
  const int num_nodes = 5;
  enum nodes { A, B, C, D, E };
  char name[] = "ABCDE";
  Edge edge_array[] = { Edge(A, C), Edge(B, B), Edge(B, D), Edge(B, E),
    Edge(C, B), Edge(C, D), Edge(D, E), Edge(E, A), Edge(E, B)
  };
  int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1 };
  int num_arcs = sizeof(edge_array) / sizeof(Edge);
  //*/
  
  std::vector<Edge> edgesVec;//ADDED BY DAVID
  std::vector<int> weights;//ADDED BY DAVID
  for (int uNode = 0; uNode < (numNodes-1); uNode++) {//ADDED BY DAVID
	for (int vNode = uNode+1; vNode < numNodes; vNode++) {//ADDED BY DAVID
	  if (probability() <= edgeDensity) {//ADDED BY DAVID
		edgesVec.push_back(std::make_pair(uNode, vNode));//ADDED BY DAVID
		weights.push_back(weightRange());//ADDED BY DAVID
	  }//ADDED BY DAVID
	}//ADDED BY DAVID
  }//ADDED BY DAVID

  std::cout << ", " << edgesVec.size();//ADDED BY DAVID
  //std::cout << "Percentage of all possible edges: " << ((float) edgesVec.size())/((float) numTotalEdges) << std::endl << std::endl;//ADDED BY DAVID
  
  std::stringstream weightsResult;//ADDED BY DAVID
  std::copy(weights.begin(), weights.end(), std::ostream_iterator<int>(weightsResult, " "));//ADDED BY DAVID
  //std::cout << "Edge-weights: " << weightsResult.str().c_str() << std::endl;//ADDED BY DAVID
  
  //std::cout << "Corresponding edges: ";//ADDED BY DAVID
  //for (int pp = 0; pp < edgesVec.size(); pp++) {//ADDED BY DAVID
//	  std::cout << "<" << edgesVec[pp].first << ", " << edgesVec[pp].second << "> ";//ADDED BY DAVID
 // }//ADDED BY DAVID
  //std::cout << std::endl << std::endl;//ADDED BY DAVID
  
  
  //BEGIN IF-CLAUSE
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  /*//COMMENT-BLOCK ADDED BY DAVID
  graph_t g(num_nodes);
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
  for (std::size_t j = 0; j < num_arcs; ++j) {
    edge_descriptor e; bool inserted;
    tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
    weightmap[e] = weights[j];
  }
  //*/
  //std::cout << "(defined(BOOST_MSVC) && BOOST_MSVC <= 1300) is TRUE" << std::endl;//ADDED BY DAVID
  graph_t g(numNodes);//ADDED BY DAVID
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);//ADDED BY DAVID
	
  std::vector<Edge>::iterator eiter = edgesVec.begin();//ADDED BY DAVID
  std::vector<int>::iterator w = weights.begin();//ADDED BY DAVID
	
  for (int x = 0; x < edgesVec.size(); x++) {//ADDED BY DAVID
	edge_descriptor e;//ADDED BY DAVID
	bool inserted;//ADDED BY DAVID
	tie(e, inserted) = add_edge(*eiter.first, *eiter.second, g);//ADDED BY DAVID
	weightmap[e] = *w;//ADDED BY DAVID
	eiter++;//ADDED BY DAVID
	w++;//ADDED BY DAVID
  }//ADDED BY DAVID
  
#else
  /*//COMMENT-BLOCK ADDED BY DAVID
  graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
  //*/
  
  //std::cout << "(defined(BOOST_MSVC) && BOOST_MSVC <= 1300) is FALSE" << std::endl;//ADDED BY DAVID
  //http://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-in-c
  int *weightsArr = &weights[0];//ADDED BY DAVID
  Edge *edge_array = &edgesVec[0];//ADDED BY DAVID
  //int num_arcs = sizeof(edge_array) / sizeof(Edge);//ADDED BY DAVID//Didn't use this because it was always 1//Which now that I see it, makes sense as there's no other implicit way to get the size of an array like with a vector
  int num_arcs = edgesVec.size();//ADDED BY DAVID
  graph_t g(edge_array, edge_array + num_arcs, weightsArr, numNodes);//ADDED BY DAVID
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);//ADDED BY DAVID
#endif

  //STAND-ALONE CODE
  std::vector<vertex_descriptor> p(num_vertices(g));
  std::vector<int> d(num_vertices(g));
  //vertex_descriptor s = vertex(A, g);//COMMENT-OUT ADDED BY DAVID
  vertex_descriptor s = vertex(0, g);//ADDED BY DAVID
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
  //BEGIN IF-CLAUSE
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  // VC++ has trouble with the named parameters mechanism
  property_map<graph_t, vertex_index_t>::type indexmap = get(vertex_index, g);
  dijkstra_shortest_paths(g, s, &p[0], &d[0], weightmap, indexmap,
                          std::less<int>(), closed_plus<int>(),
                          (std::numeric_limits<int>::max)(), 0,
                          default_dijkstra_visitor());
#else
  dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));
#endif
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp = diff(time1,time2);

      std::cout << ", ";
      std::cout << (long int)((double)(CPG)*(double)(GIG * time_stamp.tv_sec + time_stamp.tv_nsec));
      std::cout << '\n';
  //std::cout << "distances and parents:" << std::endl;
  graph_traits < graph_t >::vertex_iterator vi, vend;
  //for (tie(vi, vend) = vertices(g); vi != vend; ++vi) {
    //std::cout << "distance(" << name[*vi] << ") = " << d[*vi] << ", ";//COMMENT-OUT ADDED BY DAVID
    //std::cout << "parent(" << name[*vi] << ") = " << name[p[*vi]] << std:://COMMENT-OUT ADDED BY DAVID
	//std::cout << "distance(" << boost::lexical_cast<int>(*vi) << ") = " << d[*vi] << ", ";//ADDED BY DAVID
	//std::cout << "parent(" << boost::lexical_cast<int>(*vi) << ") = " << boost::lexical_cast<int>(p[*vi]) << std:://ADDED BY DAVID
	//std::cout << "distance(Node" << boost::lexical_cast<int>(*vi) << ") = " << d[*vi] << ", ";//ADDED BY DAVID
	//std::cout << "parent(Node" << boost::lexical_cast<int>(*vi) << ") = Node" << boost::lexical_cast<int>(p[*vi]) << std:://ADDED BY DAVID
      //endl;
  //}
  //std::cout << std::endl;

  std::ofstream dot_file("figs/dijkstra-eg.dot");

  dot_file << "digraph D {\n"
    << "  rankdir=LR\n"
    << "  size=\"4,3\"\n"
    << "  ratio=\"fill\"\n"
    << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

  graph_traits < graph_t >::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits < graph_t >::edge_descriptor e = *ei;
    graph_traits < graph_t >::vertex_descriptor
      u = source(e, g), v = target(e, g);
    //dot_file << name[u] << " -> " << name[v]//COMMENT-OUT ADDED BY DAVID
	//dot_file << boost::lexical_cast<int>(u) << " -> " << boost::lexical_cast<int>(v)//ADDED BY DAVID
	dot_file << "Node" << boost::lexical_cast<int>(u) << " -> Node" << boost::lexical_cast<int>(v)//ADDED BY DAVID
      << "[label=\"" << get(weightmap, e) << "\"";
    if (p[v] == u)
      dot_file << ", color=\"black\"";
    else
      dot_file << ", color=\"grey\"";
    dot_file << "]";
  }
  dot_file << "}";
  return EXIT_SUCCESS;
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