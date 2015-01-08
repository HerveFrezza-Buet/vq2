/*   This file is part of vq2
 *
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Herve Frezza-Buet
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : Herve.Frezza-Buet@supelec.fr
 *
 */

//#define vq2DEBUG

#include <vq2.h>
#include <iostream>
#include <string>


typedef vq2::Graph<std::string,double>   Graph;
typedef Graph::vertex_type                 Vertex;
typedef Graph::edge_type                 Edge;
typedef Graph::ref_vertex_type             RefVertex;
typedef Graph::ref_edge_type             RefEdge;


// This is a loop functor class.
class DisplayEdge {
public:
  std::string prefix;
  bool operator()(Edge& e) {

    std::cout << prefix << "Edge " << e.value << " : " 
	      << (*(e.n1)).value << " -> " << (*(e.n2)).value 
	      << std::endl;

    return false; // the element should not be removed.
  }
};

// This is a loop functor class.
class DisplayVertex {
public:
  bool operator()(Vertex& n) { 
    DisplayEdge display_edge;

    std::cout << "Vertex \"" << n.value << "\"" << std::endl;
    display_edge.prefix = "  ";
    n.for_each_edge(display_edge);
    return false; // the element should not be removed.
  }
};


// This is a loop functor class.
class RemoveZeroEdge {
public:
  bool operator()(Edge& e) {

    if(e.value < 1) {
      std::cout << "Removing " 
		<< (*(e.n1)).value << " -> " << (*(e.n2)).value 
		<< std::endl;
      return true;
    }
   
    return false;
  }
};

// This is a loop functor class.
class RemoveIsolatedVertex {
public:
  bool operator()(Vertex& n) { 

    if(n.edges.empty()) {
      std::cout << "Vertex \"" << n.value << "\" is isolated. Let's remove it." << std::endl;
      return true;
    }
    return false; 
  }
};


int main(int argc, char* argv[]) {
  Graph              graph;
  DisplayVertex        display_vertex;
  DisplayEdge        display_edge;
  RemoveZeroEdge     remove_zero_edge;
  RemoveIsolatedVertex remove_isolated_vertex;

  // Here, references are handled. Vertex reference are just used for
  // making connexions... except for N, that is stored for further
  // use. The same stands for edge AC. For those two references,
  // take()/release() or take()/free() mechanism have to be used.

  // Let us put vertexs.

  RefVertex A(graph += "A");
  RefVertex B(graph += "B");
  RefVertex C(graph += "C");
  RefVertex D(graph += "D");
  RefVertex E(graph += "E");

  // N is used for further manipulation (actually, this is just for a
  // further free).
  RefVertex N = D;
  N.take();

  // Let us connect vertexs.

  graph.connect(1.1,A,B);
  // AC is used for further manipulation (actually, this is just for a
  // further free).
  RefEdge AC = graph.connect(1.2,A,C);
  AC.take();
  graph.connect(1.3,A,D);
  graph.connect(1.4,B,C);
  graph.connect(1.5,B,D);
  graph.connect(1.6,C,D);
  graph.connect(0.1,E,A);
  graph.connect(0.2,E,B);
  graph.connect(0.3,E,C);
  
  // Let us display the graph
  graph.for_each_edge(display_edge);
  graph.for_each_vertex(display_vertex);

  // Let us show the memory actually used.
  std::cout << std::endl;
  graph.displayMemory(std::cout);

  // Let us remove all 0-valued edges
  std::cout << std::endl;
  graph.for_each_edge(remove_zero_edge);

  std::cout << std::endl
	    << "After loop edge free..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);
  std::cout << std::endl
	    << "After loop edge free and running through the graph..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);

  // Let us remove all isolated vertexs
  std::cout << std::endl;
  graph.for_each_vertex(remove_isolated_vertex);

  std::cout << std::endl
	    << "After loop vertex free..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);
  std::cout << std::endl
	    << "After loop vertex free and running through the graph..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);

  // Let us remove an edge from an outside reference.
  AC.free(); // This is ok since we have previously taken it.

  std::cout << std::endl
	    << "After outside edge free..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);
  std::cout << std::endl
	    << "After outside edge free and running through the graph..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);

  // Let us remove a vertex from an outside reference
  N.free(); // This is ok since we have previously taken it.

  std::cout << std::endl
	    << "After outside vertex free..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);
  std::cout << std::endl
	    << "After outside vertex free and running through the graph..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);

  std::cout << std::endl
	    << "Clearing..." << std::endl
	    << std::endl;

  graph.clear();

  std::cout << std::endl
	    << "After clearing..." << std::endl
	    << std::endl;
  graph.displayMemory(std::cout);

  return 0;
}
