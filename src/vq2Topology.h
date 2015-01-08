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

#ifndef vq2TOPOLOGY_H
#define vq2TOPOLOGY_H

#include <vq2Concept.h>
#include <vq2Functors.h>
#include <utility>
#include <queue>

namespace vq2 {

  namespace algo {

    namespace make {
      
      /**
       * @short This builds a line.
       * @param vinit a vertex initialization function returning a vertex value, and taking w line coordinate as arguments (unsigned int).
       * @param einit an edge initialization function returning an edge value, and taking w,w' line coordinates as arguments (unsigned int).
       */
      template<typename GRAPH,
	       typename VERTEX_INIT,
	       typename EDGE_INIT>
       std::vector<typename GRAPH::ref_vertex_type> line(GRAPH& g, 
							 unsigned int length, 
							 const VERTEX_INIT& vinit,
							 const EDGE_INIT& einit) {
	std::vector<typename GRAPH::ref_vertex_type> line;
	unsigned int w;
	line.resize(length);
	for(w=0; w<length; ++w) 
	  line[w] = (g += vinit(w));
	for(w=0; w<length-1; ++w) 
	  g.connect(einit(w,w+1),line[w],line[w+1]);
	return line;
      }
      
      /**
       * @short This builds a ring.
       * @param vinit a vertex initialization function returning a vertex value, and taking w line coordinate as arguments (unsigned int).
       * @param einit an edge initialization function returning an edge value, and taking w,w' line coordinates as arguments (unsigned int).
       * @param line is set/reset by the mfunction call.
       */
      template<typename GRAPH,
	       typename VERTEX_INIT,
	       typename EDGE_INIT>
      std::vector<typename GRAPH::ref_vertex_type> ring(GRAPH& g, 
							unsigned int length, 
							const VERTEX_INIT& vinit,
							const EDGE_INIT& einit) {
	std::vector<typename GRAPH::ref_vertex_type> line;
	unsigned int w;
	line.resize(length);
	for(w=0; w<length; ++w) 
	  line[w] = (g += vinit(w));
	for(w=0; w<length-1; ++w) 
	  g.connect(einit(w,w+1),line[w],line[w+1]);
	if(length>2)
	  g.connect(einit(w,0),line[w],line[0]);
	return line;
      }
      
      /**
       * @short This builds a grid.
       * @param vinit a vertex initialization function returning a vertex value, and taking w,h grid coordinates as arguments (unsigned int).
       * @param einit an edge initialization function returning an edge value, and taking w,h,w',h' grid coordinates as arguments (unsigned int).
       */
      template<typename GRAPH,
	       typename VERTEX_INIT,
	       typename EDGE_INIT>
      std::vector<typename GRAPH::ref_vertex_type> grid(GRAPH& g, 
							unsigned int width, 
							unsigned int height,
							const VERTEX_INIT& vinit,
							const EDGE_INIT& einit) {
	std::vector<typename GRAPH::ref_vertex_type> grid;
	unsigned int w,h,k;

	grid.resize(width*height);
	for(h=0, k=0; h<height; ++h)
	  for(w=0; w<width; ++w, ++k) 
	    grid[k] = (g += vinit(w,h));

	for(h=0; h<height-1; ++h)
	  for(w=0; w<width-1; ++w) {
	    g.connect(einit(w,h,w,h+1),grid[h*width+w],grid[(h+1)*width+w]);
	    g.connect(einit(w,h,w+1,h),grid[h*width+w],grid[h*width+w+1]);
	  }

	if(height > 0)
	  for(w=0; w<width-1; ++w)
	    g.connect(einit(w,height-1,w+1,height-1),grid[(height-1)*width+w],grid[(height-1)*width+w+1]);
	if(width > 0)
	  for(h=0; h<height-1; ++h)
	    g.connect(einit(width-1,h,width-1,h+1),grid[h*width+width-1],grid[(h+1)*width+width-1]);
	return grid;
      }
    }

    /**
     * This computes the distance, in terms of edges, between n and
     * all vertices.  For each vertex n, n.stuff.distance contains the distance
     * (-1 if not connected to n through a path).
     */
    template<typename GRAPH>
    inline void distance(GRAPH& g,
			 typename GRAPH::vertex_type* n) {
      std::queue<typename GRAPH::vertex_type*> fifo;
      
      // We clear distances.
      functor::ClearDistance<GRAPH> clear_distance;
      g.for_each_vertex(clear_distance);
      
      fifo.push(n);
      n->stuff.distance = 0;
      
      functor::TagDistanceAtEdge<GRAPH> tag_distances(fifo);
      while(!fifo.empty()) {
	n = fifo.front();
	fifo.pop();
	tag_distances.n = n;
	tag_distances.dist = n->stuff.distance+1;
	n->for_each_edge(tag_distances);
      }
    }
  }
}

#endif
