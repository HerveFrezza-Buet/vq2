/*   This file is part of vq2
 *
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Herve Frezza-Buet
 *
 *   Contributor : Fabien Campos
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


#ifndef vq2_SPEED_H
#define vq2_SPEED_H

#include <iostream>
#include <iomanip>

#include <vq2Functors.h>
#include <vq2Fig.h>
// #define vq2DEBUG_SPEED
// #define vq2DEBUG_PROTO_UPDATE

namespace vq2 {
  namespace temporal {

    typedef enum Status {
      statusNew,
      statusNoPreviousProto,
      statusOk
    } Status;
    

    template<typename UNIT>
    class Unit: public UNIT {
    public:

      typedef typename UNIT::prototype_type prototype_type;
      typedef UNIT super_type;

#ifdef vq2DEBUG_SPEED
      std::string printStatus(void) {
	switch(status) {
	case statusNew: return "New"; break;
	case statusNoPreviousProto: return "NoPrev"; break;
	case statusOk: return "Ok"; break;
	default: return "???"; break;
	}
      }

      void print(void) {
	std::cout << "    status = " << printStatus() << "                                       " << this << std::endl
		  << "         s = (" << dproto.x << ", " << dproto.y << ")                                       " << this << std::endl
		  << "  s.module = " << sqrt(dproto.x*dproto.x+dproto.y*dproto.y) << "                                       " << this << std::endl
		  << "     proto = (" << this->prototype().x << ", " << this->prototype().y << ")                                       " << this << std::endl
		  << "    proto_ = (" << proto_.x << ", " << proto_.y << ")                                       " << this << std::endl
		  << " ex-proto_ = (" << previousProto_.x << ", " << previousProto_.y << ")                                       " << this << std::endl;
      }
#endif

    private :

      prototype_type previousProto_;
      prototype_type proto_;
      prototype_type dproto;
      Status status;
      int count;

    public:

      Unit(void) 
	: super_type(),  
	  previousProto_(), proto_(),
	  dproto(),
	  status(statusNew),
	  count(0) {
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : Unit()" << std::endl;
#endif
      }
      Unit(const Unit<UNIT>& copy) 
	: super_type(copy), 
	  previousProto_(copy.previousProto_), 
	  proto_(copy.proto_),
	  dproto(copy.dproto),
	  status(copy.status),
	  count(copy.count) {
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : Unit(const Unit<UNIT>&)" << std::endl;
	print();
#endif
}
      Unit(const prototype_type& proto) 
	: super_type(proto),
	  previousProto_(proto), proto_(proto),
	  dproto(),
	  status(statusNew),
	  count(0) {
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : Unit(const prototype_type&)" << std::endl;
#endif
}

      Unit<UNIT>& operator=(const Unit& copy) {
	if(this != &copy) {
	  this->super_type::operator=(copy);
	  previousProto_ = copy.previousProto_; 
	  proto_         = copy.proto_;
	  dproto         = copy.dproto; 
	  status         = copy.status;
	  count          = copy.count;
	}
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : operator=(const Unit&)" << std::endl;
	print();
#endif
	return *this;
      }

      Unit<UNIT>& operator=(const prototype_type& proto) {
	this->super_type::operator=(proto);
	previousProto_(proto);
	proto_(proto);
	status = statusNew;
	dproto = prototype_type();
	count = 0;
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : operator=(const prototype_type&)" << std::endl;
#endif
	return *this;
      }

      void clearSpeed(void) {
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : clearSpeed" << std::endl;
#endif
	previousProto_ = prototype_type();
	proto_         = prototype_type();
	dproto         = prototype_type();
	status         = statusNew;
	count          = 0;
      }

      /*
       * @return speed vector
       *
       */
      const prototype_type& speed(void) const {
        return dproto;
      }

      Status getStatus(void) const {
	return status;
      }
		    

      void onNewSpeedUpdate(const prototype_type& neighbourMeanSpeed) {
	status = statusNoPreviousProto;

	dproto         = neighbourMeanSpeed;
	proto_         = prototype_type();
	previousProto_ = prototype_type();
	count          = 0;
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : onNewSpeedUpdate" << std::endl;
	print();
#endif
      }	    

      template <typename VECTOR_OP>
      void onNoPrevSpeedUpdate(VECTOR_OP& op, const prototype_type& neighbourMeanSpeed) {
	status = statusOk;

	if(count <= 0)
	  clearSpeed();
	else {
	  dproto = neighbourMeanSpeed;
	  op.div(proto_,(double)count);
	  previousProto_ = proto_;
	  proto_         = prototype_type();
	  count          = 0;
	}
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : onNoPrevSpeedUpdate" << std::endl;
	print();
#endif
      }

      template <typename VECTOR_OP>
      void onSpeedUpdate(VECTOR_OP& op, double dt) {
#ifdef vq2DEBUG_SPEED
	std::cout << this << " : onSpeedUpdate" << std::endl;
#endif
	if(count <= 0 || dt <= 0)
	  clearSpeed();
	else {
	  op.div(proto_,(double)count);
	  dproto = previousProto_;
	  op.mul(dproto,-1);
	  op.add(dproto,proto_);
	  op.div(dproto,dt);

	  // double norm = sqrt(dproto.x*dproto.x+dproto.y*dproto.y);
	  // if(norm > 100)
	  //   std::cout << "Speed  = " << norm << std::endl
	  // 	      << "  prev = " << previousProto_.x << ' ' << previousProto_.y << std::endl
	  // 	      << "  prot = " << proto_.x << ' ' << proto_.y << std::endl
	  // 	      << "  dt   = " << dt << std::endl
	  // 	      << "  v    = " << dproto.x << ' ' << dproto.y << std::endl;

	  previousProto_ = proto_;
	  proto_ = prototype_type(); // Default constructor has to provide 0.
	  count = 0;

	}
#ifdef vq2DEBUG_SPEED
	print();
#endif
      }


      template <typename VECTOR_OP>
      void onProtoUpdate(VECTOR_OP& op) {
#ifdef vq2DEBUG_PROTO_UPDATE
	std::cout << this << " : onProtoUpdate" << std::endl;
#endif
	if(status != statusNew) {
	  op.add(proto_,this->prototype());
	  ++count;
#ifdef vq2DEBUG_PROTO_UPDATE
	  std::cout << "   curr   = (" << this->prototype().x << ", " <<  this->prototype().y << ")" 
		    << "                                       " << this << std::endl
		    << "   curr_  = (" << proto_.x/(double)count << ", " <<  proto_.y/(double)count << ")" 
		    << "                                       " << this << std::endl
		    << "   count = " << count 
		    << "                                       " << this << std::endl;
#endif
	}
#ifdef vq2DEBUG_PROTO_UPDATE
	else
	  std::cout << "   statusNew skipped." 
		    << "                                       " << this << std::endl;
#endif
	
      }



      class copy_constructor {
      public:
        void operator()(Unit<UNIT>& that,
                        const Unit<UNIT>& copy){
	  that = copy;
        }
      };

      friend class copy_constructor;
    };


    /**
     * @short Functor for internal use.
     */
    template<typename GRAPH>
    class ClearSpeed
    {
    public:
      bool operator()(typename GRAPH::vertex_type& n) {
        n.value.clearSpeed();
        return false; 
      }
    };
    
    /**
     * @short Functor for internal use.
     */
    template<typename GRAPH,typename VECTOR_OP>
    class NeighborsMeanSpeed {
      VECTOR_OP& op;
      int count;
      const typename GRAPH::vertex_type& currentVertex_;
      typename GRAPH::vertex_value_type::prototype_type speed_;

    public:

      NeighborsMeanSpeed(VECTOR_OP& vop,
			 const typename GRAPH::vertex_type& currentVertex)
        :op(vop),count(0),currentVertex_(currentVertex),speed_() {}
      
      const typename GRAPH::vertex_value_type::prototype_type& get(void) {
	if(count != 0)
	  op.div(speed_,(double)count);
	return speed_;
      }

      bool operator()(typename GRAPH::edge_type& e) {
        typename GRAPH::vertex_value_type *pUnit;

	if(e.stuff.efficient) {
	  if(&(*(e.n2)) != &currentVertex_)
	    pUnit = &((*(e.n2)).value);
	  else
	    pUnit = &((*(e.n1)).value);
	  
	  if(pUnit->getStatus() == statusOk) {
	    op.add(speed_,pUnit->speed());
	    ++count;
	  }
	}
        return false;
      }
    };


    /**
     * @short Functor for internal use.
     */
    template<typename GRAPH, typename VECTOR_OP>
    class Tick {
      VECTOR_OP& op;
      double dt;
      
    public:
      Tick(VECTOR_OP& vop, double delta_t)
        :op(vop),dt(delta_t) {}
      
      bool operator()(typename GRAPH::vertex_type& n) {
	NeighborsMeanSpeed<GRAPH,VECTOR_OP> mean(op,n);
	switch(n.value.getStatus()) {
	case statusNew:
	  n.for_each_edge(mean);
	  n.value.onNewSpeedUpdate(mean.get());
	  break;
	case statusNoPreviousProto:
	  n.for_each_edge(mean);
	  n.value.onNoPrevSpeedUpdate(op,mean.get());
	  break;
	case statusOk:
	  n.value.onSpeedUpdate(op,dt);
	  break;
	default:
	  std::cerr << "Error : Tick : bad status. Report bug." << std::endl;
	  break;
	}
        return false; // the element should not be removed.
      }
    };

    /**
     * @short Functor for internal use.
     */
    template<typename GRAPH, typename VECTOR_OP>
    class Frame {
      VECTOR_OP& op;
      
    public:
      Frame(VECTOR_OP& vop)
        :op(vop) {}
      
      bool operator()(typename GRAPH::vertex_type& n) {
        n.value.onProtoUpdate(op);
        return false; // the element should not be removed.
      }
    };

    
    template<typename GRAPH, typename VECTOR_OP>
    void tick(GRAPH& g, VECTOR_OP& op, double dt) {
#ifdef vq2DEBUG_SPEED
      static int i = 0;
      std::cout << "###.x.y.module### tick " << i++ << std::endl;
#endif 
      Tick<GRAPH,VECTOR_OP> tick(op,dt);
      g.for_each_vertex(tick);
    }
    
    template<typename GRAPH, typename VECTOR_OP>
    void frame(GRAPH& g, VECTOR_OP& op) {
#ifdef vq2DEBUG_SPEED
      static int i = 0;
      std::cout << "###.x.y.module### frame " << i++ << std::endl;
#endif 
      Frame<GRAPH,VECTOR_OP> frame(op);
      g.for_each_vertex(frame);
    }

    template<typename GRAPH>
    void clear_speeds(GRAPH& g) {
      ClearSpeed<GRAPH> clear;
      g.for_each_vertex(clear);
    }

    namespace xfig {
      namespace functor{

	template<typename XFIG_CONVERTER, typename GRAPH>
	class FigVertex  : public vq2::functor::FigVertex<XFIG_CONVERTER,GRAPH> {
	  vq2::xfig::GC sp_gc;
	  double sp_scale;
	  bool sp_direction;

	public:

	  typedef vq2::functor::FigVertex<XFIG_CONVERTER,GRAPH> super_type;

	  FigVertex(XFIG_CONVERTER& convert, std::ofstream& file,
		    double vertex_radius, int vertex_thickness,
		    int vertex_depth, double speed_scale,
		    int speed_thickness, int speed_depth, bool speed_direction)
	    : super_type(convert,file,
			 vertex_radius,
			 vertex_thickness,vertex_depth),
	      sp_gc(),
	      sp_scale(speed_scale),
	      sp_direction(speed_direction) {
	    sp_gc.depth     = speed_depth;
	    sp_gc.thickness = speed_thickness;
	    sp_gc.colorRed();
	  }


	  bool operator()(typename GRAPH::vertex_type& v) {
	    this->super_type::operator()(v);
	    
	    typename GRAPH::vertex_value_type& vp = v.value;
	    typename GRAPH::vertex_value_type::prototype_type speed;
	      
	    speed = vp.speed();
	    speed *= (sp_direction?1.0:-1.0);
	    speed *= sp_scale;
	    speed += vp.prototype();
	      
	    vq2::xfig::line(this->c,this->f,sp_gc,
			    vp,
			    typename GRAPH::vertex_value_type(speed));

	    return false;
	  }
	};

	template<typename XFIG_CONVERTER, typename GRAPH>
	class Speeds {
	  vq2::xfig::GC gc;
	  double scale;
	  std::ofstream& f;
	  XFIG_CONVERTER& c;
	  const typename GRAPH::vertex_value_type::prototype_type& o;

	public:


	  Speeds(XFIG_CONVERTER& convert, std::ofstream& file,
		 const typename GRAPH::vertex_value_type::prototype_type& origin,
		 int speed_depth,
		 double speed_scale,
		 int thickness)
	    : gc(),
	      scale(speed_scale),
	      f(file),
	      c(convert),
	      o(origin) {
	    gc.depth  = speed_depth;
	    gc.thickness = thickness;
	  }


	  bool operator()(typename GRAPH::vertex_type& v) {
	    typename GRAPH::vertex_value_type& vp = v.value;
	    typename GRAPH::vertex_value_type::prototype_type speed;

	    speed = vp.speed();
	    speed *= scale;
	    speed += o;
	    if(v.stuff.efficient)
	      gc.colorLabel(v.stuff.label);
	    else
	      gc.colorUnefficient();
	    vq2::xfig::point(c,f,gc,
			     typename GRAPH::vertex_value_type(speed));

	    return false;
	  }
	};
      }

      template<typename XFIG_CONVERTER, typename GRAPH>
      void graph(XFIG_CONVERTER& convert, std::ofstream& file,
		 GRAPH& g, double vertex_radius, int vertex_thickness,
		 int vertex_depth, int edge_thickness, int edge_depth,
		 double speed_scale, int speed_thickness, int speed_depth,
		 bool speed_direction) {
	vq2::temporal::xfig::functor::FigVertex<XFIG_CONVERTER,GRAPH> fig_vertex(convert,file,
										 vertex_radius,vertex_thickness,vertex_depth,
										 speed_scale,speed_thickness,speed_depth,speed_direction);
	
	vq2::functor::FigEdge<XFIG_CONVERTER,GRAPH> fig_edge(convert,file,
							     edge_thickness, edge_depth);
	g.for_each_vertex(fig_vertex);
	g.for_each_edge(fig_edge);
      }

      template<typename XFIG_CONVERTER, typename GRAPH>
      void speeds(XFIG_CONVERTER& convert, std::ofstream& file,
		  GRAPH& g,
		  const typename GRAPH::vertex_value_type::prototype_type& origin,
		  int axis_depth, int speed_depth, int speed_thickness,
		  double speed_scale,
		  double amplitude) {
	vq2::xfig::GC gc;
	vq2::temporal::xfig::functor::Speeds<XFIG_CONVERTER,GRAPH> speeds(convert,file,
									  origin,speed_depth,speed_scale,speed_thickness);

	gc.depth = axis_depth;
	gc.thickness = 1;
	gc.colorBlack();

	typename GRAPH::vertex_value_type::prototype_type A,B;

	A = typename GRAPH::vertex_value_type::prototype_type(-amplitude,0);
	B = typename GRAPH::vertex_value_type::prototype_type(amplitude,0);
	A += origin;
	B += origin;
	vq2::xfig::line(convert,file,gc,
			typename GRAPH::vertex_value_type(A),
			typename GRAPH::vertex_value_type(B));

	A = typename GRAPH::vertex_value_type::prototype_type(0,-amplitude);
	B = typename GRAPH::vertex_value_type::prototype_type(0,amplitude);
	A += origin;
	B += origin;
	vq2::xfig::line(convert,file,gc,
			typename GRAPH::vertex_value_type(A),
			typename GRAPH::vertex_value_type(B));

	g.for_each_vertex(speeds);
      }
      
    }
  }
}
#endif
