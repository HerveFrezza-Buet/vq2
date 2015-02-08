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
#include <map>
#include <iostream>
#include <string>
#include <fstream>


typedef vq2::Graph<std::string,double> Graph;
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
class RemoveNonNullEdge {
public:
  bool operator()(Edge& e) {
    return e.value != 0; 
  }
};

void display_components(std::map<unsigned int,Graph::Component*>& components,
			const std::string& title);
int main(int argc, char* argv[]) {
  Graph              graph;

  RefVertex A(graph += "A");
  RefVertex B(graph += "B");
  RefVertex C(graph += "C");
  RefVertex D(graph += "D");
  RefVertex W(graph += "W");
  RefVertex X(graph += "X");
  RefVertex Y(graph += "Y");
  RefVertex Z(graph += "Z");
  RefVertex K(graph += "K");
  RefVertex L(graph += "L");

  // Let us connect vertexs.

  graph.connect(1,A,B);
  graph.connect(0,B,C);
  graph.connect(0,C,D);
  graph.connect(0,D,A);

  graph.connect(1,W,X);
  graph.connect(0,X,Y);
  graph.connect(0,Y,Z);
  graph.connect(0,Z,W);

  graph.connect(1,Z,L);
  graph.connect(0,K,L);

  // Let us now compute the connected components.
  std::map<unsigned int,Graph::Component*> components;
  graph.computeConnectedComponents(components,false);
  display_components(components, "Initial graph (no labelling)");

  // Let us now remove non-null edges, and rebuild the connected
  // components.

  RemoveNonNullEdge remove_non_null;
  graph.for_each_edge(remove_non_null);
  graph.computeConnectedComponents(components,false);
  display_components(components, "1-edge removed (no labelling)");

  // Let us now re-compute the connected components, while actually
  // using the labelling procedure.
  graph.computeConnectedComponents(components,true);
  display_components(components, "Labelling");

  // Let us re-connect Z and L
  RefEdge ZL = graph.connect(0,Z,L); ZL.take();
  graph.computeConnectedComponents(components,true);
  display_components(components, "[Z-L] added.");

  // Let us connect A and W
  RefEdge AW = graph.connect(0,A,W); AW.take();
  graph.computeConnectedComponents(components,true);
  display_components(components, "[A-W] added.");

  // Let us remove A and W
  AW.free();
  graph.computeConnectedComponents(components,true);
  display_components(components, "[A-W] removed.");

  // Let us remove Z and L
  ZL.free();
  graph.computeConnectedComponents(components,true);
  display_components(components, "[Z-L] removed.");


  return 0;
}

void display_components(std::map<unsigned int,Graph::Component*>& components,
			const std::string& title) {
  std::map<unsigned int,Graph::Component*>::iterator iter,end;
  Graph::Component*                         comp;
  DisplayVertex                               display_vertex;

  std::cout << std::endl
	    << "########################" << std::endl
	    << title << std::endl
	    << "########################" << std::endl
	    << std::endl;
  for(iter = components.begin(), end = components.end();
      iter != end;
      ++iter) {
    std::cout << "Label " << (*iter).first << std::endl
	      << std::endl;
    comp = (*iter).second;
    comp->for_each_vertex(display_vertex);
    std::cout << std::endl
	      << "----------------------" << std::endl
	      << std::endl;
  }
}
