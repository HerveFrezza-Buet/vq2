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

#ifndef vq2FUNCTORS_H
#define vq2FUNCTORS_H

#include <queue>
#include <vector>
#include <limits>

#include <cstdlib>

namespace vq2 {

  namespace functor {


    template<typename CONTENT,
	     typename VALUE_OF>
    class Sort {

      const VALUE_OF& value_of;

    public:

      Sort(const VALUE_OF& v) : value_of(v) {}
      bool operator()(const CONTENT& a, const CONTENT& b) const {
	return value_of(a) < value_of(b);
      }
    };

    template<typename CONTENT,
	     typename VALUE_OF>
    Sort<CONTENT,VALUE_OF> sort(const VALUE_OF& v, const CONTENT& dummy) {
      return Sort<CONTENT,VALUE_OF>(v);
    }

    template<typename GRAPH,
	     typename SIMILARITY>
    class ClosestFunctor {
    public:
      const typename SIMILARITY::sample_type& v;
      SIMILARITY& sim;
      double min;
      typename GRAPH::ref_vertex_type closest;

      ClosestFunctor(const typename SIMILARITY::sample_type& value,
		      SIMILARITY& similarity) :
	v(value), sim(similarity), min(std::numeric_limits<double>::max()), closest() {}

      bool operator()(typename GRAPH::ref_vertex_type& n) {
	double tmp = sim((*n).value,v);
	if(tmp < min) {
	  closest = n;
	  min = tmp;
	}
	return false;
      }
    };

    template<typename GRAPH,
	     typename SIMILARITY>
    class TwoClosestFunctor {
    public:
      const typename SIMILARITY::sample_type& v;
      SIMILARITY& sim;
      double min1,min2;
      typename GRAPH::ref_vertex_type closest1;
      typename GRAPH::ref_vertex_type closest2;

      TwoClosestFunctor(const typename SIMILARITY::sample_type& value,
			 SIMILARITY& similarity) :
	v(value), sim(similarity), 
	min1(std::numeric_limits<double>::max()), min2(std::numeric_limits<double>::max()), 
	closest1(), closest2() {}
      
      bool operator()(typename GRAPH::ref_vertex_type& n) {
	double tmp = sim((*n).value,v);
	if(tmp < min1) {
	  closest2 = closest1;
	  min2     = min1;
	  closest1 = n;
	  min1     = tmp;
	}
	else if(tmp < min2) {
	  closest2 = n;
	  min2     = tmp;
	}
	return false;
      }
    };

    template<typename GRAPH>
    class ClearDistance  {
    public:
      bool operator()(typename GRAPH::vertex_type& n) {
	n.stuff.distance = -1;
	return false;
      }
    };

    template<typename GRAPH>
    class TagDistanceAtEdge  {
    public:
      typename GRAPH::vertex_type* n;
      std::queue<typename GRAPH::vertex_type*>& fifo;
      int dist;

      TagDistanceAtEdge(std::queue<typename GRAPH::vertex_type*>& q)
	: n(0), fifo(q), dist(-1) {}
      bool operator()(typename GRAPH::edge_type& e) {
	typename GRAPH::vertex_type* nn;
	if(&(*(e.n1)) == n) 
	  nn = &(*e.n2);
	else
	  nn = &(*e.n1);
	if(nn->stuff.distance < 0) {// nn hasn't be tagged yet 
	  nn->stuff.distance = dist;
	  fifo.push(nn);
	}
	return false;
      }
    };
    
    template<typename GRAPH,
	     typename WTM,
	     typename LEARN>
    class Learn {
    public:
      
      WTM& wtm;
      LEARN& learn;
      double coef;
      const typename LEARN::sample_type& xi;

      Learn(WTM& neighbour, LEARN& rule, 
	    double alpha, 
	    const typename LEARN::sample_type& sample)
	: wtm(neighbour), learn(rule), 
	  coef(alpha), xi(sample) {}

      bool operator()(typename GRAPH::vertex_type& n) {
	double alpha;
	if((n.stuff.tag >= 0)
	   && ((alpha=wtm(n.stuff.distance))>0))
	  learn(alpha*coef,n.value,xi);
	return false;
      }
    };

    template<typename GRAPH,
	     typename SET_ITER>
    class LBGInit {
    public:
      
      std::vector<typename GRAPH::vertex_type*> nodes;
      const SET_ITER& begin;
      unsigned int size;

      LBGInit(const SET_ITER& first,
	      unsigned int sample_set_size) 
	: nodes(),
	  begin(first),
	  size(sample_set_size) {
	nodes.resize(0);
      }
      bool operator()(typename GRAPH::vertex_type& n) {
	int k = (int)(size*(rand()/(1.0+RAND_MAX)));
	n.value.prototype() = *(begin+k);
	n.stuff.tag = (int)(nodes.size());
	nodes.push_back(&n);
	return false;
      }
      
    };

    template<typename GRAPH>
    class UnefficientEdge  {
    public:

      bool operator()(typename GRAPH::edge_type& e) {
	e.stuff.efficient = false;
	return false;
      }
    };
  }
}

#endif
