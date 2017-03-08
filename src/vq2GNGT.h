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

#ifndef vq2GNGT_H
#define vq2GNGT_H

#include <vq2Concept.h>
#include <vq2Closest.h>
#include <vq2Functors.h>
#include <vq2Graph.h>
#include <vq2Proba.h>
#include <vq2Speed.h>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

namespace vq2 {
  namespace by_default {
    namespace gngt {
      template<typename EVOLUTION_PARAM>
      class Evolution {
      public:
	
	std::vector<double> disto_distrib;
	double min,max;
	bool first_run;
	EVOLUTION_PARAM& param;
	
	Evolution(EVOLUTION_PARAM& evolution_param)
	: first_run(true),
	  param(evolution_param){}

	void clear(void) {
	  disto_distrib.clear();
	}
	void operator+=(double value) {disto_distrib.push_back(value);}
	int operator()() {
	  int res = 0;
	  double a,b;
	  vq2::proba::shortest_confidence_interval(disto_distrib.begin(),
						   disto_distrib.end(),
						   [](double x) -> double {return x;},
						   param.delta(),a,b);
	  if(first_run) {
	    min = a;
	    max = b;
	    first_run = false;
	  }
	  else {
	    min += param.lowPassCoef()*(a-min);
	    max += param.lowPassCoef()*(b-max);
	  }
	  
	  double width;
	  
	  width = (max-min);
	  double _min = min + param.margin()*width;
	  double _max = min + (1-param.margin())*width;
	  
	  double nt = param.nbSamples()*param.target();
	  
	  if(nt < _min)
	    res = 1;
	  else if(nt > _max) 
	    res = -1;
	  return res;
	}
      };
      
      template<typename EVOLUTION_PARAM>
      Evolution<EVOLUTION_PARAM> evolution(EVOLUTION_PARAM& p) {
	return  Evolution<EVOLUTION_PARAM>(p);
      }
    }
  }

  namespace algo {
    namespace gngt {
      

      template<typename PROTOTYPE>
      class Unit : public unit::Base<PROTOTYPE>{
      public:
	
	/**
	 * The distorion accumulator.
	 */
	double e;
	
	/**
	 * The number of times this prototype was the best during last
	 * epoch.
	 */
	int n;

	/**
	 * false means that, at least once during last epoch, the neuron has won or has been the second closest. 
	 */
	bool useless;
	
	typedef typename unit::Base<PROTOTYPE>::prototype_type prototype_type;

	Unit(void) 
	  : unit::Base<PROTOTYPE>(), 
	  e(0), n(0), useless(true) {}
	Unit(const Unit<PROTOTYPE>& copy) 
	  : unit::Base<PROTOTYPE>(copy), 
	  e(copy.e), n(copy.n), useless(copy.useless) {}
	Unit(const prototype_type& proto) : 
	  unit::Base<PROTOTYPE>(proto), 
	    e(0), n(0), useless(true) {}

	Unit<PROTOTYPE>& operator=(const Unit& copy) {
	  if(this != &copy) {
	    this->unit::Base<PROTOTYPE>::operator=(copy);
	    e      = copy.e;
	    n      = copy.n;
	    useless = copy.useless;
	  }
	  return *this;
	}

	Unit<PROTOTYPE>& operator=(const prototype_type& proto) {
	  
	  this->unit::Base<PROTOTYPE>::operator=(proto);
	  e      = 0;
	  n      = 0;
	  useless = true;
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

      namespace internal {

	template<typename VERTEX>
	class ClearNeuronStats {
	private:
	public:
	  ClearNeuronStats(void) {}
	  bool operator()(VERTEX& v) {
	    v.value.n      = 0;
	    v.value.e      = 0;
	    v.value.useless = true;
	    return false; 
	  }
	};
	
	template<typename VERTEX,typename REF_VERTEX,
		 typename EDGE,
		 typename GNGT_PARAMS>
	class UpdateAge {
	public:
	  bool found;
	  VERTEX* n2;
	  GNGT_PARAMS& params;
	  
	  UpdateAge(REF_VERTEX second,
		    GNGT_PARAMS& p) 
	    : found(false), 
	      n2(&(*second)), 
	      params(p) {}
	  
	  bool operator()(EDGE& e) {
	    if((&(*(e.n1)) == n2) || (&(*(e.n2)) == n2)) {
	      e.value = 0;
	      found = true;
	    }
	    else {
	      e.value++;
	      if(e.value > params.ageMax())
		return true;
	    }
	    return false;
	  }
	};

	template<typename VERTEX,typename REF_VERTEX,
		 typename EDGE,
		 typename LEARN,
		 typename GNGT_PARAMS>
	class UpdateNeighbour {
	public:
	  VERTEX* n1;
	  LEARN& learn;
	  const typename LEARN::sample_type& xi;
	  double lr;
	  
	  UpdateNeighbour(REF_VERTEX& first,
			  const typename LEARN::sample_type& x,
			  GNGT_PARAMS& p,
			  LEARN& l) 
	    : n1(&(*first)), 
	      learn(l),
	      xi(x),
	      lr(p.learningRate()*p.learningRatio()){}
	  
	  bool operator()(EDGE& e) {
	    VERTEX* n2;
	    if(&(*(e.n1)) == n1)
	      n2 = &(*(e.n2));
	    else
	      n2 = &(*(e.n1));
	    
	    learn(lr,n2->value,xi);
	    return false;
	  }
	};


	template<typename REF_VERTEX,
		 typename UNIT,
		 typename GNGT_EVOLUTION>
	class MakeNeuronStat {
	public:

	  GNGT_EVOLUTION& stats;
	  REF_VERTEX min,max;
	  double mine,maxe;
	  unsigned int& nb;

	  MakeNeuronStat(GNGT_EVOLUTION& e,
			 unsigned int& nb_vertices) 
	    : stats(e), 
	      min(), max(), 
	      mine(-1), maxe(0),
	      nb(nb_vertices) {
	    nb = 0;
	    stats.clear();
	  }
	  bool operator()(REF_VERTEX& ref) {
	    UNIT& proto = (*ref).value;
	    if(proto.useless)
	      return true;

	    if(proto.n > 0) {// if the unit is not a dead unit, placed in an empty region
	      nb++;
	      stats += proto.e;
	      
	      
	      if(proto.e < mine || mine < 0) {
		mine = proto.e;
		min = ref;
	      }
	      
	      // Not elseif... if a single neuron is there
	      // >= in case of e=0...
	      if(proto.e >= maxe) {
		maxe = proto.e;
		max = ref;
	      }
	    }
	    return false;
	  }
	};

	template<typename VERTEX,typename REF_VERTEX,
		 typename EDGE,
		 typename GNGT_PARAMS>
	class FindHighestNeighbour {
	public:

	  REF_VERTEX& n1;
	  REF_VERTEX  n2;
	  double max;
	  

	  FindHighestNeighbour(REF_VERTEX& max) 
	    : n1(max), n2(), max(0) {}

	  bool operator()(EDGE& e) {
	    REF_VERTEX n;
	    if(&(*(e.n1)) == &(*n1))
	      n = e.n2;
	    else
	      n = e.n1;
	    
	    if((*n).value.e >= max) {
	      max = (*n).value.e;
	      n2 = n;
	    }
	    return false;
	  }
	};
      }
      

	
      template<typename GRAPH,
	       typename GNGT_EVOLUTION>
      void open_epoch(GRAPH& g,
		      GNGT_EVOLUTION& stats) {
	internal::ClearNeuronStats<typename GRAPH::vertex_type> clear_vertex_stats;
	g.for_each_vertex(clear_vertex_stats);
      }
      
      /**
       * Once StartEpoch is called, use this method to submit examples.
       * @param dynamic_topology true means that topology (number of vertices and edges) can change.
       */
      template<typename GRAPH,typename GNGT_PARAMS,
	       typename SIMILARITY, typename LEARN>
      static void submit(GNGT_PARAMS& params,
			 GRAPH& g,
			 SIMILARITY& distance,
			 LEARN& learn,
			 const typename LEARN::sample_type& xi,
			 bool dynamic_topology) {

	// Create two first vertextes from xi.
	if(g.nbVertices() < 2) {
	  if(dynamic_topology) {
	    typename GRAPH::ref_vertex_type ref(g += typename GRAPH::vertex_value_type(xi));
	    (*ref).value.n = 1;
	  }
	  return;
	}

	// Find the two closest vertices.
	std::pair<double,double> closest_dists;
	std::pair<typename GRAPH::ref_vertex_type,
		  typename GRAPH::ref_vertex_type> two_closest = vq2::algo::twoClosest(g,distance,
										       xi,closest_dists);

	// Set them as useless.
	(*(two_closest.first )).value.useless = false;
	(*(two_closest.second)).value.useless = false;

	if(dynamic_topology) {
	  // Update edges.
	  internal::UpdateAge<typename GRAPH::vertex_type,
	    typename GRAPH::ref_vertex_type,
	    typename GRAPH::edge_type,
	    GNGT_PARAMS> update_age(two_closest.second,params);
	  (*(two_closest.first)).for_each_edge(update_age);
	  if(!(update_age.found))
	    g.connect(0,two_closest.first,two_closest.second);
	}

	// Update closest.
	typename GRAPH::vertex_value_type& n1 = (*(two_closest.first)).value;
	n1.e += closest_dists.first;
	++n1.n;
	learn(params.learningRate(),n1,xi);

	// Update its neighbours.
	internal::UpdateNeighbour<typename GRAPH::vertex_type,
				  typename GRAPH::ref_vertex_type,
				  typename GRAPH::edge_type,
				  LEARN,
				  GNGT_PARAMS> update_neighbour(two_closest.first,
								xi,params,learn);
	(*(two_closest.first)).for_each_edge(update_neighbour);
      }

	
      /**
       * This close an epoch.
       * @param dynamic_topology true means that topology (number of vertices and edges) can change.
       */
      template<typename GNGT_PARAMS,
	       typename GRAPH,
	       typename LEARN,
	       typename GNGT_EVOLUTION>
      void close_epoch(GNGT_PARAMS& params,
		       GRAPH& g,
		       LEARN& learn,
		       GNGT_EVOLUTION& stats,
		       bool dynamic_topology) {
	unsigned int nb_vertices;
	internal::MakeNeuronStat<typename GRAPH::ref_vertex_type,
				 typename GRAPH::vertex_value_type,
				 GNGT_EVOLUTION> make_neuron_stat(stats,nb_vertices);
	  
	  
	if(!dynamic_topology)
	  return;

	g.for_each_vertex_ref(make_neuron_stat);
	if(nb_vertices == 0)
	  return;

	int increment = stats();
	if(increment == 0)
	  return;
	  
	// T is too low, we have to remove some neuron.
	if(increment < 0) {
	  make_neuron_stat.min.take();
	  make_neuron_stat.min.free();
	  return;
	}
	  
	// T is too high, we have to add some neuron.
	  
	// Let us find the neighbour with the highest e
	internal::FindHighestNeighbour<typename GRAPH::vertex_type,
				       typename GRAPH::ref_vertex_type,
				       typename GRAPH::edge_type,
				       GNGT_PARAMS>  find_highest_neighbour(make_neuron_stat.max);
	(*(make_neuron_stat.max)).for_each_edge(find_highest_neighbour);

	if(!(find_highest_neighbour.n2)) {
	  // max is an isolated node. We clone it.
	  typename GRAPH::ref_vertex_type clone(g += (*(make_neuron_stat.max)).value);
	  g.connect(0,make_neuron_stat.max,clone);
	}
	else {
	  typename GRAPH::vertex_value_type& a = (*(find_highest_neighbour.n1)).value;
	  typename GRAPH::vertex_value_type& b = (*(find_highest_neighbour.n2)).value;
	  typename GRAPH::vertex_value_type  c(a.prototype());
	  learn(params.lambda(),c,b.prototype());
	  typename GRAPH::ref_vertex_type middle(g += c);
	  g.connect(0,find_highest_neighbour.n1,middle);
	  g.connect(0,find_highest_neighbour.n2,middle);
	}
      }

      /**
       * This performs several epochs.
       * @param n the number of epochs
       * @param dynamic_topology true means that topology (number of vertices and edges) can change.
       * @param samples a class that provides data (it may re-suffle at each call of begin).
       * @param sample_of A functor that gets the sample from the current data.
       */
      template<typename GNGT_PARAMS,
	       typename GRAPH,
	       typename SIMILARITY, 
	       typename LEARN,
	       typename GNGT_EVOLUTION,
	       typename GNGT_SAMPLING,
	       typename SAMPLE_OF>
      void epoch(GNGT_PARAMS& params,
		 GRAPH& g,
		 SIMILARITY& unit_distance,
		 LEARN& unit_learn,
		 GNGT_EVOLUTION& evolution,
		 GNGT_SAMPLING& samples,
		 const SAMPLE_OF& sample_of,
		 int n,
		 bool dynamic_topology = true) {
	for(int e = 0; e < n; ++e) {
	  auto iter = samples.begin();
	  auto end = samples.end();
	  open_epoch(g,evolution);
	  for(;iter != end; ++iter) 
	    submit(params,g,unit_distance,unit_learn,sample_of(*iter),dynamic_topology);
	  close_epoch(params,g,unit_learn,evolution,dynamic_topology);
	}
      }

      namespace temporal {
	/**
	 * This performs a temporal epoch.
	 * @param params   used for adjusting and evolution epochs.
	 * @param params_for_stats   used for statistics epochs.
	 * @param n_adjust the number of epochs for structural adjustment
	 * @param n_evol   the number of epochs for network evolution
	 * @param n_stat   the number of epochs for statistics about new prototype positions
	 * @param samples a class that provides data (it may re-suffle at each call of begin).
	 * @param sample_of A functor that gets the sample from the current data.
	 */
	template<typename GNGT_PARAMS,
		 typename GNGT_PARAMS_FOR_STATS,
		 typename GRAPH,
		 typename SIMILARITY, 
		 typename LEARN,
		 typename GNGT_EVOLUTION,
		 typename VECTOR_OP,
		 typename GNGT_SAMPLING,
		 typename SAMPLE_OF>
	void epoch(GNGT_PARAMS& params,
		   GNGT_PARAMS_FOR_STATS& params_for_stats,
		   GRAPH& g,
		   SIMILARITY& unit_distance,
		   LEARN& unit_learn,
		   GNGT_EVOLUTION& evolution,
		   VECTOR_OP& op,
		   GNGT_SAMPLING& samples,
		   const SAMPLE_OF& sample_of,
		   double dt,
		   int n_adjust,
		   int n_evol,
		   int n_stats) {

	  vq2::temporal::tick(g,op,dt);

	  // Structural adjustment.
	  for(int e = 0; e < n_adjust; ++e) {
	    auto iter = samples.begin();
	    auto end = samples.end();
	    open_epoch(g,evolution);
	    for(;iter != end; ++iter) 
	      submit(params,g,unit_distance,unit_learn,sample_of(*iter),false);
	    close_epoch(params,g,unit_learn,evolution,false);
	  }

	  // Network evolution.
	  for(int e = 0; e < n_evol; ++e) {
	    auto iter = samples.begin();
	    auto end = samples.end();
	    open_epoch(g,evolution);
	    for(;iter != end; ++iter) 
	      submit(params,g,unit_distance,unit_learn,sample_of(*iter),true);
	    close_epoch(params,g,unit_learn,evolution,true);
	  }

	  // Stats for the new state.
	  for(int e = 0; e < n_stats; ++e) {
	    auto iter = samples.begin();
	    auto end = samples.end();
	    open_epoch(g,evolution);
	    for(;iter != end; ++iter) 
	      submit(params_for_stats,g,unit_distance,unit_learn,sample_of(*iter),false);
	    close_epoch(params_for_stats,g,unit_learn,evolution,false);
	    vq2::temporal::frame(g,op); 
	  }
	}
      }

    }
  }
}

#endif
