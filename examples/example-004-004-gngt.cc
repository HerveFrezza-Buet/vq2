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
// the vector operation functor is obtained easily. This is usefull
// for density related tools of this example.
typedef vq2::by_default::VectorOp<Point> VectorOp;

// This is the GNGT neurons.
typedef vq2::algo::gngt::Unit<Point> Unit;
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

#define NB_SAMPLES 10000
#define TARGET      1e-4

// This is the parameter set for GNG-T
class Params {
public:
  // GNG-T
  int ageMax(void)           {return 20;}
  double learningRate(void)  {return .005;}
  double learningRatio(void) {return .2;}
  double lambda(void)        {return .001;}

  // Evolution
  double target(void)        {return TARGET;}
  double nbSamples(void)     {return NB_SAMPLES;}
  double lowPassCoef(void)   {return .4;}
  double delta(void)         {return .75;}
  double margin(void)        {return .2;}
};

typedef vq2::by_default::gngt::Evolution<Params> Evolution;

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

 
  Density&    disk; 
  Density&    small_disk;
  Density&    hole;
  Density&    crown;
  Translate&  crown1;
  Translate&  crown2;
  Density&    noise;
  Density&    glue1;
  Density&    glue2;

public:

  std::vector<Point> samples;

  Input(VectorOp& op) 
    : disk(*(new Disk(1.0))),
      small_disk(vq2::proba::scale(disk,op,.7)),
      hole(vq2::proba::invert(small_disk)),
      crown(vq2::proba::min(disk,hole)),
      crown1(vq2::proba::translate(crown,op,Point())),
      crown2(vq2::proba::translate(crown,op,Point())),
      noise(*(new Box(Point(-3,-2),3,4,.05))),
      glue1(vq2::proba::max(crown1,crown2)),
      glue2(vq2::proba::max(glue1,noise)) {
  }
  
  ~Input(void) {
    delete &disk;
    delete &small_disk;
    delete &hole;
    delete &crown;
    delete &crown1;
    delete &crown2;
    delete &noise;
    delete &glue1;
    delete &glue2;
  }

  void acquireSamples(int t) {
    Point p;
    double radius = sin(.05*t);
    crown1.translation.x = 2*radius;
    crown2.translation.x = -2*radius;

    samples.clear();
    for(int i=0; i<NB_SAMPLES; ++i) {
      p.x = vq2::proba::random::uniform(-3,3);
      p.y = vq2::proba::random::uniform(-3,3);
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

// This class invalidates long edges. This is a for_each_edge functor.
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

#define NB_TICKS             200
int main(int argc, char* argv[]) {
  Graph g;
  Params           params;
  Similarity       distance;
  UnitSimilarity   unit_distance(distance);
  Learn            learn;
  UnitLearn        unit_learn(learn);
  Evolution        evolution(params);
  VectorOp         op;
  Input            input(op);
  int              epoch,tick;
  

  generateMakefile();

  vq2::proba::random::init();

  for(tick = 0; tick < NB_TICKS; ++tick) {
    input.acquireSamples(tick);
    ReshuffleSamples reshuffler(input.samples);
    vq2::algo::gngt::epoch(params,g,
			   unit_distance,unit_learn,
			   evolution,reshuffler,
			   [] (const Point& p) -> const Point& {return p;}, // A sample_of lambda function.
			   10);

    
    // First, we invalidate long edges.
    InvalidateLongEdge invalidate_long_edge;
    g.for_each_edge(invalidate_long_edge);
    // Second, we invalidate any vertex that owns only invalid edges.
    InvalidateNoisyVertex invalid_noisy_vertex;
    g.for_each_vertex(invalid_noisy_vertex);
    // Third, we labelize the connected components.
    std::map<unsigned int,Graph::Component*> components;
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



#define VERTEX_RADIUS     .2  // cm
#define VERTEX_THICKNESS   3  // 1/80 inches (see xfig thickness)
#define VERTEX_DEPTH      50  // xfig depth
#define EDGE_THICKNESS     5  // 1/80 inches (see xfig thickness)
#define EDGE_DEPTH        51  // xfig depth
#define DATA_THICKNESS     2  // 1/80 inches (see xfig thickness)
#define DATA_DEPTH        52  // xfig depth
#define FRAME_DEPTH      999  // xfig depth
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
  vq2::xfig::line(converter,file,gc,Point(-3.5,-2.5),Point(-3.5, 2.5));
  vq2::xfig::line(converter,file,gc,Point(-3.5, 2.5),Point( 3.5, 2.5));
  vq2::xfig::line(converter,file,gc,Point( 3.5, 2.5),Point( 3.5,-2.5));
  vq2::xfig::line(converter,file,gc,Point( 3.5,-2.5),Point(-3.5,-2.5));

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
  vq2::xfig::graph(unit_converter,file,g,
		   VERTEX_RADIUS,VERTEX_THICKNESS,VERTEX_DEPTH,
		   EDGE_THICKNESS,EDGE_DEPTH);
  
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
       << "\tffmpeg -i gngt-%06d.jpg -b 1M gngt.avi" << std::endl
       << "\t@rm -f $(JPG) $(FIG) Makefile" << std::endl;

  file.close();

}
