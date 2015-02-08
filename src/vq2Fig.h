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

#ifndef vq2FIG_H
#define vq2FIG_H

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vq2Concept.h>
#include <vq2Graph.h>

namespace vq2 {
  namespace concept {
    class XfigConverter {
    public:
      typedef Any position_type;
      
      /**
       * This converts pos into a point (x_cm,y_cm) in centimeters.
       */
      void locate(const position_type& pos,
		  double& x_cm, double& y_cm);
    };
  }

  namespace xfig {

    /**
     * Graphic contaxt for xfig
     */
    class GC {
    private:

      enum {nb_label = 10};

    public:

      typedef enum {
	modeLine,
	modeCircle
      } Mode;
      Mode mode;

      int line_style;
      int thickness;
      int pen_color;
      int fill_color;
      int depth;
      int area_style;
      double style_val;
      int join_style;
      int cap_style;
      int radius;
      int forward_arrow;
      int backward_arrow;

      GC(void) {
	clear();
      }

      int figUnit(double x_cm) {
	return (int)(450*x_cm);
      }

      void filled() {
	area_style = 20;
	style_val = 0;
      }

      void contour(void) {
	area_style = -1;
	style_val = 0;
      }

      void clear(void) {
	mode = modeLine;
	line_style = 0;
	thickness = 1;
	colorBlack();
	fill_color = 7;
	depth = 50;
	contour();
	join_style = 1;
	cap_style = 1;
	radius = -1;
	forward_arrow = 0;
	backward_arrow = 0;
      }

      void colorBlack(void)  {pen_color =  0;}
      void colorBlue(void)   {pen_color =  1;}
      void colorRed(void)    {pen_color =  4;}
      void colorYellow(void) {pen_color =  6;}
      void colorWhite(void)  {pen_color =  7;}
      void colorGreen(void)  {pen_color = 13;}
      void colorUnefficient(void) {pen_color = 32;}
      void colorLabel(unsigned int label) {
	if(label==0) {
	  colorBlack();
	  return;
	}

	label--;
	label = label % nb_label;
	pen_color = 33+(int)(label);
      }

      friend std::ostream& operator<<(std::ostream& os,
				      const GC& gc) {
	os << gc.line_style    << ' '
	   << gc.thickness     << ' '
	   << gc.pen_color     << ' '
	   << gc.fill_color    << ' '
	   << gc.depth         << ' '
	   << "-1 " // pen style, not used.
	   << gc.area_style    << ' ' << gc.style_val << ' ';

	if(gc.mode == modeLine) {
	  os << gc.join_style    << ' ' << gc.cap_style << ' '
	     << gc.radius        << ' '
	     << gc.forward_arrow << ' ' << gc.backward_arrow << ' ';
	}
	return os;
      }
    };
  }


  namespace xfig {
    template<typename XFIG_CONVERTER>
    void circle(XFIG_CONVERTER& convert,std::ofstream& file, GC& gc,
		const typename XFIG_CONVERTER::position_type& O,
		double R) {
      double x,y;
      
      convert.locate(O,x,y);
      gc.mode = GC::modeCircle;
      file << "1 3 " << gc 
	   << "1 0 " 
	   << gc.figUnit(x) << ' ' << gc.figUnit(y) << ' '
	   << gc.figUnit(R) << ' ' << gc.figUnit(R) << ' '
	   << gc.figUnit(x) << ' ' << gc.figUnit(y) << ' '
	   << gc.figUnit(x)+ gc.figUnit(R) << ' ' << gc.figUnit(y) << std::endl;
    }

    
    template<typename XFIG_CONVERTER>
    void line(XFIG_CONVERTER& convert,
	      std::ofstream& file, GC& gc,
	      const typename XFIG_CONVERTER::position_type& A,
	      const typename XFIG_CONVERTER::position_type& B) {
      double xa,ya,xb,yb;
      
      convert.locate(A,xa,ya);
      convert.locate(B,xb,yb);
      gc.mode = GC::modeLine;
      file << "2 1 " << gc << "2 " << std::endl
	   << "    " 
	   << gc.figUnit(xa) << ' ' << gc.figUnit(ya) << ' '
	   << gc.figUnit(xb) << ' ' << gc.figUnit(yb) << std::endl;
    }

    template<typename XFIG_CONVERTER>
    void point(XFIG_CONVERTER& convert,
	       std::ofstream& file, GC& gc,
	       const typename XFIG_CONVERTER::position_type& A) {
      double x,y;
      
      convert.locate(A,x,y);
      gc.mode = GC::modeCircle;
      int R = gc.thickness+1;
      file << "1 3 " << gc 
	   << "1 0 " 
	   << gc.figUnit(x) << ' ' << gc.figUnit(y) << ' '
	   << R << ' ' << R << ' '
	   << gc.figUnit(x) << ' ' << gc.figUnit(y) << ' '
	   << gc.figUnit(x)+R << ' ' << gc.figUnit(y) << std::endl;
    }

    inline void open(const std::string& filename,
		     std::ofstream& file) {
      file.close();
      file.clear();
      file.open(filename.c_str());
      file << "#FIG 3.2  Produced by vq2" << std::endl
	   << "Portrait" << std::endl
	   << "Center" << std::endl
	   << "Metric" << std::endl
	   << "A4" << std::endl 
	   << "100.00" << std::endl
	   << "Single" << std::endl
	   << "-2" << std::endl
	   << "1200 2" << std::endl
	   << "0 32 #bebebe" << std::endl
	   << "0 33 #ff0000" << std::endl
	   << "0 34 #00b700" << std::endl
	   << "0 35 #0000ff" << std::endl
	   << "0 36 #9400ff" << std::endl
	   << "0 37 #ff8a00" << std::endl
	   << "0 38 #0083ff" << std::endl
	   << "0 39 #ff0aa2" << std::endl
	   << "0 40 #916bff" << std::endl
	   << "0 41 #79ac00" << std::endl
	   << "0 42 #8a6700" << std::endl;
    }

    inline void open(const std::string& fileprefix, 
		     unsigned int rank,
		     std::ofstream& file) {
      std::ostringstream ostr;
      ostr << fileprefix << '-' << std::setfill('0')
	   << std::setw(6) << rank << ".fig";
      open(ostr.str(),file);
    }

    inline void close(std::ofstream& file) {
      file.close();
    }
  }
  
  namespace functor {

    template<typename XFIG_CONVERTER, typename GRAPH>
    class FigEdge  {
    private:
      XFIG_CONVERTER& c;
      std::ofstream& f;
      xfig::GC gc;

    public:

      FigEdge(XFIG_CONVERTER& convert,
	      std::ofstream& file,
	      int edge_thickness, int edge_depth) 
	: c(convert), f(file),gc() {
	gc.mode      = xfig::GC::modeLine;
	gc.depth     = edge_depth;
	gc.thickness = edge_thickness;
      }

      bool operator()(typename GRAPH::edge_type& e) {
	if(e.stuff.efficient)
	  gc.colorLabel(e.stuff.label);
	else
	  gc.colorUnefficient();
	xfig::line(c,f,gc,(*(e.n1)).value,(*(e.n2)).value);
	return false;
      }
    };

    template<typename XFIG_CONVERTER, typename GRAPH>
    class FigVertex  {
    protected:
      XFIG_CONVERTER& c;
      std::ofstream& f;
      double vr;
      xfig::GC gc;

    public:

      FigVertex(XFIG_CONVERTER& convert,
		std::ofstream& file,
		double vertex_radius, int vertex_thickness, int vertex_depth) 
	: c(convert), f(file),
	  vr(vertex_radius), 
	  gc() {
	gc.mode      = xfig::GC::modeCircle;
	gc.depth     = vertex_depth;
	gc.thickness = vertex_thickness;
	gc.filled();
      }

      bool operator()(typename GRAPH::vertex_type& v) {
	if(v.stuff.efficient)
	  gc.colorLabel(v.stuff.label);
	else
	  gc.colorUnefficient();
	xfig::circle(c,f,gc,v.value,vr);
	return false;
      }
    };

  }

  namespace xfig {
    
    template<typename XFIG_CONVERTER, typename GRAPH>
    void graph(XFIG_CONVERTER& convert,
	       std::ofstream& file,
	       GRAPH& g,
	       double vertex_radius, int vertex_thickness, int vertex_depth,
	       int edge_thickness, int edge_depth) {
      vq2::functor::FigVertex<XFIG_CONVERTER,GRAPH> fig_vertex(convert,file,
							       vertex_radius,
							       vertex_thickness,
							       vertex_depth);
      vq2::functor::FigEdge<XFIG_CONVERTER,GRAPH> fig_edge(convert,file,
							   edge_thickness,
							   edge_depth);
      g.for_each_vertex(fig_vertex);
      g.for_each_edge(fig_edge);
    }

  }
}


#endif
