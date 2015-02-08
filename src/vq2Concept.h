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

#ifndef vq2CONCEPT_H
#define vq2CONCEPT_H

namespace vq2 { 

  template<typename CONCEPT>
  class fits {};
  template<typename CONCEPT1,typename CONCEPT2>
  class have_to_match {};
  typedef int Any;

  namespace concept {
    class Similarity {
    public:
      typedef Any value_type;
      typedef Any sample_type;
      /**
       * @param arg1 This is what the vertices store (vertex.value)
       * @param arg2 This is some external data.
       * @return a <b>positive</b> value that is close to zero if arg1 and arg2 are close.
       */
      double operator()(const value_type& arg1,
			const sample_type& arg2);
    };

    class Split {
    public:
      typedef Any value_type;
      /**
       * @param source This is the value from which we split
       * @param destination This is set as a perturbation of source.
       */
      void operator()(value_type& destination,
		      const value_type& source);
    };

    template<typename SAMPLE,typename INDEX>
    class Set {
    public:
      const SAMPLE& operator[](const INDEX& index);
    };

    /**
     * This is the winner take most class.
     */
    class WTM {
    public:
      /**
       * @param d The topological distance (i.e. the number of edges to the winning prototype).
       * @result A value in [0,1].
       */
      double operator()(double d);
    };

    /**
     * This is the learning procedure
     */
    class Learn {
    public:
      typedef Any weight_type;
      typedef Any sample_type;
      void operator()(double coef,
		      weight_type& prototype,
		      const sample_type& target);
    };

    /**
     * This provide elementary vector operations
     */
    class VectorOp {
    public:
      void raz(Any& v);
      void add(Any& v, const Any& w);
      void div(Any& v, double coef);
      void mul(Any& v, double coef);
    };

    /**
     * This is the unit for all vq algorithms. A unit handles the
     * prototype and algorithm-dependent supplementary stuff.
     */
      class Unit {
      public:
	typedef Any prototype_type;

	/**
	 * This returns the prototype of the unit.
	 */
	prototype_type& prototype(void) const;
      };

    /**
     * This is a param class for GNGT
     */
    class GNGTParams {
    public:
      int ageMax(void);
      /**
       * @return the learning rate for the winner.
       */
      double learningRate(void);
      /**
       * @return the learning rate ration r : winner neighbours have a r*learningRate() learning rate.
       */
      double learningRatio(void);
      double lambda(void);
    };

    
    /**
     * This provides examples to integrated gngt-epoch methods.
     */
    class GNGTSampling {
    public:
      typedef Any iterator;
      iterator begin(void);
      iterator end(void);
    };

    /**
     * This handles GNGT statistics for the control of the graph
     * evolution. A user-defined instance of GNGTEvolution has to be
     * provided to the GNG-T functions.
     */
    class GNGTEvolution {
    public:
      
      /**
       * Clears the statistics.
       */
      void clear(void);

      /**
       * Declares a value to be considered n the statistics.
       */
      void operator+=(double value);
      
      /**
       * @short evolution méthod. 
       * @returns -1 if there are too many prototypes, 1 if prototypes should be added, 0 otherwise.
       */
      int operator()();
    };
    /**
     * This is a param class for GNGT evolutions
     */
    class EvolutionParams {
    public:

      /**
       * @returns the target.
       */
      double target(void);

      /**
       * @returns Return the parameter that drives the input sampling process. <b>This is not the size of your dataset</b>.
       */
      int nbSamples(void);

      /**
       * @returns the low-pass filter coefficient for minmax bounds smoothing (ex 0.4).
       */
      double lowPassCoef(void);

      /**
       * @returns the quantile of the sortes confident interval. (ex 0.75).
       */
      double delta(void);
      
      /**
       * @returns the m such as target*nbSamples is desired in [min+m,max-m], where min/max are the low pass filtered bounds of the shortest-confident-inverval. (ex 0.2)
       */
      double margin(void);
      
      
    };

  }
}

#endif

