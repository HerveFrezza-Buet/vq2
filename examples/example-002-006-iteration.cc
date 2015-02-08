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

typedef vq2::Graph<std::string,std::string>  Graph;
typedef Graph::vertex_type                   Vertex;
typedef Graph::edge_type                     Edge;
typedef Graph::ref_vertex_type               RefVertex;
typedef Graph::ref_edge_type                 RefEdge;
 

// These are some functors, using ref and non ref types. They all do
// the same.

struct PrintEdge {
  bool operator()(Edge& e) { 
    std::cout << "--- " << e.value << std::endl;
    return false; // the element should not be removed.
  }
};

struct PrintEdgeRef {
  bool operator()(RefEdge& ref) { 
    Edge& e = *ref;
    std::cout << "--- " << e.value << std::endl;
    return false; // the element should not be removed.
  }
};

struct PrintVertex {
  bool operator()(Vertex& n) { 
    std::cout << "### " << n.value << std::endl;
    PrintEdge pe;
    n.for_each_edge(pe);
    return false; // the element should not be removed.
  }
};

struct PrintVertexRef {
  bool operator()(RefVertex& ref) { 
    Vertex& n = *ref;
    std::cout << "### " << n.value << std::endl;
    PrintEdgeRef per;
    n.for_each_edge_ref(per);
  }
};




int main(int argc, char* argv[]) {
  Graph              graph;

  // Let us put vertices.

  RefVertex A(graph += "A");
  RefVertex B(graph += "B");
  RefVertex C(graph += "C");
  RefVertex X(graph += "X");
  RefVertex Y(graph += "Y");
  RefVertex Z(graph += "Z");

  // Let us create two connected components.

  graph.connect("ab",A,B);
  graph.connect("bc",B,C);
  graph.connect("ca",C,A);

  graph.connect("xy",X,Y);
  graph.connect("yz",Y,Z);
  graph.connect("zx",Z,X);

  // Let us compute the connected components.
  
  std::map<unsigned int,Graph::Component*> components;
  graph.computeConnectedComponents(components,false);

  // Now, let ys play with all the iteration functors.

  PrintVertex    pv;
  PrintEdge      pe;
  PrintVertexRef pvr;
  PrintEdgeRef   per;

  std::cout << std::endl << "1" << std::endl << std::endl;
  graph.for_each_vertex(pv);

  std::cout << std::endl << "2" << std::endl << std::endl;
  graph.for_each_edge(pe);

  std::cout << std::endl << "3" << std::endl << std::endl;
  for(auto& labelcomp : components) {
    std::cout << "LABEL " << labelcomp.first << std::endl;
    Graph::Component& comp =  *(labelcomp.second);
    std::cout << "VERTICES" << std::endl;
    comp.for_each_vertex(pv);
    std::cout << "EDGES" << std::endl;
    comp.for_each_edge(pe);
  }

  std::cout << std::endl << "4" << std::endl << std::endl;
  graph.for_each_vertex_ref(pvr);

  std::cout << std::endl << "5" << std::endl << std::endl;
  graph.for_each_edge_ref(per);

  std::cout << std::endl << "6" << std::endl << std::endl;
  for(auto& labelcomp : components) {
    std::cout << "LABEL " << labelcomp.first << std::endl;
    Graph::Component& comp =  *(labelcomp.second);
    std::cout << "VERTICES" << std::endl;
    comp.for_each_vertex_ref(pvr);
    std::cout << "EDGES" << std::endl;
    comp.for_each_edge_ref(per);
  }


  return 0;
}
