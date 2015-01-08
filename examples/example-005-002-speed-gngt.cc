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
// Our GNG-T will contain 2D points as prototypes.
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

// This is the GNGT neurons,temporal management is added.
typedef vq2::algo::gngt::Unit<Point>  GNGTUnit;
typedef vq2::temporal::Unit<GNGTUnit> Unit;
// Do not forget the vertex initialization functor (3rd argument).
typedef vq2::Graph<Unit,char,Unit::copy_constructor>    Graph;


// For finding the winner in the GNGT algorithm, we need some
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
    x_cm = 21.0/2 + 2*pos.x;
    y_cm = 29.7/2 + 2*pos.y;
  }
};
typedef vq2::unit::XfigConverter<Unit,FigConverter> UnitFigConverter;

#define NB_SAMPLES 20000
#define TARGET      5e-5

// This is the parameter set for GNG-T
class Params {
public:
  // GNG-T
  int ageMax(void)           {return 20;}
  double lambda(void)        {return .001;}
  double learningRatio(void) {return .2;}

  // Evolution
  double target(void)        {return TARGET;}
  int nbSamples(void)        {return NB_SAMPLES;}
  double lowPassCoef(void)   {return .4;}
  double delta(void)         {return .75;}
  double margin(void)        {return .2;}
};

class ParamsSlow : public Params {
public:
  double learningRate(void)  {return 5e-4;}
};

class ParamsFast : public Params {
public:
  double learningRate(void)  {return 5e-3;}
};

typedef vq2::by_default::gngt::Evolution<Params> Evolution;


#define NB_TICKS             300

// Let us gather density manipulations in some class.
class Input {
private:
  typedef vq2::proba::Density<Point>            Density;
  typedef vq2::proba::Translate<Point,VectorOp> Translate;

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

 
  Density&    unity_disk; 
  Density&    big_disk; 
  Density&    small_disk;
  Density&    hole;
  Density&    crown;
  Translate&  crown1;
  Translate&  crown2;
  Box&        noise;
  Density&    glue1;
  Density&    glue2;

public:

  std::vector<Point> samples;

  Input(VectorOp& op) 
    : unity_disk(*(new Disk(1.0))),
      big_disk(vq2::proba::scale(unity_disk,op,.8)),
      small_disk(vq2::proba::scale(unity_disk,op,.5)),
      hole(vq2::proba::invert(small_disk)),
      crown(vq2::proba::min(big_disk,hole)),
      crown1(vq2::proba::translate(crown,op,Point())),
      crown2(vq2::proba::translate(crown,op,Point())),
      noise(*(new Box(Point(-3.5,-3.5),7,7,.05))),
      glue1(vq2::proba::max(crown1,crown2)),
      glue2(vq2::proba::max(glue1,(Density&)noise)) {
  }
  
  ~Input(void) {
    delete &unity_disk;
    delete &big_disk;
    delete &small_disk;
    delete &hole;
    delete &crown;
    delete &crown1;
    delete &crown2;
    delete &noise;
    delete &glue1;
    delete &glue2;
  }

  double phase(int t) {
    return fabs(-1+2*(t%50)/49.0);
  }

  void acquireSamples(int t) {
    Point p;
    double phi1 = phase(t+12);
    double phi2 = phase(t+12);
    crown1.translation.x = -2;
    crown1.translation.y = 2.5*(-1 + 2*phi1);
    crown2.translation.x = 2.5*(-1 + 2*phi2);
    crown2.translation.y = 0;

    if(t>=NB_TICKS/2)
      noise.density=0;

    samples.clear();
    for(int i=0; i<10000; ++i) {
      p.x = vq2::proba::random::uniform(-3.5,3.5);
      p.y = vq2::proba::random::uniform(-3.5,3.5);
      if(vq2::proba::random::toss(glue2(p)))
	samples.push_back(p);
    }
  }
};

// These are some functions used inside the main, and defined
// afterwards.
std::string toString(int nb);
void print(int     nb,
	   Graph&  g, 
	   Input&  input); 
void generateMakefile(void);




// This class invalidates long edges (and validates short ones). This
// is a for_each_edge functor.
#define MAX_VALID_DISTANCE .5
#define MAX_VALID_SQUARED_DISTANCE (MAX_VALID_DISTANCE*MAX_VALID_DISTANCE)
class InvalidateLongEdge {
public:
  bool operator()(Graph::edge_type& e) { 
    Similarity squared_dist;
    Point& A = (*(e.n1)).value.prototype(); // This is the way to retrieve the weight
    Point& B = (*(e.n2)).value.prototype();
    e.stuff.efficient = squared_dist(A,B) < MAX_VALID_SQUARED_DISTANCE;
    return false; // We invalidate, but we keep the edge.
  }
};


// This tells whether some valid edges exist.
class IsThereValidEdges {
public:
  bool is_any_valid;

  IsThereValidEdges(void) : is_any_valid(false) {}
  bool operator()(Graph::edge_type& e) { 
    is_any_valid = is_any_valid || e.stuff.efficient;
    return false;
  }
};

// This invalidates a vertex if all its edges are invalid.
class InvalidateNoisyVertex {
public:
  bool is_any_valid;

  bool operator()(Graph::vertex_type& v) { 
    IsThereValidEdges search;
    v.for_each_edge(search);
    v.stuff.efficient = search.is_any_valid;
    return false;
  }
};

// We need a class that re-shuffles the examples for each new GNG-T epoch.
class ReshuffleSamples {
private:
  std::vector<Point>& samples;
  
public:
  ReshuffleSamples(std::vector<Point>& s) : samples(s) {}
  std::vector<Point>::iterator begin(void) {
    std::random_shuffle(samples.begin(),samples.end());
    return samples.begin();
  }
  
  std::vector<Point>::iterator end(void) {
    return samples.end();
  }
};

int main(int argc, char* argv[]) {
  Graph g;
  ParamsSlow       params_slow;
  ParamsFast       params_fast;
  Similarity       distance;
  UnitSimilarity   unit_distance(distance);
  Learn            learn;
  UnitLearn        unit_learn(learn);
  VectorOp         op;
  Evolution        evolution(params_fast);
  Input            input(op);
  int              epoch,tick;
  double           dt = 1.0;
  
  

  generateMakefile();

  vq2::proba::random::init();

  for(tick = 0; tick < NB_TICKS; ++tick) {

    input.acquireSamples(tick);
    ReshuffleSamples reshuffler(input.samples);
    vq2::algo::gngt::temporal::epoch(params_fast,params_slow,
				     g,
				     unit_distance,unit_learn,
				     evolution,op,
				     reshuffler,
				     [] (const Point& p) -> const Point& {return p;}, 
				     dt,5,5,10);

    // First, we invalidate long edges.
    InvalidateLongEdge invalidate_long_edge;
    g.for_each_edge(invalidate_long_edge);
    // Second, we invalidate any vertex that owns only invalid edges.
    InvalidateNoisyVertex invalid_noisy_vertex;
    g.for_each_vertex(invalid_noisy_vertex);
    // Third, we labelize the connected components.
    std::map<unsigned int,Graph::Component*> components;
    // Last, we re-labelize the graph and print it.
    g.computeConnectedComponents(components,true);
    print(tick,g,input);
  }
  std::cout << std::endl
	    << std::endl
	    << "type 'make' to make the movie." << std::endl;
}


std::string toString(int nb) {
  std::ostringstream ostr;
  ostr << std::setw(6) << std::setfill('0') << nb;
  return ostr.str();
}



 
#define VERTEX_RADIUS       .2  // cm
#define VERTEX_THICKNESS     3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH        50  // xfig depth
#define EDGE_THICKNESS       5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH          51  // xfig depth
#define DATA_THICKNESS       2  // 1/80 inches (see xfig thickness)
#define DATA_DEPTH          52  // xfig depth
#define FRAME_DEPTH        999  // xfig depth
#define GRAPH_SPEED_SCALE  3.0
#define PLOT_SPEED_SCALE   4.0
#define SPEED_THICKNESS      3
#define PLOT_SPEED_THICKNESS 6
#define SPEED_DEPTH         49
#define SPEED_DIRECTION  false // inverted
#define MARGIN               2
void print(int     nb,
	   Graph&  g, 
	   Input&  input) {
  FigConverter converter;
  UnitFigConverter unit_converter(converter);
  std::ofstream file;
  vq2::xfig::GC gc;

  vq2::xfig::open("gngt",nb,file);

  // Let us draw an invisible frame surrounding the drawing.
  gc.depth = FRAME_DEPTH;
  gc.colorWhite();
  vq2::xfig::line(converter,file,gc,
		  Point(-3-MARGIN,-3-MARGIN),Point(-3-MARGIN, 3+MARGIN));
  vq2::xfig::line(converter,file,gc,
		  Point(-3-MARGIN, 3+MARGIN),Point( 8+MARGIN, 3+MARGIN));
  vq2::xfig::line(converter,file,gc,
		  Point( 8+MARGIN, 3+MARGIN),Point( 8+MARGIN,-3-MARGIN));
  vq2::xfig::line(converter,file,gc,
		  Point( 8+MARGIN,-3-MARGIN),Point(-3-MARGIN,-3-MARGIN));

  // Let us draw inputs;
  gc.depth = DATA_DEPTH;
  gc.thickness = DATA_THICKNESS;
  gc.colorBlack();
  std::vector<Point>::iterator iter,end;
  for(iter=input.samples.begin(),end=input.samples.end();
      iter != end;
      ++iter)
    vq2::xfig::point(converter,file,gc,*iter);

  // Let us draw the graph
  vq2::temporal::xfig::graph(unit_converter,file,g,
			     VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
			     EDGE_THICKNESS,EDGE_DEPTH,
			     GRAPH_SPEED_SCALE,SPEED_THICKNESS,SPEED_DEPTH,SPEED_DIRECTION);

  // Let us draw the speed graph.
  vq2::temporal::xfig::speeds(unit_converter,file,g,
			      Point(6,0),
			      VERTEX_DEPTH,SPEED_DEPTH,PLOT_SPEED_THICKNESS,PLOT_SPEED_SCALE,
			      2);
  
  vq2::xfig::close(file);

  std::cout << "Frame " << std::setw(3) << nb+1 << '/' << NB_TICKS << "     \r" << std::flush;
  if(nb == NB_TICKS-1)
    std::cout << std::endl;
}

void generateMakefile(void) {
  std::ofstream file;
  
  file.open("Makefile");


  file << "FIG = $(wildcard gngt-*.fig)" << std::endl
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
       << "\t@rm gngt-*.jpg gngt-*.fig" << std::endl
       << std::endl
       << "gngt-%.jpg : gngt-%.fig" << std::endl
       << "\t@fig2dev -L jpeg -q 100 $< $@" << std::endl
       << "\t@echo $@ \"generated.\"" << std::endl
       << std::endl
       << "jpeg : $(JPG)" << std::endl
       << std::endl
       << "movie : jpeg" << std::endl
       << "\t@rm -f gngt.avi" << std::endl
       << "\tffmpeg -r 10 -i gngt-%06d.jpg -b 1M gngt.avi" << std::endl
       << "\t@rm -f $(JPG) $(FIG) Makefile" << std::endl;

  file.close();

}
