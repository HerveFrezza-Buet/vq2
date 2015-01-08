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
#include <array>
#include <iostream>
#include <fstream>
#include <cstdlib>

// Prototypes will be 2D points
#define RMIN .35
#define RMAX .5
class Point {
public:
  double x,y;
  
  Point(void) : x(), y() {
    // It is convenient that default constructor tosses a point from
    // the distribution that we want to quantify. Here, this is some
    // 2D toroidal shape.
    double d,dx,dy;
    
    x = 0;
    y = 0;
    d = 0;
    while((d < RMIN*RMIN) || (d > RMAX*RMAX)) {
      x = vq2::proba::random::uniform();
      y = vq2::proba::random::uniform();
      dx = x-.5;
      dy = y-.5;
      d = dx*dx+dy*dy;
    }
  }
  Point(const Point& p) : x(p.x), y(p.y)       {}
  Point& operator=(const Point& p) {
    if(this != &p) {
      x = p.x;
      y = p.y;
    }
    return *this;
  }


  // We will obtain vector operation from the default class. So our
  // type must implement those methods.

  Point& operator=(double value) {
    x = value;
    y = value;
    return *this;
  }

  Point& operator+=(const Point& p) {
    x += p.x;
    y += p.y;
    return *this;
  }

  Point& operator/=(double scale) {
    x /= scale;
    y /= scale;
    return *this;
  }

  Point& operator*=(double scale) {
    x *= scale;
    y *= scale;
    return *this;
  }
};

// The value stored in the graph vertices is not directly the points,
// but some units. Units encapsulate the user data, that is stored as
// a prototype. The vq algorithms suppose that the vertex type is a
// unit, and a vertex u can provides the prototype by calling
// u.prototype(). The value u.prototype() is our Point. This
// encapsulation is not meaningfull for k-means, but it has been done
// for complience with other algorithms. You will not have to define
// Unit types by yourself (you could), since suitable default ones are
// provided.

typedef vq2::algo::kmeans::Unit<Point> Unit; // This will be stored as graph vertices.

// As we have instrumented the Point class with the required method,
// the vector operation functor is obtained easily.
typedef vq2::by_default::VectorOp<Point> VectorOp;

typedef vq2::Graph<Unit,char,Unit::copy_constructor> Graph; // char is dummy....
typedef Graph::vertex_type                           Vertex;

// For finding the winner in the LBG algorithm, we need some
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

// The similarity is instrumented to handle units.
typedef vq2::unit::Similarity<Unit,Similarity> UnitSimilarity;


// LBG adds prototypes by splitting existing once. Splitting a
// prototype consists of creating a new one, that is very close to the
// original.

#define SPLIT_AMPLITUDE 1e-5
class Split {
public:
  typedef Point value_type;
  typedef Point sample_type;

  void operator()(value_type& destination,
		  const sample_type& source) {
    destination.x = source.x + SPLIT_AMPLITUDE*(1-2*vq2::proba::random::uniform());
    destination.y = source.y + SPLIT_AMPLITUDE*(1-2*vq2::proba::random::uniform());
  }
};

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

// The fig converter type is instrumented to handle units.
typedef vq2::unit::XfigConverter<Unit,FigConverter> UnitFigConverter;

#define VERTEX_RADIUS     .2  // cm
#define VERTEX_THICKNESS   3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH      50  // xfig depth
#define EDGE_THICKNESS     5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH        51  // xfig depth
#define DATA_THICKNESS     2  // 1/80 inches (see xfig thickness)
#define DATA_DEPTH        52  // xfig depth


#define K                40
#define SAMPLE_SIZE    3000
int main(int argc, char* argv[]) {

  Graph          prototypes;
  Similarity     distance;   
  UnitSimilarity unit_distance(distance);
  Split          split;   
  VectorOp       op;
  
  vq2::proba::random::init();

  // Now, let us define a data set with random values.
  std::array<Point,SAMPLE_SIZE> data_set;

  // We apply k-means
  vq2::algo::kmeans::process(prototypes,K,
			     [](void) -> Point {return Point();}, // Point initializer function (lambda here).
			     unit_distance,op,split,
			     data_set.begin(),data_set.end());

  // Let us plot the result in some xfig file.
  std::ofstream file;
  
  FigConverter converter;
  UnitFigConverter unit_converter(converter); 

  vq2::xfig::GC gc;
  vq2::xfig::open("lbg.fig",file);
  vq2::xfig::graph(unit_converter,file,prototypes,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  gc.thickness = DATA_THICKNESS;
  gc.depth     = DATA_DEPTH;
  gc.colorBlue();
  for(int i=0;i<SAMPLE_SIZE;++i)
    vq2::xfig::point(converter,file,gc,data_set[i]);
  vq2::xfig::close(file);
  std::cout << "\"lbg.fig\" generated." << std::endl;
}
