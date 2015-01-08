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

#ifndef vq2KMEANS_H
#define vq2KMEANS_H

#include <vq2Concept.h>
#include <vq2Functors.h>
#include <vq2Closest.h>
#include <vq2Unit.h>
#include <vector>
#include <algorithm>

namespace vq2 {
  namespace algo {
    namespace kmeans {


      template<typename PROTOTYPE>
      class Unit : public unit::Base<PROTOTYPE>{
      public:
	typedef typename unit::Base<PROTOTYPE>::prototype_type prototype_type;
	Unit(void) : unit::Base<PROTOTYPE>() {}
	Unit(const Unit<PROTOTYPE>& copy) : unit::Base<PROTOTYPE>(copy) {}
	Unit(const prototype_type& proto) : unit::Base<PROTOTYPE>(proto) {}

	Unit<PROTOTYPE>& operator=(const Unit& copy) {
	  this->unit::Base<PROTOTYPE>::operator=(copy);
	  return *this;
	}

	Unit<PROTOTYPE>& operator=(const prototype_type& proto) {
	  this->unit::Base<PROTOTYPE>::operator=(proto);
	  return *this;
	}

	class copy_constructor {
	public:
	  void operator()(Unit<PROTOTYPE>& that,
			  const Unit<PROTOTYPE>& copy) {
	    that = copy;
	  }
	};
      };

      template<typename PROTOTYPE>
      Unit<PROTOTYPE> unit_of(const PROTOTYPE& proto) {return Unit<PROTOTYPE>(proto);}

      /**
       * This is the Linde-Buzo-Gray algorithm. The given graph will
       * be cleared by the call.
       */
      template<typename GRAPH,
	       typename GET_PROTO,
	       typename SIMILARITY,
	       typename VECTOR_OP,
	       typename SPLIT,
	       typename SET_ITER>
      void process(GRAPH& g,
		   unsigned int k,
		   const GET_PROTO& get_proto,
		   SIMILARITY& sim,
		   VECTOR_OP& op,
		   SPLIT& split,
		   const SET_ITER& begin,const SET_ITER& end) {
	unsigned int size = (unsigned int)(end-begin);
	std::vector<typename GRAPH::vertex_type*> nodes;
	std::vector<int> labels;
	std::vector<int> nb;
	SET_ITER i;
	double dummy;
	bool achieved;
	typename std::vector<int>::iterator liter,lend;
	typename std::vector<typename GRAPH::vertex_type*>::iterator piter,pend;
	typename GRAPH::ref_vertex_type clst;
	
	g.clear();

	if(k == 0 || size == 0 || size <= k)
	  return;

	labels.resize(size);

	// We create a prototype, that is the barycenter of the data. 
	typename GRAPH::vertex_value_type::prototype_type first_proto(get_proto());
	op.raz(first_proto);
	typename GRAPH::ref_vertex_type first_node_ref(g += unit_of(first_proto));
	typename GRAPH::vertex_value_type::prototype_type& first_proto_ref = (*first_node_ref).value.prototype();
	for(i=begin; i!= end; ++i)
	  op.add(first_proto_ref,*i);
	op.div(first_proto_ref,(double)size);
	
	// We add the node in nodes.
	{
	  typename GRAPH::vertex_type& n = *first_node_ref;
	  n.stuff.tag = (int)(nodes.size());
	  nodes.push_back(&n);
	}

	// Check if we have enough nodes.
	int remaining = k - nodes.size();
	while(remaining > 0) {
	  // we need to split existing nodes.
	  int nb_splits = std::min(nodes.size(),(typename std::vector<typename GRAPH::vertex_type*>::size_type)remaining);

	  // Let us shuffle nodes, since we will systematically split
	  // from the first index.
	  // std::random_shuffle(nodes.begin(), nodes.end()); // quite useless

	  // Let us now split the nb_split first nodes.
	  for(int splt=0; splt < nb_splits; ++splt) {
	    typename GRAPH::vertex_value_type::prototype_type proto(get_proto());
	    split(proto,nodes[splt]->value.prototype());
	    typename GRAPH::ref_vertex_type node_ref(g += unit_of(proto));

	    // We add the node in nodes.
	    typename GRAPH::vertex_type& n = *node_ref;
	    n.stuff.tag = (int)(nodes.size());
	    nodes.push_back(&n);
	  }

	  // Now, we can start the k-mean labelling and updating
	  // procedure with the current nodes.

	  nb.resize(nodes.size());
	  labels[0] = -1; // First labelling will at least change this value...
	  achieved = false;
	  while(!achieved) {
	    achieved = true;
	    std::fill(nb.begin(),nb.end(),0);
	  
	    // Compute labels for each sample.
	    for(i = begin, liter = labels.begin(), lend = labels.end();
		liter != lend;
		++liter, ++i) {
	      clst = closest(g,sim,
			     *i,dummy);
	      int l = (*clst).stuff.tag;
	      if(l != *liter) {
		*liter = l;
		achieved = false;
	      }
	      nb[l]++;
	    }
	  
	    // Le us update barycenters.
	    if(!achieved) {
	      for(piter = nodes.begin(), pend = nodes.end();
		  piter != pend;
		  ++piter)
		op.raz((*piter)->value.prototype());
	    
	      for(liter = labels.begin(), lend = labels.end(), i=begin;
		  liter != lend;
		  ++liter,++i)
		op.add((nodes[*liter])->value.prototype(),*i);
	    
	      std::vector<int>::iterator n;
	      for(piter = nodes.begin(), pend = nodes.end(), n = nb.begin();
		  piter != pend;
		  ++piter,++n)
		if(*n != 0)
		  op.div((*piter)->value.prototype(), *n);
		else
		  std::cout << "Warning : vq2::algo::kmeans::LBG::process : non winning prototype found." << std::endl;
	    }
	  }

	  // Check if we have enough nodes.
	  remaining = k - nodes.size();
	}
      }
    }
  }
		       
}

#endif
