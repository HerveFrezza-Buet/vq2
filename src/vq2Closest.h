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

#ifndef vq2CLOSEST_H
#define vq2CLOSEST_H

#include <utility>
#include <vq2Concept.h>
#include <vq2Functors.h>

namespace vq2 {

  namespace algo {
    template<typename GRAPH,
	     typename SIMILARITY>
    typename GRAPH::ref_vertex_type closest(GRAPH& g,
					    SIMILARITY& sim,
					    const typename SIMILARITY::sample_type& v,
					    double& closest_similarity) {

      functor::ClosestFunctor<GRAPH,SIMILARITY> functor(v,sim);
      g.for_each_vertex_ref(functor);
      closest_similarity = functor.min;
      return functor.closest;
    }

    /**
     * @returns a pair (closest, second-closest).
     */
    template<typename GRAPH,
	     typename SIMILARITY>
    std::pair<typename GRAPH::ref_vertex_type,
	      typename GRAPH::ref_vertex_type> twoClosest(GRAPH& g,
							  SIMILARITY& sim,
							  const typename SIMILARITY::sample_type& v,
							  std::pair<double,double>& closest_similarity) {

      functor::TwoClosestFunctor<GRAPH,SIMILARITY> functor(v,sim);
      g.for_each_vertex_ref(functor);
      closest_similarity.first  = functor.min1;
      closest_similarity.second = functor.min2;
      return std::pair<typename GRAPH::ref_vertex_type,
		       typename GRAPH::ref_vertex_type>(functor.closest1,functor.closest2);
    }

  }

  namespace functor {
    template<typename GRAPH,
	     typename SIMILARITY>
    class GraphSimilarityFunctor {
    public:
      GRAPH&      g;
      SIMILARITY& sim;
      int nb;
      double d;
      
      GraphSimilarityFunctor(GRAPH& graph,
			     SIMILARITY& similarity)
	: g(graph), sim(similarity), nb(0), d(0) {}

      bool operator()(typename GRAPH::vertex_type& n) {
	double dd;
	vq2::algo::closest(g,sim,n.value,dd);
	++nb;
	d += dd;
	return false;
      }
    };
  }

  namespace algo {
    /**
     * This computes some similarity of two graphs, from their vertex positions.
     */
    template<typename GRAPH,
	     typename SIMILARITY>
    double similarity(GRAPH& g1, GRAPH& g2,
		      SIMILARITY& sim) {
      functor::GraphSimilarityFunctor<GRAPH,SIMILARITY> functor1(g2,sim);
      functor::GraphSimilarityFunctor<GRAPH,SIMILARITY> functor2(g1,sim);
      g1.for_each_vertex(functor1);
      g2.for_each_vertex(functor2);
      return (functor1.d+functor2.d)/(functor1.nb+functor2.nb);
    }
    

    template<typename GRAPH,
	     typename SIMILARITY,
	     typename Iterator>
    double distortion(GRAPH& g,
		      SIMILARITY& sim,
		      Iterator begin, Iterator end) {
      double d;
      double sum=0;
      
      for(Iterator sample_set = begin; sample_set != end; ++sample_set ) {
	vq2::algo::closest(g,sim,*sample_set,d);
	sum += d;
      }
      return sum/(end-begin);
    }
  }
}

#endif
