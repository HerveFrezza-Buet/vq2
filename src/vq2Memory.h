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

#ifndef vq2MEMORY_H
#define vq2MEMORY_H

#include <stack>
#include <vector>
#include <iostream>
#include <iomanip>
#include <set>


namespace vq2 {
  /**
   * This is a memory chunk. For internal use.
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   */
  template<typename DATA>
  class Chunk {
  public:
    typedef DATA data_type;
    void (*onFree)(data_type&);
    DATA content;
    unsigned int ref;
    bool free;

    Chunk(void) : onFree(0), content(), ref(0),  free(true) {}
  };


  /**
   * This is some internal class.
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   */
  template<typename DATA>
  class Free {
  public:
    typedef DATA data_type;
    
    virtual void free(Chunk<data_type>* chunk)=0;
  };
  
  /**
   * This is our pointer type.
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   */
  template<typename DATA>
  class Ref {
  public:
    typedef DATA data_type;
    typedef Chunk<data_type> chunk_type;
    
  private:
    
    Free<data_type>* heap;
    chunk_type* chunk;
    
    void tagAsFree(void) {
      chunk->free = true;
      if(chunk->onFree != 0) {
#ifdef vq2DEBUG
	std::cout << "-->  Calling free callback for content." << std::endl;
#endif
	(*(chunk->onFree))(chunk->content);
	chunk->onFree = 0;
#ifdef vq2DEBUG
	std::cout << "<--  Calling free callback for content (cb cleared)." << std::endl;
#endif
      }
    }
    
  public:

    Ref(Free<data_type>* owner,
	chunk_type* c) : heap(owner), chunk(c) {
      chunk->free = false;
	chunk->ref = 0;
#ifdef vq2DEBUG
	std::cout << "#[" << chunk << "]"
		  << " : ref=" << chunk->ref << ", free=" 
		  << chunk->free << std::endl;
#endif
    }
    
    Ref(void) : heap(0), chunk(0) {}
    
    Ref(const Ref& cp) 
      : heap(cp.heap), chunk(cp.chunk) {}
    

    void clear(void) {
      heap   = (Free<data_type>*)0;
      chunk  = (chunk_type*) 0;
    }

    /**
     * The ref must be valid.
     */
    data_type& operator*(void) {
      return chunk->content;
    }
    
    /**
     * The ref must be valid.
     */
    const data_type& operator*(void) const {
      return chunk->content;
    }
    
    /**
     * Tells wether the ref is invalid.
     */
    bool operator!(void) {
      if(chunk == 0)
	return true;
      
      if(chunk->free) {
	release();
	return true;
      }
      
      return false;
    }

#ifdef vq2DEBUG
    bool operator!(void) const {
      return (chunk == 0) || (chunk->free);
    }
#endif
    
    /**
     * Tells wether the ref is valid.
     */
    bool operator+(void) {
      return !(!(*this));
    }
    
    Ref& operator=(const Ref& cp) {
      if(&cp != this) {
	heap = cp.heap;
	chunk = cp.chunk;
      }
      
      return *this;
    }
#ifdef vq2DEBUG
    friend std::ostream& operator<<(std::ostream& os,const Ref& ref) {
      os << "&[" << ref.chunk << "]";
      return os;
    }
#endif
    
    // This do not release the ref. This has to be done explicitely.
    ~Ref(void) {
    }
    
    void free(void) {
#ifdef vq2DEBUG
	std::cout << "--> [" << chunk << "]"
		  << ".free() : "<< std::endl;
#endif
      if(chunk != 0) {
	tagAsFree();
#ifdef vq2DEBUG
	std::cout << "  Calling a release." << std::endl;
#endif
	release();
      }
#ifdef vq2DEBUG
	std::cout << "<-- [" << chunk << "]"
		  << ".free() : "<< std::endl;
#endif
    }
    
    void take(void) {
      if(heap != 0 && chunk != 0) {
	chunk->ref++;
#ifdef vq2DEBUG
	std::cout << "[" << chunk << "]"
		  << ".take() : ref=" << chunk->ref << ", free=" 
		  << chunk->free << std::endl;
#endif
      }
    }
    
    void release(void) {
#ifdef vq2DEBUG
	std::cout << "--> [" << chunk << "]"
		  << ".release() : "<< std::endl;
#endif
      if(heap != 0 && chunk != 0) {
	if(chunk->ref > 0) {
	  chunk->ref--;
#ifdef vq2DEBUG
	std::cout << "  ref=" << chunk->ref << ", free=" 
		  << chunk->free << std::endl;
#endif
	  if(chunk->ref==0) {
	    tagAsFree();
#ifdef vq2DEBUG
	std::cout << "  freed from heap." << std::endl;
#endif
	    heap->free(chunk);
	  }
	  chunk = 0;
	}
	else {
#ifdef vq2DEBUG
	  std::cout << "!!! Too many releases (ref = " << chunk->ref << ")" << std::endl;
#endif
	  std::cerr << "vq2:Error : Too many releases. Some take() must have been forgotten." << std::endl;
	}
      }
#ifdef vq2DEBUG
	std::cout << "<-- [" << chunk << "]"
		  << ".release() : "<< std::endl;
#endif
    }
  };
  

  /**
   * @short This is a fast memory manager
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   */
  template<typename DATA,int CHUNK_SIZE=1000>
  class Heap : public Free<DATA> {
  private:
    typedef Chunk<DATA> chunk_type;
    std::stack<chunk_type*> free_entries;
    std::stack<std::vector<chunk_type> > memory;
#ifdef vq2DEBUG
    std::set<chunk_type*> allocated;
#endif
    
  public:

    typedef DATA data_type;

    
    typedef Ref<data_type> ref_type;

    typedef void (*memory_func)(data_type&);

    enum {chunk_size = CHUNK_SIZE};
    
    memory_func onAlloc;
    memory_func onFree;

    Heap(void) : Free<data_type>(), onAlloc(0), onFree(0) {}
    ~Heap(void) {}

    /**
     * This returns a ref on a new available value.
     */
    ref_type alloc(void) {
      chunk_type* chunk;
      int i;

      if(free_entries.empty()) {
	memory.push(std::vector<chunk_type>(CHUNK_SIZE));
	for(i=0;i<CHUNK_SIZE;i++)
	  free_entries.push(&(memory.top()[i]));
      }

      chunk = free_entries.top();
      free_entries.pop();
      if(onAlloc != 0)
	(*onAlloc)(chunk->content);
      
#ifdef vq2DEBUG
      allocated.insert(allocated.begin(),chunk);
#endif

      chunk->onFree = onFree;
      return ref_type(this,chunk);
    }

    virtual void free(chunk_type* chunk) {
      free_entries.push(chunk);
#ifdef vq2DEBUG
      allocated.erase(chunk);
#endif
    }

    int nbAllocated(void) const {
      return memory.size()*CHUNK_SIZE - free_entries.size();
    }

#ifdef vq2DEBUG
    void displayAllocated(std::ostream& os) const {
      typename std::set<chunk_type*>::const_iterator iter,end;
      chunk_type* chunk;
      for(iter = allocated.begin(),end = allocated.end();
	  iter != end; ++iter) {
	chunk = *iter;
	os << "    [" << chunk << "] = " << chunk->content << " | " << std::setw(3) << std::setfill('0') << chunk->ref;
	if(chunk->free)
	  os << 'X';
	else
	  os << ' ';
	os << std::endl;
      }
    }
#endif
  };

  /**
   * @short This is a heap-managed list
   * @author <a href="mailto:Herve.Frezza-Buet@supelec.fr">Herve Frezza-Buet</a>
   */
  template<typename DATA>
  class List {
  public:
    typedef DATA data_type;
  private:
    List(void) {}
    List(const List<data_type>& cp);
    List<data_type>& operator=(const List<data_type>& cp);

  public:

    class Link;

    typedef typename Heap<Link>::ref_type LinkRef;
    class Link {
    private:
      Link& operator=(const Link& l) {}

    public:
      typedef DATA data_type;
      LinkRef next;
      LinkRef prev;
      data_type content;
      
      Link(void) : next(), prev(), content() {}
      Link(const Link& l) : next(l.next), prev(l.prev), content(l.content) {}
#ifdef vq2DEBUG
      friend std::ostream& operator<<(std::ostream& os,const Link& l) {
	os << '<' << l.content << '>';
	return os;
      }
#endif
    };
    typedef Link link_type;

  private:

    Heap<Link>& links;
   

  public:

    LinkRef first;
    List(Heap<Link>& mem) : links(mem), first() {};
    ~List(void) {
      clear();
    }

    /**
     * Insert a new link containing elem, at first place.
     */
    void operator+=(const data_type& elem) {
      
#ifdef vq2DEBUG
      std::cout << "--> List " << (void*)this << " += " << elem << std::endl;
#endif

      LinkRef second   = first;
      first            = links.alloc();
      first.take();
      (*first).content = elem;
      (*first).prev.clear();
      (*first).next    = second;
      if(+second)
	(*second).prev = first;
#ifdef vq2DEBUG
      std::cout << "<-- List " << (void*)this << " += " << elem << std::endl;
#endif
    }

#ifdef vq2DEBUG
    void display(std::ostream& os) const {
      for(LinkRef i = first; +i; i = (*i).next)
	os  << std::endl << "        " << (*i);
    }
#endif

    /**
     * The link must belong to the list.
     * @return A link that is the element just next the removed ref.
     */
    LinkRef operator-(LinkRef& l) {
#ifdef vq2DEBUG
      std::cout << "--> List " << (void*)this << " remove " << std::endl;
#endif
      LinkRef res = (*l).next;
      if(!((*l).prev))
	first = (*l).next;
      else
	(*((*l).prev)).next = (*l).next;

      if(+((*l).next))
	(*((*l).next)).prev = (*l).prev;

      l.free();
#ifdef vq2DEBUG
      std::cout << "<-- List " << (void*)this << " remove " << std::endl;
#endif
      return res;
    }

    void clear(void) {
      LinkRef l;
      while(+first) {
	l = (*first).next;
	first.free();
	first = l;
      }
      first.clear();
    }

    bool empty(void) {
      return !first;
    }
  };

  namespace concept {

    typedef int any_type;

    /**
     * For Each functor
     */
    class Iteration {
    public:
      typedef any_type data_type;

      /**
       * This is called for all elemnts in the list, except for the first one.
       * @return true if the element has to be dropped from the list. 
       */
      bool operator()(data_type& elem);
      
    };

  }

  /**
   * This is the usual for each loop
   */
  template<typename LIST,typename ITERATION>
  void for_each(LIST& l,ITERATION& iter) {
#ifdef vq2DEBUG
      std::cout << "--> for_each " << &l <<std::endl;
#endif
    typename LIST::LinkRef i;

    for(i = l.first; 
	+i && iter((*i).content); 
	i = l-i);

    l.first = i;
    if(!(l.first)) {
#ifdef vq2DEBUG
      std::cout << "<-- for_each " << &l <<std::endl;
#endif
      return;
    }

    for(i = (*i).next; 
	+i;) 
      if(iter((*i).content))
	i = l-i;
      else 
	i = (*i).next;
#ifdef vq2DEBUG
      std::cout << "<-- for_each " << &l <<std::endl;
#endif
  }

  /**
   * This is an internal function.
   */
  template<typename REF,typename ITERATION>
  bool for_each_ref_test(REF& i,
			 ITERATION& iter) {
    typename REF::data_type::data_type data_ref((*i).content);
    if(!data_ref)
      return true;
    return iter(*data_ref);
  }

  /**
   * This is a for each for lists containing references. If some
   * element is a reference to some invalid object (for example afreed
   * object that is still referenced), it will be removed from the
   * list and ignored in the iteration.
   */
  template<typename LIST,typename ITERATION>
  void for_each_ref(LIST& l,ITERATION& iter) {
#ifdef vq2DEBUG
      std::cout << "--> for_each_ref " << &l <<std::endl;
#endif
    typename LIST::LinkRef i;

    for(i = l.first; 
	+i && for_each_ref_test(i,iter); 
	i = l-i);

    l.first = i;
    if(!(l.first)) {
#ifdef vq2DEBUG
      std::cout << "<-- for_each_ref " << &l <<std::endl;
#endif
      return;
    }

    for(i = (*i).next; 
	+i;) 
      if(for_each_ref_test(i,iter))
	i = l-i;
      else
	i = (*i).next;
#ifdef vq2DEBUG
    std::cout << "<-- for_each_ref " << &l <<std::endl;
#endif
  }
}


#endif
