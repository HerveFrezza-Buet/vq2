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

#ifndef vq2PROBA_H
#define vq2PROBA_H

#include <cstdlib>
#include <ctime>

#include <vq2ByDefault.h>

namespace vq2 {
  namespace proba {
    /**
     * This computes the d-shortest confidence intervall.
     * @param begin,end This sequence will be sorted !
     * @param value_of A functor that extracts a double from *iter
     * @param d belongs to [.5,1], it is the confidence level.
     * @param min,max Are set to the interval bounds.
     */
    template<typename ITER, typename VALUE_OF>
    void shortest_confidence_interval(const ITER& begin,
				      const ITER& end,
				      const VALUE_OF& value_of,
				      double d,
				      double& min, double& max) {
      unsigned int size = end - begin;

      if(size == 0)
	return;
      else if(size <= 2) {
	min = value_of(*begin);
	max = min;
      }
      else {
	
	std::sort(begin,end,vq2::functor::sort(value_of,*begin));
      
	unsigned int window_size = (unsigned int)(size*d+.5);
	unsigned int upper_bound = size-window_size+1;
	unsigned int kmin = 0;
	double delta_min,delta;
	delta_min = value_of(*(begin+(size-1)))-value_of(*begin)+1;
	for(unsigned int k=0; k<upper_bound; ++k)
	  if((delta = value_of(*(begin+(k + (window_size-1)))) - value_of(*(begin+k))) < delta_min) {
	    delta_min = delta;
	    kmin = k;
	  }
      
	min = value_of(*(begin+kmin));
	max = value_of(*(begin+(kmin + (window_size-1))));
      }
    }

    class random {
    public:
      /**
       * This initializes the random generator... randomly !
       */
      static void init(void) {
	srand(time((time_t*)0));
      }

      /**
       * This initializes the random seed.
       */
      static void init(unsigned int seed) {
	srand(seed);
      }

      /**
       * @return A value in [min, max[
       */
      static double uniform(double min, double max) {
	return min + (max-min)*(rand()/(1.0+RAND_MAX));
      }

      /**
       * @return A value in [0, 1[
       */
      static double uniform(void) {
	return uniform(0,1);
      }

      /**
       * @return true with a probability proba.
       */
      static bool toss(double proba) {
	return uniform() < proba;
      }
    };

    template<typename ANY>
    class Density {
    public:
      typedef ANY event_type;
      virtual double operator()(const event_type& evt)=0;
      virtual ~Density(void) {}
    };

    template<typename ANY>
    class Uniform : public Density<ANY> {
    public:
      typedef ANY event_type;
      double density;
      Uniform(double proba_density) : density(proba_density) {}
      virtual double operator()(const event_type& evt) {
	return density;
      }
    };

    template<typename ANY>
    class Min : public Density<ANY> {
    public:
      typedef ANY event_type;
      Density<ANY>& a;
      Density<ANY>& b;
      Min(Density<ANY>& aa,
	  Density<ANY>& bb) : a(aa), b(bb) {}
      virtual double operator()(const event_type& evt) {
	double da = a(evt);
	double db = b(evt);
	if(da < db)
	  return da;
	else
	  return db;
      }
    };

    template<typename ANY>
    class Max : public Density<ANY> {
    public:
      typedef ANY event_type;
      Density<ANY>& a;
      Density<ANY>& b;
      Max(Density<ANY>& aa,
	  Density<ANY>& bb) : a(aa), b(bb) {}
      virtual double operator()(const event_type& evt) {
	double da = a(evt);
	double db = b(evt);
	if(da > db)
	  return da;
	else
	  return db;
      }
    };

    template<typename ANY>
    class Not : public Density<ANY> {
    public:
      typedef ANY event_type;
      Density<ANY>& a;
      Not(Density<ANY>& aa) : a(aa) {}
      virtual double operator()(const event_type& evt) {
	return 1-a(evt);
      }
    };

    template<typename ANY,
	     typename VECTOR_OP>
    class Scale : public Density<ANY> {
    public:
      typedef ANY event_type;
      Density<ANY>& a;
      VECTOR_OP& op;
      double factor;

      Scale(Density<ANY>& aa, 
	    VECTOR_OP& vector_op,
	    double scale_factor) 
	: a(aa), op(vector_op), factor(scale_factor) {}
      virtual double operator()(const event_type& evt) {
	event_type x(evt);
	op.div(x,factor);
	return a(x);
      }
    };

    template<typename ANY,
	     typename VECTOR_OP>
    class Translate : public Density<ANY> {
    public:
      typedef ANY event_type;
      Density<ANY>& a;
      VECTOR_OP& op;
      event_type translation;

      Translate(Density<ANY>& aa, 
		VECTOR_OP& vector_op,
		const event_type& translation_vector) 
	: a(aa), op(vector_op), translation(translation_vector) {}
      virtual double operator()(const event_type& evt) {
	event_type x(translation);
	op.mul(x,-1);
	op.add(x,evt);
	return a(x);
      }
    };

    /**
     * The result is allocated by new.
     */
    template<typename ANY>
    Uniform<ANY>& uniform(double proba) {
      return *(new Uniform<ANY>(proba));
    }

    template<typename DENSITY>
    Min<typename DENSITY::event_type>& min(DENSITY& aa,DENSITY& bb) {
      return *(new Min<typename DENSITY::event_type>(aa,bb));
    }

    template<typename DENSITY>
    Max<typename DENSITY::event_type>& max(DENSITY& aa,DENSITY& bb) {
      return *(new Max<typename DENSITY::event_type>(aa,bb));
    }

    template<typename DENSITY>
    Not<typename DENSITY::event_type>& invert(DENSITY& aa) {
      return *(new Not<typename DENSITY::event_type>(aa));
    }

    template<typename DENSITY, typename VECTOR_OP>
    Scale<typename DENSITY::event_type,VECTOR_OP>& scale(DENSITY& aa,VECTOR_OP& op,double factor) {
      return *(new Scale<typename DENSITY::event_type,VECTOR_OP>(aa,op,factor));
    }

    template<typename DENSITY, typename VECTOR_OP>
    Translate<typename DENSITY::event_type,VECTOR_OP>& translate(DENSITY& aa,VECTOR_OP& op,const typename DENSITY::event_type& t) {
      return *(new Translate<typename DENSITY::event_type,VECTOR_OP>(aa,op,t));
    }
    
  }
}

#endif
