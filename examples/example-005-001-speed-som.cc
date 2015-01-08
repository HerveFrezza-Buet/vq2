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
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cmath>





// Let us use 2D points as prototypes.
// Our SOM will contain 2D points as prototypes.
//
// Warning : Default constructor must build 0 for speed handling !!!
//
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


// This is the SOM neurons.
typedef vq2::algo::som::Unit<Point> SOMUnit;
// Let us decorate them with speed management capabilities and thus
// define our final unit type.
typedef vq2::temporal::Unit<SOMUnit> Unit;
// Do not forget the vertex initialization functor (3rd argument).
typedef vq2::Graph<Unit,char,Unit::copy_constructor> Graph;
typedef Graph::ref_vertex_type                       RefVertex;

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
typedef vq2::unit::Learn<Unit,Learn>  UnitLearn;

#define INPUT_HALF_SIDE         1
#define INPUT_AMPLITUDE         3
#define INPUT_STAGE_LENGTH     20
#define INPUT_NB_SAMPLES    10000
class Input {
private:
  
  double cost,sint;
  double x;
  
  void setAngle(double deg) {
    double rad = deg*M_PI/180.0;
    cost = cos(-rad);
    sint = sin(-rad);
  }
  
  void setPos(double xpos) {
    x = xpos;
  }

  void setTick(int t) {
    int stage = t / INPUT_STAGE_LENGTH;
    double tt = (t % INPUT_STAGE_LENGTH)/(INPUT_STAGE_LENGTH-1.0);
    int phase = stage % 4;
    switch(phase) {
    case 0:
    case 2:
      setAngle(tt*90);
	break;
    case 1:
      setPos(tt*INPUT_AMPLITUDE);
	break;
    case 3:
      setPos((1-tt)*INPUT_AMPLITUDE);
      break;
    }
  }
  
  bool isOn(const Point& evt) {

    Point m(cost*(evt.x-x)-sint*evt.y,
	    sint*(evt.x-x)+cost*evt.y);
    if(m.x >= -INPUT_HALF_SIDE
       && m.x <= INPUT_HALF_SIDE
       && m.y >= -INPUT_HALF_SIDE
       && m.y <= INPUT_HALF_SIDE)
      return 1;
    return 0;
  }

public:
  
  std::vector<Point> samples;
  Input() : cost(1), sint(0), x(0) {}
  
  void tick(int t) {
    setTick(t);
    samples.clear();
    
    for(int s = 0; s < INPUT_NB_SAMPLES; ++s) {
      Point p(vq2::proba::random::uniform(-1.5*INPUT_HALF_SIDE,INPUT_AMPLITUDE+1.5*INPUT_HALF_SIDE),
	      vq2::proba::random::uniform(-1.5*INPUT_HALF_SIDE,+1.5*INPUT_HALF_SIDE));
      if(isOn(p))
	samples.push_back(p);
    }
  }
};

class FigConverter {
public:
  typedef Point position_type;
  
  void locate(const position_type& pos,
              double& x_cm, double& y_cm) {
    x_cm = 4*(pos.x-INPUT_AMPLITUDE/2)  + 21.0/2;
    y_cm = 29.7/2 - 4*pos.y;
  }
};
typedef vq2::unit::XfigConverter<Unit,FigConverter> UnitFigConverter;

#define VERTEX_RADIUS       .2  // cm
#define VERTEX_THICKNESS     3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH        50  // xfig depth
#define EDGE_THICKNESS       5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH          51  // xfig depth
#define DATA_THICKNESS       3  // 1/80 inches (see xfig thickness)
#define DATA_DEPTH          52  // xfig depth
#define FRAME_DEPTH        999  // xfig depth
#define SPEED_SCALE        3.0
#define SPEED_THICKNESS      3
#define PLOT_SPEED_THICKNESS 6
#define SPEED_DEPTH         49
#define SPEED_DIRECTION  false // inverted
#define FRAME_MARGIN         2
void print(int     nb,
           Graph&  g, 
           Input&  input) {
  FigConverter converter;
  UnitFigConverter unit_converter(converter);
  std::ofstream file;
  vq2::xfig::GC gc;

  vq2::xfig::open("som",nb,file);

  // Let us draw an invisible frame surrounding the drawing.
  gc.depth = FRAME_DEPTH;
  gc.colorWhite();
  vq2::xfig::line(converter,file,gc,Point(-FRAME_MARGIN,-FRAME_MARGIN),Point(INPUT_AMPLITUDE+FRAME_MARGIN,-FRAME_MARGIN));
  vq2::xfig::line(converter,file,gc,Point(INPUT_AMPLITUDE+FRAME_MARGIN,-FRAME_MARGIN),Point(INPUT_AMPLITUDE+FRAME_MARGIN,1.5*FRAME_MARGIN));
  vq2::xfig::line(converter,file,gc,Point(INPUT_AMPLITUDE+FRAME_MARGIN,1.5*FRAME_MARGIN),Point(-FRAME_MARGIN,1.5*FRAME_MARGIN));
  vq2::xfig::line(converter,file,gc,Point(-FRAME_MARGIN,1.5*FRAME_MARGIN),Point(-FRAME_MARGIN,-FRAME_MARGIN));

  // Let us draw inputs;
  gc.depth = DATA_DEPTH;
  gc.thickness = DATA_THICKNESS;
  gc.colorBlack();
  std::vector<Point>::iterator iter,end;
  for(iter=input.samples.begin(),end=input.samples.end();
      iter != end;
      ++iter)
    vq2::xfig::point(converter,file,gc,*iter);

  // Let us draw the graph with the speeds.
  vq2::temporal::xfig::graph(unit_converter,file,g,
			     VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
			     EDGE_THICKNESS,EDGE_DEPTH,
			     SPEED_SCALE,SPEED_THICKNESS,SPEED_DEPTH,SPEED_DIRECTION);

  // Let us draw the speed graph.
  vq2::temporal::xfig::speeds(unit_converter,file,g,
			      Point(INPUT_AMPLITUDE/2,FRAME_MARGIN),
			      VERTEX_DEPTH,SPEED_DEPTH,PLOT_SPEED_THICKNESS,SPEED_SCALE,FRAME_MARGIN/2);
  
  vq2::xfig::close(file);
}

void generateMakefile(void) {
  std::ofstream file;
  
  file.open("Makefile");


  file << "FIG = $(wildcard som-*.fig)" << std::endl
       << "JPG = $(sort $(patsubst %.fig, %.jpg, $(FIG)))" << std::endl
       << std::endl
       << "help:" << std::endl
       << "\t@echo \"fig2dev and ffmpeg are needed.\"" << std::endl
       << "\t@echo" << std::endl
       << "\t@echo \"type one of the following commands\"" << std::endl
       << "\t@echo \"    make jpeg\"" << std::endl
       << "\t@echo \"    make clear\"" << std::endl
       << "\t@echo \"    make movie\"" << std::endl
       << "\t@echo" << std::endl
       << std::endl
       << "clear:" << std::endl
       << "\t@rm som-*.jpg som-*.fig" << std::endl
       << std::endl
       << "som-%.jpg : som-%.fig" << std::endl
       << "\t@fig2dev -L jpeg -q 100 $< $@" << std::endl
       << "\t@echo $@ \"generated.\"" << std::endl
       << std::endl
       << "jpeg : $(JPG)" << std::endl
       << std::endl
       << "movie : jpeg" << std::endl
       << "\t@rm -f som.avi" << std::endl
       << "\tffmpeg -r 8 -i som-%06d.jpg -b 1M som.avi" << std::endl
       << "\t@rm -f $(JPG) $(FIG) Makefile" << std::endl;

  file.close();
}

Point init_vertex(unsigned int w, unsigned int h) {
  return Point();
}

char init_edge(unsigned int w,  unsigned int h,
	       unsigned int ww, unsigned int hh) {
  return ' ';
}

#define GRID_SIDE              7
#define NB_TICKS             100
#define NB_EPOCHS_PER_TICK    10
#define LEARNING_RATE        .01
#define COMPETITION_COEF     .4
int main(int argc, char **argv){
  
  Graph          som;
  VectorOp       op;
  Similarity     distance;
  UnitSimilarity unit_distance(distance);
  Learn          learning_rule;
  UnitLearn      unit_learning_rule(learning_rule);
  WinnerTakeMost competition;
  Input          input;
  double         dt = 1.0;

  std::vector<Point>::iterator iter;
  
  vq2::proba::random::init();

  generateMakefile();

  vq2::algo::make::grid(som,GRID_SIDE,GRID_SIDE,
			init_vertex,init_edge);


  competition.coef = COMPETITION_COEF;
  int epoch,tick;

  // In order to avoid spurious speeds, due to the unfolding (and
  // sliding) of the map, let us allow the map to unfold on initial
  // frame, and then clean the speeds.
  for(epoch = 0; epoch < NB_EPOCHS_PER_TICK; ++epoch) {
    input.tick(0); // Let us re-sample at each epoch to avoid spatial biases.
    for(iter = input.samples.begin(); iter != input.samples.end(); ++iter) {
      vq2::algo::som::step(som,unit_distance,competition,
			   unit_learning_rule,
			   LEARNING_RATE,
			   *iter);
    }
  }
  vq2::temporal::clear_speeds(som); // Here, speeds are re-initialized.
  
  for(tick = 0; tick < NB_TICKS; ++tick) {
    std::cout << "Frame " << std::setw(4) << tick+1 
	      << '/' << NB_TICKS << "     \r" << std::flush;
    vq2::temporal::tick(som,op,dt); // Here, speeds are updated.
    for(epoch = 0; epoch < NB_EPOCHS_PER_TICK; ++epoch) {
      input.tick(tick); // Let us re-sample at each epoch to avoid spatial biases.
      for(iter = input.samples.begin(); iter != input.samples.end(); ++iter) {
	 vq2::algo::som::step(som,unit_distance,competition,
			      unit_learning_rule,
			      LEARNING_RATE,
			      *iter);
	 vq2::temporal::frame(som,op); // Here, the end of an epoch is notified for further speed-updated.
      }
    }
    
    print(tick,som,input);
  }
  std::cout << std::endl;

    
  std::cout << std::endl
	    << std::endl
	    << "type 'make' to make the movie." << std::endl;
  return 0;
}
