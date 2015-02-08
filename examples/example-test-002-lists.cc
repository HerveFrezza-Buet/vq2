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

#include <vq2.h>
#include <iostream>


// Let us play with list of pointers... I mean list of our home made references.

typedef vq2::Heap<int>           Integers; // This will provide integers.
typedef Integers::ref_type         IntRef;   // This is a pointer to some integer.
typedef vq2::List<IntRef>        IntList;  // This is a list of pointers.
typedef vq2::Heap<IntList::Link> Links;    // This is the memory for the list.

// This is a display function
class Display {
public:
  bool operator()(int nb) {
    std::cout << ' ' << nb; 
    return false; // the element should not be removed.
  }
};

int main(int argc, char* argv[]) {

  Links    links;
  IntList  l(links);
  Integers integers;
  IntRef   ref;
  IntRef   ten,fifteen;
  int i;

  // Let us allocate integers, put them (by reference) in a list.
  for(i=0;i<10;++i) {
    ref  = integers.alloc(); // we get an integer.
    *ref = i;                // we set it to some value.
    l += ref;                // we put a refernce to it in a list.
    ref.take();              // we increase the reference counter for that integer.
  }

  // For 10, we do the same...
  ref  = integers.alloc();
  *ref = i;                
  l += ref;                
  ref.take(); 
  // ... but we also manage a reference to 10 outside the list. We
  // have to increase the reference counter one more time.
  ten = ref;
  ten.take();

  // Let us allocate new values
  for(i++;i<15;++i) {
    ref  = integers.alloc(); 
    *ref = i;               
    l += ref;               
    ref.take();             
  }

  // For 15, we proceed as for 10
  ref  = integers.alloc();
  *ref = i;                
  l += ref;                
  ref.take(); 
  fifteen = ref;
  fifteen.take();

  // Let us allocate new values
  for(i++;i<20;++i) {
    ref  = integers.alloc(); 
    *ref = i;               
    l += ref;               
    ref.take();             
  }
  
  // Let us display the list. A convenient for loop exists for lists
  // containing references.
  std::cout << "Full list :";
  Display display_functor;
  vq2::for_each_ref(l,display_functor);
  std::cout << std::endl;

  // Now, let free 10 and 15.
  std::cout << "There are " << integers.nbAllocated() 
	    << " integers allocated." << std::endl;
  ten.free();
  fifteen.free();
  std::cout << "After freeing 10 and 15, there are still " 
	    << integers.nbAllocated() << " integers allocated." << std::endl;
  
  // Since 10 and 15 are still refered in the list, they are not
  // really freed. Nevertheless, iterating on the list will help to to
  // clean that.
  std::cout << "The list without 10 and 15 :";
  vq2::for_each_ref(l,display_functor);
  std::cout << std::endl;
  std::cout << "After displaying the list, there are now " 
	    << integers.nbAllocated() << " integers allocated." << std::endl;
  
}
