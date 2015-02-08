/*   This file is part of vq2
 *
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Herve Frezza-Buet
 *
 *   Contributor :
 *
 *   This library is free softwar; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) s published by the Free Software Foundation; either
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

#ifndef vq2GRAPH_H
#define vq2GRAPH_H

#include <vq2Memory.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <utility>

namespace vq2 {  

  /**
   * @short This is base class for vertices and edge.
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   *
   *  = and default constructor have to work for CONTENT.
   */
  class GraphStuff {
  public:
    /**
     * This is the label of the connected component the container belongs to.
     */
    unsigned int label;

    /**
     * Unefficient containers (false) will be considered as removed
     * for the computation of the connected components.
     */
    bool efficient;

    /**
     * This is a tag for graph algorithms.
     */
    int tag;

    /**
     * This is a distance to some vertex. 
     */
    int distance;


    GraphStuff(void) : label(0), efficient(true), distance(-1) {}
    GraphStuff(const GraphStuff& copy)
      : label(copy.label), 
	efficient(copy.efficient),
	tag(copy.tag),
	distance(copy.distance) {}
    ~GraphStuff(void){}
    
    GraphStuff& operator=(const GraphStuff& copy) {
      if(this != &copy) {
	label     = copy.label;
	efficient = copy.efficient;
	tag       = copy.tag;
	distance  = copy.distance;
      }
      
      return *this;
    }

    void clear(void) {
      this->label     = 0;
      this->efficient = true;
      this->tag       = 0;
      this->distance  = -1;
    }
    
    void read(std::istream& is) {
      char sep;
      is >> label >> efficient >> tag >> distance;
      is.get(sep);
    }

    void write(std::ostream& os) {
      os << label << ' ' << efficient 
	 << ' ' << tag << ' ' << distance << '\n';
    }
  };


  /**
   * @short This is the default constructor (nop) for graphs elements.
   */
  template<typename ANY>
  class DefaultConstructor {
  public:
    void operator()(ANY& that, const ANY& copy) {that = copy;}
  };

  /**
   * @short This is an unoriented graph manager
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   *
   * VERTEX is the value type of the vertex labels (= has to work).<br> 
   * 
   * EDGE is the value type of the edge labels (= has to work).<br> 
   *
   * VERTEX_CHUNK and EDGE_CHUNK are the sizes of memory chunks used for
   * allocating vertices and edges.
   *
   * VERTEX_CONSTRUCTOR and EDGE_CONSTRUCTOR ar functors for
   * initializing the newly allocated data from an existing one. See
   * DefaultConstructor as an example.
   */
  template<typename VERTEX,
	   typename EDGE,
	   typename VERTEX_CONSTRUCTOR = DefaultConstructor<VERTEX>,
	   typename EDGE_CONSTRUCTOR = DefaultConstructor<EDGE>,
	   int VERTEX_CHUNK=1000, int EDGE_CHUNK=1000>
  class Graph {
  private:

  public:

    typedef Graph<VERTEX,EDGE,
		  VERTEX_CONSTRUCTOR,EDGE_CONSTRUCTOR,  
		  VERTEX_CHUNK,
		  EDGE_CHUNK>       graph_type;
    typedef VERTEX                  vertex_value_type;
    typedef EDGE                    edge_value_type;


    class Vertex;
    class Edge;

    typedef Vertex      vertex_type;
    typedef Edge      edge_type;
    typedef Ref<Vertex> ref_vertex_type;
    typedef Ref<Edge> ref_edge_type;

    typedef vq2::List< ref_edge_type >           ref_edge_list_type;
    typedef typename ref_edge_list_type::link_type ref_edge_list_link_type;
    typedef Heap<ref_edge_list_link_type>          ref_edge_list_heap_type;

  private:
    static ref_edge_list_heap_type& refEdgeListHeap(void) {
      static ref_edge_list_heap_type heap;
      return heap;
    }

    typedef vq2::List< ref_vertex_type >               ref_vertex_list_type;
    typedef typename ref_vertex_list_type::link_type ref_vertex_list_link_type;
    typedef Heap<ref_vertex_list_link_type>          ref_vertex_list_heap_type;

    static ref_vertex_list_heap_type& refVertexListHeap(void) {
      static ref_vertex_list_heap_type heap;
      return heap;
    }


    template<typename EDGE_ITERATION>
    class ForEachEdge {
    public:
      EDGE_ITERATION& iter;

      ForEachEdge(EDGE_ITERATION& eiter) : iter(eiter){}

      bool operator()(ref_edge_type& ref) {
	if(!ref) return true;
	edge_type& e = *ref;
	if((!e) || iter(e)) {
	  ref.free();
	  return true;
	}
	return false;
      }
    };

    template<typename REF_EDGE_ITERATION>
    class ForEachEdgeRef {
    public:
      REF_EDGE_ITERATION& iter;

      ForEachEdgeRef(REF_EDGE_ITERATION& eiter) : iter(eiter){}

      bool operator()(ref_edge_type& ref) {
	if(!ref) return true;
	edge_type& e = *ref;
	if((!e) || iter(ref)) {
	  ref.free();
	  return true;
	}
	return false;
      }
    };

    

    template<typename EDGE_ITERATION>
    class ForEachEdgeFromVerticesN1 {
    public:
      EDGE_ITERATION& iter;
      Vertex* that;

      ForEachEdgeFromVerticesN1(EDGE_ITERATION& eiter,
		    Vertex& n) 
	: iter(eiter), that(&n) {}
      
      bool operator()(Edge& e) {
	if(!e) return true;
	if(&(*(e.n1))==that)
	  return iter(e);
	return false;
      }
    };

    template<typename EDGE_ITERATION>
    class ForEachEdgeFromVertices {
    public:
      EDGE_ITERATION& iter;
      ForEachEdgeFromVertices(EDGE_ITERATION& eiter) : iter(eiter){}
      
      bool operator()(vertex_type& vertex) { 
	ForEachEdgeFromVerticesN1<EDGE_ITERATION> functor(iter,vertex);
	vertex.for_each_edge(functor);
	return false;
      }
    };

    template<typename REF_EDGE_ITERATION>
    class ForEachEdgeFromVerticesN1Ref {
    public:
      REF_EDGE_ITERATION& iter;
      Vertex* that;

      ForEachEdgeFromVerticesN1Ref(REF_EDGE_ITERATION& eiter,
				Vertex& n) 
	: iter(eiter), that(&n) {}
      
      bool operator()(ref_edge_type& ref) {
	if(!ref) return true;
	edge_type& e = *ref;
	if(!e) return true;
	if(&(*(e.n1))==that)
	  return iter(ref);
	return false;
      }
    };

    template<typename REF_EDGE_ITERATION>
    class ForEachEdgeFromVerticesRef {
    public:
      REF_EDGE_ITERATION& iter;
      ForEachEdgeFromVerticesRef(REF_EDGE_ITERATION& eiter) : iter(eiter){}
      
      bool operator()(vertex_type& vertex) { 
	ForEachEdgeFromVerticesN1Ref<REF_EDGE_ITERATION> functor(iter,vertex);
	vertex.for_each_edge_ref(functor);
	return false;
      }
    };

    template<typename VERTEX_ITERATION>
    class ForEachVertex {
    public:
      VERTEX_ITERATION& iter;
      ForEachVertex(VERTEX_ITERATION& eiter) : iter(eiter){}
      
      bool operator()(ref_vertex_type& ref) { 
	if(!ref)
	  return true;

	if(iter(*ref)) {
	  ref.free();
	  return true;
	}
	return false;
      }
    };

    template<typename REF_VERTEX_ITERATION>
    class ForEachVertexRef {
    public:
      REF_VERTEX_ITERATION& iter;
      ForEachVertexRef(REF_VERTEX_ITERATION& eiter) : iter(eiter){}
      
      bool operator()(ref_vertex_type& ref) { 
	if(!ref)
	  return true;
	
	if(iter(ref)) {
	  ref.free();
	  return true;
	}
	return false;
      }
    };


    class ClearEdgeTag {
    public:
      bool operator()(edge_type& e) {
	e.stuff.tag = 0;
	return false;
      }
    };

    class CountNodes {
    public:
      unsigned int nb;
      CountNodes(void) : nb(0) {}
      bool operator()(vertex_type& n) {
	++nb;
	return false;
      }
    };

    class ClearTag {
    public:
      bool operator()(vertex_type& n) {
	ClearEdgeTag clear;
	n.stuff.tag = 0;
	n.for_each_edge(clear);
	return false;
      }
    };

    class FreeVertex {
    public:
      bool operator()(vertex_type& vertex) {
	return true;
      }
    };

    class FreeEdge {
    public:
      bool operator()(ref_edge_type& ref) {
	ref.free();
	return true;
      }
    };

    class ReleaseEdge {
    public:
      bool operator()(ref_edge_type& ref) {
	ref.release();
	return true;
      }
    };

    class ReleaseVertex {
    public:
      bool operator()(ref_vertex_type& ref) {
	ref.release();
	return true;
      }
    };

  public:
    

    class Vertex {
    private:

      Vertex& operator=(const Vertex& n) {
	// if(this != &n) {
	//   stuff = n.stuff;
	//   value = n.value;
	// }
	// return *this;
      }

    public:
      
      GraphStuff stuff;
      vertex_value_type value;
      ref_edge_list_type edges;

      Vertex(const Vertex& e) 
	: stuff(), 
	  value(), 
	  edges(graph_type::refEdgeListHeap()) {/* for compatibility with internal stl manipulation*/}

      Vertex(void) 
	: stuff(), 
	  value(), 
	  edges(graph_type::refEdgeListHeap()) {}

      Vertex(const vertex_value_type& vertex_value) 
	: stuff(), 
	  value(vertex_value), 
	  edges(graph_type::refEdgeListHeap()) {}

      /**
       * for internal use.
       */
      void onFree(void) {
	FreeEdge free_edge;
	for_each(edges,free_edge);
      }

      /**
       * for internal use.
       */
      void onAlloc(void) {
	stuff.clear();
	edges.clear();
      }

      template<typename EDGE_ITERATION>
      void for_each_edge(EDGE_ITERATION& edge_iteration) {
#ifdef vq2DEBUG
	std::cout << "--> for_each_edge : vertex=" << this << std::endl;
#endif
	ForEachEdge<EDGE_ITERATION> check(edge_iteration);
	for_each(edges,check);
#ifdef vq2DEBUG
	std::cout << "<-- for_each_edge : vertex=" << this << std::endl;
#endif
      }

      template<typename REF_EDGE_ITERATION>
      void for_each_edge_ref(REF_EDGE_ITERATION& edge_iteration) {
#ifdef vq2DEBUG
	std::cout << "--> for_each_edge_ref : vertex=" << this << std::endl;
#endif
	ForEachEdgeRef<REF_EDGE_ITERATION> check(edge_iteration);
	for_each(edges,check);
#ifdef vq2DEBUG
	std::cout << "<-- for_each_edge_ref : vertex=" << this << std::endl;
#endif
      }

#ifdef vq2DEBUG
      friend std::ostream& operator<<(std::ostream& os,const Vertex& vertex) {
	os << vertex.value << " (edges=" << &(vertex.edges) << ')';
	vertex.edges.display(os);
	return os;
      }
#endif
    };

    class Edge {
    private:

      Edge& operator=(const Edge& e) {
	// if(this != &e) {
	//   stuff = e.stuff;
	//   n1 = e.n1;
	//   n2 = e.n2;
	// }
	// return *this;
      }

    public:

      GraphStuff stuff;
      edge_value_type value;
      ref_vertex_type n1,n2;

      Edge(const Edge& e) {/* for compatibility with internal stl manipulation*/}
      Edge(void) : stuff(), value(), n1(), n2() {}

      /**
       * for internal use.
       */
      void onFree(void) {
#ifdef vq2DEBUG
	std::cout << "--> Edge : onFree : releasing " << n1 << " and " << n2 << std::endl;
#endif
	n1.release();
	n2.release();
#ifdef vq2DEBUG
	std::cout << "<--Edge : onFree" << std::endl;
#endif
      }

      /**
       * for internal use.
       */
      void onAlloc(void) {
	stuff.clear();
	n1.clear();
	n2.clear();
      }


#ifdef vq2DEBUG
      friend std::ostream& operator<<(std::ostream& os,const Edge& edge) {
	
	os << edge.value << "~(";
	if(!(edge.n1))
	  os << "#";
	else
	  os << (*(edge.n1)).value;
	os << ',';
	if(!(edge.n2))
	  os << "#";
	else
	  os << (*(edge.n2)).value;
	os << ')';
	return os;
      }
#endif

      /**
       * @return true is the edge is invalid (and releases vertices)
       */
      bool operator!(void) {
#ifdef vq2DEBUG
	std::cout << "--> !edge : value=" << value << std::endl;
#endif
	if(!n1) {
	  n2.release();
#ifdef vq2DEBUG
	  std::cout << "<-- !edge : value=" << value << " : TRUE" << std::endl;
#endif
	  return true;
	}

	if(!n2) {
	  n1.release();
#ifdef vq2DEBUG
	  std::cout << "<-- !edge : value=" << value << " : TRUE" << std::endl;
#endif
	  return true;
	}

#ifdef vq2DEBUG
	std::cout << "<-- !edge : value=" << value << " : FALSE" << std::endl;
#endif
	return false;
      }

      /**
       * Tells wether the edge is valid.
       */
      bool operator+(void) {
	return !(!(*this));
      }
    };

    typedef vq2::Heap<Edge>   edge_heap_type;

    static edge_heap_type& edgeHeap(void) {
      static edge_heap_type heap;
      return heap;
    }

    typedef vq2::Heap<Vertex>   vertex_heap_type;

    static vertex_heap_type& vertexHeap(void) {
      static vertex_heap_type heap;
      return heap;
    }


  private:

    ref_vertex_list_type vertices;

  public:

    /**
     * For internal use.
     */
    static void onFreeVertex(Vertex& n) {n.onFree();}
    /**
     * For internal use.
     */
    static void onFreeEdge(Edge& e) {e.onFree();}

    /**
     * For internal use.
     */
    static void onAllocVertex(Vertex& n) {n.onAlloc();}
    /**
     * For internal use.
     */
    static void onAllocEdge(Edge& e) {e.onAlloc();}

    /**
     * For internal use.
     */
    static void onAllocRefVertex(ref_vertex_list_link_type& link) {link.content.clear();}
    /**
     * For internal use.
     */
    static void onAllocRefEdge(ref_edge_list_link_type& link) {link.content.clear();}

  public:
    friend class Component;

    class Component {
    private:

      friend class Graph<VERTEX,EDGE,
			 VERTEX_CONSTRUCTOR,EDGE_CONSTRUCTOR,  
			 VERTEX_CHUNK,
			 EDGE_CHUNK>;
      int label;
      ref_vertex_list_type vertices;
      ref_edge_list_type edges;

      Component(void) 
	: label(-1),
	  vertices(refVertexListHeap()),
	  edges(refEdgeListHeap())  {}
      ~Component(void) {
	ReleaseEdge release_edge;
	ReleaseVertex release_vertex;
	for_each(edges,release_edge);
	for_each(vertices,release_vertex);
	vertices.clear();
	edges.clear();
      }

      void operator+=(ref_vertex_type& ref) {
	if(+ref) {
	  ref.take();
	  vertices += ref;
	}
      }

      /**
       * Only untagged edges are added, and then the edge is tagged.
       */
      void operator+=(ref_edge_type& ref) {
	edge_type& e = *ref;
	if(+ref && (e.stuff.tag == 0)) {
	  e.stuff.tag = 1;
	  ref.take();
	  edges += ref;
	}
      }

    public:

      template<typename VERTEX_ITERATION>
      void for_each_vertex(VERTEX_ITERATION& vertex_iteration) {
	ForEachVertex<VERTEX_ITERATION> functor(vertex_iteration);
	for_each(vertices,functor);
      }

      template<typename EDGE_ITERATION>
      void for_each_edge(EDGE_ITERATION& edge_iteration) {
	ForEachEdge<EDGE_ITERATION> check(edge_iteration);
	for_each(edges,check);
      }

      template<typename REF_VERTEX_ITERATION>
      void for_each_vertex_ref(REF_VERTEX_ITERATION& vertex_iteration) {
	ForEachVertexRef<REF_VERTEX_ITERATION> functor(vertex_iteration);
	for_each(vertices,functor);
      }

      template<typename REF_EDGE_ITERATION>
      void for_each_edge_ref(REF_EDGE_ITERATION& edge_iteration) {
	ForEachEdgeRef<REF_EDGE_ITERATION> check(edge_iteration);
	for_each(edges,check);
      }
    };

    
  private:
    
    std::vector<Component*>   components;
    
    unsigned int free_label;
    unsigned int nextLabel(void) {
      if(free_label==0)
	++free_label;
      return free_label++;
    }
    

  public:
    
    Graph(void) 
      : vertices(refVertexListHeap()),
	free_label(1) {
      vertexHeap().onFree = onFreeVertex;
      edgeHeap().onFree = onFreeEdge;
      vertexHeap().onAlloc = onAllocVertex;
      edgeHeap().onAlloc = onAllocEdge;
      refVertexListHeap().onAlloc = onAllocRefVertex;
      refEdgeListHeap().onAlloc = onAllocRefEdge;
    }
    ~Graph(void) {clear();}
    
    unsigned int nbVertices(void) {
      CountNodes counter;
      this->for_each_vertex(counter);
      return counter.nb;
    }
  

    void clear(void) {
      FreeVertex free_vertex;
      for_each_vertex(free_vertex);
      clearComponents();
    }

    /**
     * This clears the memory related to connected components. You might
     * need this to save memory, but this method is called internally
     * when new connected component computation is performed.
     */
    void clearComponents(void) {
      typename std::vector<Component*>::iterator iter,end;
      for(iter=components.begin(), end=components.end();
	  iter != end;
	  ++iter)
	delete (*iter);
      components.clear();
    }

  private:
    
    class ExtendComponent {
    private:
      
      std::queue<ref_vertex_type*>& fifo;
      Component& comp;
      vertex_type* n;

    public:
      
      ExtendComponent(std::queue<ref_vertex_type*>& f,
		    Component& c,
		    vertex_type* vertex)
	: fifo(f), comp(c), n(vertex) {}

      bool operator()(ref_edge_type& ref) {
	if(!ref)
	  return true;
	
	edge_type& e = *ref;
	if(e.stuff.tag == 0) {
	  if(e.stuff.efficient) {
	    comp += ref;
	    vertex_type* nn;
	    ref_vertex_type* refnn;
	    if((nn = &(*(e.n1))) == n) {
	      nn = &(*(e.n2));
	      refnn = &(e.n2);
	    }
	    else
	      refnn = &(e.n1);
	    
	    if(nn->stuff.tag == 0) {
	      nn->stuff.tag = 1;
	      if(nn->stuff.efficient)
		fifo.push(refnn);
	      else {
		functor::UnefficientEdge<graph_type> unefficient_edge;
		nn->for_each_edge(unefficient_edge);
	      }
	    }
	  }
	  else
	    e.stuff.tag = 1;
	}

	return false;
      }
    };

    class NewComponent {
      std::queue<ref_vertex_type*> fifo;
      std::vector<Component*>& components;
    public:
      NewComponent(std::vector<Component*>& comps) : components(comps) {}

      bool operator()(ref_vertex_type& ref) {
	Component* comp;
	
	if(!ref)
	  return true;

	vertex_type* n = &(*ref);
	ref_vertex_type* popref;

	if(n->stuff.tag == 0) {
	  n->stuff.tag = 1;
	  if(n->stuff.efficient) {
	    fifo.push(&ref);
	    comp = new Component();
	    components.push_back(comp);
	    while(!fifo.empty()) {
	      popref = fifo.front();
	      fifo.pop();
	      *comp += *popref;
	      n = &(*(*popref));
	      ExtendComponent extend(fifo,*comp,n);
	      for_each(n->edges,extend);
	    }
	  }
	  else {
	    functor::UnefficientEdge<graph_type> unefficient_edge;
	    n->for_each_edge(unefficient_edge);
	  }
	}
	return false;
      }
    };


    struct invertorderop {
      bool operator()(unsigned int i1, unsigned int i2) const
      {
	return i2 < i1;
      }
    };

    class OccLabel {
    public:
      /* occ[i,j] = d <=> d edges of component i have a label j. */
      std::map<std::pair<unsigned int,unsigned int>, 
	       unsigned int> occurences;
      unsigned int comp;

      OccLabel(void) : occurences() {}

      bool operator()(edge_type& e) {
	std::map<std::pair<unsigned int,unsigned int>, 
		 unsigned int>::iterator occ;
	unsigned int label  = e.stuff.label;

	if(label!=0) {
	    occ = occurences.find(std::pair<unsigned int,unsigned int>(comp,label));
	    if(occ != occurences.end())
	      occ->second++;
	    else
	      occurences[std::pair<unsigned int,unsigned int>(comp,label)]=1;
	  }
	return false;
      }
    };

    class LabelEdge {
    public:
      unsigned int label;
      bool operator()(edge_type& e) {
	e.stuff.label = label;
	return false;
      }
    };

    class LabelVertex {
    public:
      unsigned int label;
      bool operator()(vertex_type& n) {
	n.stuff.label = label;
	return false;
      }
    };

  public:
      
    /**
     * @param do_labelling Tells whether the labelling has to be done. Labels are irrelevant if not.
     * @param connected_components It is a map of pointers to internal components (do not free). The key is the label. The pointers are invalidated by a new call to the method. The call clears the parameter.
     */
    void computeConnectedComponents(std::map<unsigned int,Component*>& connected_components,
				    bool do_labelling) {
      ClearTag clear_tag;
      NewComponent newcomp(components);
      typename std::vector<Component*>::iterator citer,cend;
      unsigned int l;

      for_each_vertex(clear_tag);
      clearComponents();
      for_each(vertices,newcomp);

      connected_components.clear();

      if(do_labelling) {
	std::map<std::pair<unsigned int,unsigned int>, 
		 unsigned int>::iterator occ;
	// Elements are stored in descending order.
	std::multimap<unsigned int, 
		      std::pair<unsigned int,
				unsigned int>, 
		      invertorderop> secnerucco;
	OccLabel occlabel;
	unsigned int comp, label;
	std::vector<unsigned int> labelling(components.size(),0);
	typename std::vector<unsigned int>::iterator pliter,plend;


	// Let us compute the map : [comp,label] -> nb : 'nb' edges of
	// connected component 'comp' have label 'label'
	for(citer = components.begin(), 
	      cend = components.end(), 
	      occlabel.comp = 0;
	    citer != cend;
	    ++citer, 
	      occlabel.comp++)
	  (*citer)->for_each_edge(occlabel);
	
	// Now, we build the reverse map.
	// nb -> (comp,label)
	for(occ = occlabel.occurences.begin(); 
	    occ != occlabel.occurences.end();
	    ++occ)
	  secnerucco.insert(std::pair<unsigned int, 
				      std::pair<unsigned int,
						unsigned int > >(occ->second,occ->first));

	// Now, we can start real labelling, from main labels in the edges.
	typename std::multimap<unsigned int, std::pair<unsigned int,unsigned int>, invertorderop>::iterator cco;
	std::set<unsigned int> used_labels;
	for(cco = secnerucco.begin(); cco != secnerucco.end(); cco++) {
	  comp = cco->second.first;
	  label = cco->second.second;
	  if(labelling[comp] == 0 && (used_labels.count(label)==0))
	    labelling[comp] = label;
	    used_labels.insert(label);
	}

	// There may be some unlabeled components.
	for(comp = 0, 
	      pliter = labelling.begin(),
	      plend  = labelling.end();
	    pliter != plend;
	    ++comp,++pliter)
	  if(*pliter == 0) {
	    *pliter = nextLabel();
	  }
	  

	LabelVertex label_vertex;
	LabelEdge label_edge;
	// Label all vertices and edges in each component.
	for(citer = components.begin(),
	      cend = components.end(), 
	      pliter = labelling.begin();
	    citer != cend;
	    ++citer, ++pliter) {
	  label_vertex.label = *pliter;
	  label_edge.label = *pliter;
	  (*citer)->for_each_vertex(label_vertex);
	  (*citer)->for_each_edge(label_edge);
	  connected_components[*pliter]=*citer;
	}
	
      }
      else {
	for(l = 1, citer = components.begin(), cend = components.end();
	    citer != cend;
	    ++citer,++l) {
	  connected_components[l]=*citer;
	}
      }
    }
    

    /**
     * This adds a vertex.
     */
    ref_vertex_type operator+=(const vertex_value_type& vertex_value) {
#ifdef vq2DEBUG
      std::cout << "--> Graph " << (void*)this << " +=  " << vertex_value << std::endl;
#endif
      VERTEX_CONSTRUCTOR constructor;
      ref_vertex_type ref = vertexHeap().alloc();
      vertices += ref;
      constructor((*ref).value,vertex_value);
      (*ref).stuff.clear();
      ref.take();
#ifdef vq2DEBUG
      std::cout << "<-- Graph " << (void*)this << " +=  " << vertex_value << std::endl;
#endif
      return ref;
    }

    ref_edge_type connect(const edge_value_type& edge_value,
			  ref_vertex_type n1,
			  ref_vertex_type n2) {
#ifdef vq2DEBUG
      std::cout << "--> Graph " << (void*)this << " connect  " << edge_value << std::endl;
#endif
      EDGE_CONSTRUCTOR constructor;
      ref_edge_type ref = edgeHeap().alloc();
      (*ref).n1    = n1;
      (*ref).n2    = n2;
      constructor((*ref).value,edge_value);
      n1.take();
      n2.take();
      (*n1).edges += ref;
      ref.take();
      (*n2).edges += ref;
      ref.take();
      
#ifdef vq2DEBUG
      std::cout << "<-- Graph " << (void*)this << " connect  " << edge_value << std::endl;
#endif
      return ref;
    }

    void displayMemory(std::ostream& os) {
      os << "Graph internal memory : " << std::endl;
      
#ifdef vq2DEBUG
      os << "  Vertices" << std::endl;
      vertexHeap().displayAllocated(os);
      os << "  Edges" << std::endl;
      edgeHeap().displayAllocated(os);
      os << "  Vertices links" << std::endl;
      refVertexListHeap().displayAllocated(os);
      os << "  Edges links" << std::endl;
      refEdgeListHeap().displayAllocated(os);
#else
      os << std::setw(10) << vertexHeap().nbAllocated() << " vertices." << std::endl
	 << std::setw(10) << edgeHeap().nbAllocated() << " edges." << std::endl
	 << std::setw(10) << refVertexListHeap().nbAllocated() 
	 << " vertex references list link." << std::endl
	 << std::setw(10) << refEdgeListHeap().nbAllocated() 
	 << " edge references list link." << std::endl;
#endif
    }

    template<typename VERTEX_ITERATION>
    void for_each_vertex(VERTEX_ITERATION& vertex_iteration) {
      ForEachVertex<VERTEX_ITERATION> functor(vertex_iteration);
      for_each(vertices,functor);
    }

    template<typename REF_VERTEX_ITERATION>
    void for_each_vertex_ref(REF_VERTEX_ITERATION& ref_vertex_iteration) {
      ForEachVertexRef<REF_VERTEX_ITERATION> functor(ref_vertex_iteration);
      for_each(vertices,functor);
    }

    template<typename EDGE_ITERATION>
    void for_each_edge(EDGE_ITERATION& edge_iteration) {
      ForEachEdgeFromVertices<EDGE_ITERATION> functor(edge_iteration);
      for_each_ref(vertices,functor);
    }

    template<typename REF_EDGE_ITERATION>
    void for_each_edge_ref(REF_EDGE_ITERATION& edge_iteration) {
      ForEachEdgeFromVerticesRef<REF_EDGE_ITERATION> functor(edge_iteration);
      for_each_ref(vertices,functor);
    }

  private:

    class CountEdge {
    public:
      int nb;
      CountEdge(void) : nb(0) {}
      bool operator()(edge_type& edge) {++nb;return false;}
    };


    class CountVertex {
    public:
      std::map<int,Vertex*> vertices;
      std::map<Vertex*,int> idfs;
      int nb;
      CountVertex(void) : vertices(), nb(0) {}
      bool operator()(vertex_type& vertex) {
	vertices[nb]   = &vertex;
	idfs[&vertex] = nb;
	++nb;
	return false;
      }
    };

    class PrintEdge {
    public:
      CountVertex& cn;
      std::ostream& os;
      PrintEdge(CountVertex& count_vertex,
		std::ostream& output_stream) 
	: cn(count_vertex), os(output_stream) {}

      bool operator()(edge_type& e) {
	vertex_type *n1 = &(*(e.n1));
	vertex_type *n2 = &(*(e.n2));
	e.stuff.write(os);
	os << e.value << '\n';
	os << cn.idfs[n1] << ' ' << cn.idfs[n2] << std::endl;
	return false;
      }
    };
    
    friend std::ostream& operator<<(std::ostream& os, graph_type& graph) {
      CountEdge count_edge;
      CountVertex count_vertex;
      PrintEdge print_edge(count_vertex,os);
      typename std::map<int,Vertex*>::iterator iter,end;
      vertex_type* n;

      graph.for_each_vertex(count_vertex);
      os << count_vertex.nb << std::endl;

      for(iter = count_vertex.vertices.begin(), end = count_vertex.vertices.end();
	  iter != end;
	  ++iter) {
	n = (*iter).second;
	n->stuff.write(os);
	os << n->value << '\n';
      }

      graph.for_each_edge(count_edge);
      os << count_edge.nb << '\n';
      graph.for_each_edge(print_edge);
      return os;
    }

    friend std::istream& operator>>(std::istream& is,
				    graph_type& graph) {
      char sep;
      int nb,i,n1,n2;
      std::vector<ref_vertex_type> vertices;
      GraphStuff stuff;
      vertex_value_type vertex_value;
      edge_value_type edge_value;

      graph.clear();

      is >> nb;
      is.get(sep);
      vertices.resize(nb);
      for(i=0;i<nb;++i) {
	stuff.read(is);
	is >> vertex_value;
	is.get(sep);
	ref_vertex_type ref(graph += vertex_value);
	(*ref).stuff = stuff;
	vertices[i] = ref;
      }

      is >> nb;
      is.get(sep);
      for(i=0;i<nb;++i) {
	stuff.read(is);
	is >> edge_value;
	is.get(sep);
	is >> n1 >> n2;
	is.get(sep);
	ref_edge_type ref(graph.connect(edge_value,vertices[n1],vertices[n2]));
	(*ref).stuff = stuff;
      }
      
      return is;
    }

  };
}

#endif
