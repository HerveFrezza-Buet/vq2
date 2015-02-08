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
#include <fstream>


typedef vq2::Graph<std::string,double> Graph;
typedef Graph::vertex_type             Vertex;
typedef Graph::edge_type               Edge;
typedef Graph::ref_vertex_type         RefVertex;
typedef Graph::ref_edge_type           RefEdge;

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



int main(int argc, char* argv[]) {
  Graph              graph;
  DisplayVertex        display_vertex;

  // Here, references are handled. Vertex reference are just used for
  // making connexions... except for N, that is stored for further
  // use. The same stands for edge AC. For those two references,
  // take()/release() or take()/free() mechanism have to be used.

  // Let us put vertexs.

  RefVertex A(graph += "A");
  RefVertex B(graph += "B");
  RefVertex C(graph += "C");
  RefVertex D(graph += "D");

  // Let us connect vertexs.

  graph.connect(0,A,B);
  graph.connect(1,A,D);
  graph.connect(2,B,C);
  graph.connect(3,B,D);
  graph.connect(4,C,D);
  
  // Let us display the graph
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);

  // Let us save the graph
  {
    std::ofstream ofile;
    ofile.open("001.graph");
    ofile << graph;
    ofile.close();
  }

  // Let us continue the building of the graph
  graph.connect(3.14,A,C);
  RefVertex E(graph += "E");
  graph.connect(3.15,E,A);
  graph.connect(3.16,E,B);

  // Let us save the graph
  {
    std::ofstream ofile;
    ofile.open("002.graph");
    ofile << graph;
    ofile.close();
  }

  // Let us display the graph
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);

  // Let us load and display the graph
  {
    std::ifstream ifile;
    ifile.open("001.graph");
    ifile >> graph;
    ifile.close();
  }
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);

  // Let us load and display the graph
  {
    std::ifstream ifile;
    ifile.open("002.graph");
    ifile >> graph;
    ifile.close();
  }
  std::cout << std::endl;
  graph.for_each_vertex(display_vertex);
  graph.displayMemory(std::cout);

  return 0;
}
