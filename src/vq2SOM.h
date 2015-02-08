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

#ifndef vq2SOM_H
#define vq2SOM_H

#include <vq2Concept.h>
#include <vq2Closest.h>
#include <vq2Topology.h>
#include <vq2Functors.h>
#include <vq2Unit.h>
#include <vector>

namespace vq2 {
  namespace algo {
    namespace som {

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


      template<typename GRAPH,
	       typename SIMILARITY,
	       typename WTM,
	       typename LEARN>
      void step(GRAPH& g,
		SIMILARITY& sim,
		WTM& wtm,
		LEARN& learn,
		double coef,
		const typename LEARN::sample_type& v) {
	functor::Learn<GRAPH,WTM,LEARN> learn_functor(wtm,learn,coef,v);
	double dummy;
	distance(g,&(*(closest(g,sim,v,dummy))));
	g.for_each_vertex(learn_functor);
      }
    }
  }
}

#endif
