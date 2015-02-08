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

#include <vq2.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <array>
#include <algorithm>


// Our SOM will contain 2D points as prototypes.
class Point {
public:
  double x,y;
  
  Point(void) : x(vq2::proba::random::uniform()), 
		y(vq2::proba::random::uniform()) {}
  Point(const Point& p) : x(p.x), y(p.y)       {}
  Point& operator=(const Point& p) {
    if(this != &p) {
      x = p.x;
      y = p.y;
    }
    return *this;
  }
};

// This is the SOM neurons.
typedef vq2::algo::som::Unit<Point> Unit;

// Do not forget the vertex initialization functor (3rd argument).
typedef vq2::Graph<Unit,char,Unit::copy_constructor>     Graph;
typedef Graph::ref_vertex_type                           RefVertex;

// For finding the winner in the SOM algorithm, we need some
// similarity measure. Here, let us use the squared euclidian
// distance.
class Similarity {
public:
  typedef Point value_type;
  typedef Point sample_type;

  double operator()(const value_type& arg1,
		    const sample_type& arg2) {
    double dx = arg1.x - arg2.x;
    double dy = arg1.y - arg2.y;
    return dx*dx + dy*dy;
  }
};
typedef vq2::unit::Similarity<Unit,Similarity> UnitSimilarity;

// The neighborhood functions determines the modulation (in [0,1])
// applied to the global learning rate for each prototype, according
// the the topological distance from this prototype to the winning
// one. That topological distance is the number of edges separating
// the winning prototype to the considered prototype.
class WinnerTakeMost {
public:
  double coef;
  double operator()(double topo_dist) {
    return std::max(1-topo_dist*coef,0.0);
  }
};

// The learning process.
class Learn {
public:
  typedef Point sample_type;
  typedef Point weight_type;
  void operator()(double coef,
		  weight_type& prototype,
		  const sample_type& target) {
    prototype.x += coef * (target.x - prototype.x);
    prototype.y += coef * (target.y - prototype.y);
  }
};
typedef vq2::unit::Learn<Unit,Learn> UnitLearn;


// This converts a Point into a 2D position on some A4 sheet paper.
class FigConverter {
public:
  typedef Point position_type;
  
  void locate(const position_type& pos,
	      double& x_cm, double& y_cm) {
    x_cm = 20*pos.x;
    y_cm = 20*pos.y;
  }
};
typedef vq2::unit::XfigConverter<Unit,FigConverter> UnitFigConverter;

#define VERTEX_RADIUS     .2  // cm
#define VERTEX_THICKNESS   3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH      50  // xfig depth
#define EDGE_THICKNESS     5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH        51  // xfig depth
#define DATA_THICKNESS     2  // 1/80 inches (see xfig thickness)
#define DATA_DEPTH        52  // xfig depth



#define EPOCH_SIZE     1000
#define LINE_SIZE       100
#define LEARNING_RATE   .05
#define SAMPLE_SIZE    1000

Point init_vertex(unsigned int w) {
  return Point();
}

char init_edge(unsigned int w, unsigned int ww) {
  return ' ';
}

int main(int argc, char* argv[]) {
  Graph som;
  int i;
  
  vq2::proba::random::init();

  // let us set up a ring. This can be done by hand (see next
  // HANDCRAFTED code section) This function returns a vector with the
  // created RefVertex references.
  std::vector<RefVertex> line = vq2::algo::make::ring(som,LINE_SIZE,
						      init_vertex,
						      init_edge);
			

#ifdef HANDCRAFTED // Not defined here
  // let us add vertexes in the graph, and store references into an array
  // for further display.
  RefVertex line[LINE_SIZE];
  for(i=0; i<LINE_SIZE; ++i)
    line[i] = (som += Point());
  // Let us connect the vertexes as a ring.
  for(i=0; i<LINE_SIZE-1; ++i)
    som.connect(' ',line[i],line[i+1]);
  som.connect(' ',line[i],line[0]);
#endif

  // Now, we need SOM-algorithm related stuff.
  Similarity     distance;
  UnitSimilarity unit_distance(distance);
  Learn          learning_rule;
  UnitLearn      unit_learning_rule(learning_rule);
  WinnerTakeMost competition;

  // Let us use progressively sharper neighbourhood sizes.
  double wta_coefs[4];
  wta_coefs[0] = .1; // wide
  wta_coefs[1] = .2; // medium
  wta_coefs[2] = .4; // sharp
  wta_coefs[3] = .9; // very sharp

  // This is it.
  for(int run=0; run<4; ++run) {
    competition.coef = wta_coefs[run];
    for(i=0; i<EPOCH_SIZE; ++i)
      vq2::algo::som::step(som,unit_distance,competition,
			   unit_learning_rule,
			   LEARNING_RATE,
			   Point()); // This submits a random input.
  }

  // Let us compute the distortion
  std::array<Point,SAMPLE_SIZE> points;
  double E = vq2::algo::distortion(som,unit_distance,points.begin(),points.end());
  std::cout << "Distortion is " << E << std::endl;

  // Let us plot the result in some xfig file.
  std::ofstream file;
  FigConverter converter;
  UnitFigConverter unit_converter(converter);
  vq2::xfig::GC gc;
  vq2::xfig::open("som.fig",file);
  vq2::xfig::graph(unit_converter,file,som,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  gc.thickness = DATA_THICKNESS;
  gc.depth     = DATA_DEPTH;
  gc.colorBlue();
  for(int i=0;i<SAMPLE_SIZE;++i)
    vq2::xfig::point(converter,file,gc,points[i]);
  vq2::xfig::close(file);
  std::cout << "\"som.fig\" generated." << std::endl;



}
