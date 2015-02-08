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

#ifndef vq2UNIT_H
#define vq2UNIT_H

namespace vq2 {
  namespace unit {
    
      /**
       * This is a minimal unit class. This fits vq2::concept::Unit.
       */
      template<typename PROTOTYPE>
      class Base {
      public:
	typedef PROTOTYPE prototype_type;

	prototype_type w;

	Base(void) : w() {}
	Base(const Base& copy) : w(copy.w) {}
	Base(const prototype_type& proto) : w(proto) {}

	Base& operator=(const Base& copy) {
	  if(this != &copy) 
	    w = copy.w;
	  return *this;
	}

	Base& operator=(const prototype_type& proto) {
	  w = proto;
	  return *this;
	}

	/**
	 * This does the same job as the copy constructor. Implement
	 * this in any Unit class.
	 */
	class copy_constructor {
	public:
	  void operator()(Base<PROTOTYPE>& that,
			  const Base<PROTOTYPE>& copy) {
	    that = copy;
	  }
	};

	prototype_type& prototype(void) {return w;}
	const prototype_type& prototype(void) const {return w;}

      };

    /**
     * This implements a similarity between a unit and a sample using
     * the unit prototype.
     */
    template<typename UNIT,typename SIMILARITY>
    class Similarity {
    private:
      SIMILARITY& sim;

    public:
      typedef UNIT value_type;
      typedef typename SIMILARITY::sample_type  sample_type;

      /**
       * @param similarity compares prototypes to samples.
       */
      Similarity(SIMILARITY& similarity) : sim(similarity) {}
      /**
       * @param arg1 This is what the vertices store (vertex.value)
       * @param arg2 This is some external data.
       * @return a <b>positive</b> value that is close to zero if arg1 and arg2 are close.
       */
      double operator()(const value_type& arg1,
			const sample_type& arg2) {
	return sim(arg1.prototype(),arg2);
      }
    };
    
    

    /**
     * This adapts the learning procedure
     */
    template<typename UNIT,typename LEARN>
    class Learn {
    private:

      LEARN& rule;

    public:
      typedef UNIT weight_type;
      typedef typename LEARN::sample_type sample_type;

      Learn(LEARN& learn) : rule(learn) {}

      void operator()(double coef,
		      weight_type& prototype,
		      const sample_type& target) {
	rule(coef,prototype.prototype(),target);
      }
    };

    /**
     * This adapts a fig converter to units.
     */
    template<typename UNIT,typename XFIG_CONVERTER>
    class XfigConverter {
    private:
      XFIG_CONVERTER& conv;

    public:
      typedef UNIT position_type;

      XfigConverter(XFIG_CONVERTER& converter) : conv(converter) {}
      
      /**
       * This converts pos into a point (x_cm,y_cm) in centimeters.
       */
      void locate(const UNIT& pos,
		  double& x_cm, double& y_cm) {
	conv.locate(pos.prototype(),x_cm,y_cm);
      }
    };
  }
}

#endif
