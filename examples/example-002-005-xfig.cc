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
#include <fstream>
#include <string>
#include <map>

class Point {
public:
  double x,y;
  Point(void) : x(0), y(0) {}
  Point(double xx, double yy) : x(xx), y(yy) {}
};
typedef vq2::Graph<Point,bool> Graph;
typedef Graph::vertex_type     Vertex;
typedef Graph::edge_type       Edge;
typedef Graph::ref_vertex_type RefVertex;
typedef Graph::ref_edge_type   RefEdge;

// This functor invalidates false-valued edges.
class EdgeValidity {
public:
  bool operator()(Edge& e) {
    e.stuff.efficient = e.value;
    return false; 
  }
};

// This converts a Point into a 2D position on some A4 sheet paper.
class FigConverter {
public:
  typedef Point position_type;
  
  void locate(const position_type& pos,
	      double& x_cm, double& y_cm) {
    x_cm = 3*pos.x  + 21.0/2;
    y_cm = 29.7/2 - 3*pos.y;
  }
};


#define VERTEX_RADIUS     .2  // cm
#define VERTEX_THICKNESS   3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH      50  // xfig depth
#define EDGE_THICKNESS     5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH        51  // xfig depth
int main(int argc, char* argv[]) {
  Graph              graph;
  std::ofstream      file;
  FigConverter       converter;

  RefVertex A(graph += Point(-2,-1));
  RefVertex B(graph += Point(-1,-1));
  RefVertex C(graph += Point( 1,-1));
  RefVertex D(graph += Point( 2,-1));

  RefVertex E(graph += Point(-2, 0));
  RefVertex F(graph += Point(-1, 0));
  RefVertex G(graph += Point( 0, 0));
  RefVertex H(graph += Point( 1, 0));
  RefVertex I(graph += Point( 2, 0));

  RefVertex J(graph += Point(-2, 1));
  RefVertex K(graph += Point(-1, 1));
  RefVertex L(graph += Point( 0, 1));
  RefVertex M(graph += Point( 1, 1));
  RefVertex N(graph += Point( 2, 1));

  // Let us create edges. False ones will be invalidated later.
  graph.connect(true,A,B);
  graph.connect(true,C,D);
  graph.connect(true,E,F);
  graph.connect(true,F,G);
  graph.connect(true,G,H);
  graph.connect(true,H,I);
  graph.connect(true,J,K);
  graph.connect(true,K,L);
  graph.connect(true,L,M);
  graph.connect(true,M,N);
  graph.connect(true,A,E);
  graph.connect(true,B,F);
  graph.connect(true,C,H);
  graph.connect(true,D,I);
  graph.connect(false,E,J);
  graph.connect(false,F,K);
  graph.connect(false,G,L);
  graph.connect(false,H,M);
  graph.connect(false,I,N);

  // Let us display the graph in some .fig file.
  vq2::xfig::open("full-graph.fig",file);
  vq2::xfig::graph(converter,file,graph,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  vq2::xfig::close(file);
  std::cout << "\"full-graph.fig\" generated." << std::endl;

  // Now, let us invalidate node G
  (*G).stuff.efficient = false;
  
  // Then, we invalidate false edges.
  EdgeValidity edge_validity;
  graph.for_each_edge(edge_validity);

  // Let us display the graph in some .fig file.
  vq2::xfig::open("invalidated-graph.fig",file);
  vq2::xfig::graph(converter,file,graph,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  vq2::xfig::close(file);
  std::cout << "\"invalidated-graph.fig\" generated." << std::endl;

  // Let us now labalize the connected components (ignoring
  // invalidated stuff).
  std::map<unsigned int,Graph::Component*> components;
  graph.computeConnectedComponents(components,true);

  // Let us display the graph in some .fig file.
  vq2::xfig::open("labeled-graph.fig",file);
  vq2::xfig::graph(converter,file,graph,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  vq2::xfig::close(file);
  std::cout << "\"labeled-graph.fig\" generated." << std::endl;

  return 0;
}
