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
#include <vector>
#include <fstream>

class Point {
public:
  double x,y;
  
  Point(void) : x(0), y(0) {}
  Point(double xx, double yy) : x(xx), y(yy) {}
  Point(const Point& p) : x(p.x), y(p.y)       {}
  Point& operator=(const Point& p) {
    if(this != &p) {
      x = p.x;
      y = p.y;
    }
    return *this;
  }

  Point operator+(const Point& p) const {
    return Point(x + p.x, y + p.y);
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

// As we have instrumented the Point class with the required method,
// the vector operation functor is obtained easily.
typedef vq2::by_default::VectorOp<Point> VectorOp;

// This is an abstract class for densities.
typedef vq2::proba::Density<Point> Density;

// This converts a Point into a 2D position on some A4 sheet paper.
class FigConverter {
public:
  typedef Point position_type;
  
  void locate(const position_type& pos,
	      double& x_cm, double& y_cm) {
    x_cm = 10 + 2*pos.x;
    y_cm = 10 + 2*pos.y;
  }
};

// Let us implement some elementary probability density.

class Box : public Density {
public:
  typedef Point event_type;
  Point min,max;
  double density;
  
  Box(const Point& origin,
      double width, double height,
      double proba_density)
    : min(origin), max(origin+Point(width,height)),
      density(proba_density) {}

  virtual double operator()(const event_type& evt) {
    if(evt.x >= min.x
       && evt.x < max.x
       && evt.y >= min.y
       && evt.y < max.y)
      return density;
    return 0;
  }
};

class Disk : public Density {
public:
  typedef Point event_type;

  double density;
  Disk(double proba_density)
    : density(proba_density) {}

  virtual double operator()(const event_type& evt) {
    if(evt.x*evt.x+evt.y*evt.y <= 1)
      return 1;
    return 0;
  }
};



#define NB_SAMPLES 10000
#define X_MIN -3
#define X_MAX  3
#define Y_MIN -1
#define Y_MAX  1
int main(int argc,char* argv[]) {
  VectorOp op;
  vq2::proba::random::init();

  Density& disk = *(new Disk(1.0));
  Density& small_disk  = vq2::proba::scale(disk,op,.7);
  Density& hole        = vq2::proba::invert(small_disk);
  Density& crown       = vq2::proba::min(disk,hole);
  Density& moved_crown = vq2::proba::translate(crown,op,Point(1.5,0));
  Density& bar         = *(new Box(Point(-.5,-.2), 1,.4, 1));
  Density& light_block = *(new Box(Point(-1.5,-1), 1, 2,.2));
  Density& heavy_block = *(new Box(Point(-2.5,-1), 1, 2, 1));
  Density& glue1       = vq2::proba::max(bar,moved_crown);
  Density& glue2       = vq2::proba::max(glue1,light_block);
  Density& glue3       = vq2::proba::max(glue2,heavy_block);
  Density& density     = glue3;

  std::ofstream file;
  FigConverter converter;
  vq2::xfig::GC gc;
  Point p;
  vq2::xfig::open("density.fig",file);
  gc.thickness = 2;
  gc.colorBlue();
  for(int i=0;i<NB_SAMPLES;++i) {
    p.x = vq2::proba::random::uniform(X_MIN,X_MAX);
    p.y = vq2::proba::random::uniform(Y_MIN,Y_MAX);
    if(vq2::proba::random::toss(density(p)))
      vq2::xfig::point(converter,file,gc,p);
  }
  vq2::xfig::close(file);
  std::cout << "\"density.fig\" generated." << std::endl;

  delete &disk;
  delete &small_disk;
  delete &hole;
  delete &crown;
  delete &moved_crown;
  delete &bar;
  delete &light_block;
  delete &heavy_block;
  delete &glue1;
  delete &glue2;
  delete &glue3;

  return 0;
}
