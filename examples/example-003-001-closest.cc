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

#include <iostream>
#include <fstream>
#include <vq2.h>
#include <cstdlib>
#include <math.h>
#include <utility>
#include <array>



// Our graph will contain 2D points as vertices.
class Point {
public:
  double x,y;
  
  Point(void) 
    : x(vq2::proba::random::uniform()), 
      y(vq2::proba::random::uniform()) {}
  Point(const Point& p) : x(p.x), y(p.y)       {}
  Point& operator=(const Point& p) {
    if(this != &p) {
      x = p.x;
      y = p.y;
    }
    return *this;
  }
  
  friend std::ostream& operator<<(std::ostream& os,
				  const Point& p) {
    os << '(' << p.x << ", " << p.y << ")";
    return os;
  }
};

// This is the graph type.
typedef vq2::Graph<Point,double>  Graph;
typedef Graph::vertex_type        Vertex;
typedef Graph::ref_vertex_type    RefVertex;

// This functor displays vertices.
class ShowVertices {
public:
  bool operator()(Vertex& n) { 
    std::cout << ' ' << n.value;
    return false;
  }
};


// For closest-related algorithm, we need some similarity
// measure. Here, let us use the euclidian distance.
class Similarity {
public:

  typedef Point value_type;  // Type of vertex values
  typedef Point sample_type; // Type of external data

  double operator()(const value_type&  prototype,
		    const sample_type& sample) {
    double dx = prototype.x - sample.x;
    double dy = prototype.y - sample.y;
    return sqrt(dx*dx + dy*dy);
  }
};

// For computing distortions, the squared distances are rather used.
class Similarity2 {
public:
  typedef Point value_type;  
  typedef Point sample_type; 

  double operator()(const Point& prototype,
		    const Point& sample) const {
    double dx = prototype.x - sample.x;
    double dy = prototype.y - sample.y;
    return dx*dx + dy*dy;
  }
};

int main(int argc, char* argv[]) {
  Graph       g;
  ShowVertices   show_vertices;
  Similarity  distance;
  Similarity2 distance_2;

  vq2::proba::random::init(1000);

  for(int i = 0 ; i < 10 ; ++i) g += Point();
  std::cout << "Vertices : ";
  g.for_each_vertex(show_vertices);
  std::cout << std::endl;

  Point xi;
  std::cout << "xi : " << xi << std::endl;

  // Let us find the vertex that is the closest to xi.

  double closest_dist;
  RefVertex n = vq2::algo::closest(g,distance,
				   xi,closest_dist);
  std::cout << "closest : " << (*n).value << ", dist = " << closest_dist << std::endl;


  // Let us find the two closest vertices.

  std::pair<double,double> closest_dists;
  std::pair<RefVertex,RefVertex> two_closest = vq2::algo::twoClosest(g,distance,
								     xi,closest_dists);
  std::cout << "2 closest : " << std::endl
	    << "    first : " << (*(two_closest.first)).value  << ", dist = " << closest_dists.first << std::endl
	    << "   second : " << (*(two_closest.second)).value << ", dist = " << closest_dists.second << std::endl;


  // Let us build some other graph, progressively, and plot its
  // similarity to g.

  g.clear();
  for(int i = 0 ; i < 50 ; ++i) g += Point();

  // let us plot some similarity curve with gnuplot.
  Graph gg;
  std::ofstream file;

  file.open("similarity.plot");
  file << "set terminal wxt persist" << std::endl
       << "set title \"Evolution of similarity(g,gg) when gg gets more vertices\"" << std::endl
       << "set xlabel \"size of gg\"" << std::endl
       << "set ylabel \"similarity\"" << std::endl
       << "plot '-' using 1:2 with lines notitle" << std::endl;
  for(int i = 0 ; i < 100 ; ++i) {
    gg += Point();
    file << i << ' ' 
	 << vq2::algo::similarity(g,gg,distance) << std::endl;
    
  }
  file.close();
  std::cout << "File \"similarity.plot\" generated." << std::endl;


  // Last, from a set, let us compute the distortion.
  std::array<Point,100> point_set;
  // there are 100 random points since Point() initializes a random point.
  g.clear();
  for(int i = 0 ; i < 10 ; ++i) g += Point();
  std::cout << "Distorsion : " 
	    << vq2::algo::distortion(g,distance_2,point_set.begin(),point_set.end())
	    << std::endl;
  
}
