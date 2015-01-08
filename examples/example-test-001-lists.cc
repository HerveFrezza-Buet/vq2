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
#include <cstdlib>
#include <list>



typedef vq2::List<int> FastList;

// This is a display function
class Display {
public:
  bool operator()(int nb) {
    std::cout << ' ' << nb; 
    return false; // the element should not be removed.
  }
};

// This functor counts the number of items.
class Count {
public:
  int size;
  bool operator()(int nb) {size++; return false;}
};

// This functor says that odd numbers have to be removed from a list.
class RemoveOdds {
public:
  bool operator()(int nb) {return nb % 2;}
};

int main(int argc, char* argv[]) {

  if(argc!=2) {
    std::cout << "Usage : " << argv[0] << " <big-list-size>" << std::endl;
    return 0;
  }

  vq2::Heap<FastList::Link> list_memory;
  FastList                    l(list_memory);
  vq2::Time                 timer;
  double a,aa,b,bb,c,cc;

  int i;
  int big = atoi(argv[1]);
  int all;

  for(i=0;i<50;++i) l += i;
  all = list_memory.nbAllocated();

  // Let is display
  std::cout << "Full list :";
  Display display_functor;
  vq2::for_each(l,display_functor);
  std::cout << std::endl;

  // Let us remove odd numbers
  RemoveOdds remodd_functor;
  vq2::for_each(l,remodd_functor);
  std::cout << "Even list :";
  vq2::for_each(l,display_functor);
  std::cout << std::endl;

  // Let us count the elements
  Count count_functor;
  count_functor.size=0;
  vq2::for_each(l,count_functor);
  std::cout << "There are " << count_functor.size << " elements." << std::endl;
  
  std::cout << "Heap allocated after making the list: " << all << std::endl;
  std::cout << "Heap allocated after removing odds  : " << list_memory.nbAllocated() << std::endl;

  // Let us clear the list
  l.clear();
  std::cout << "Heap allocated after clearing       : " << list_memory.nbAllocated() << std::endl;

  // Let us compute time

  std::cout << std::endl
	    << "##### with vq2::List ##### " << std::endl;

  std::cout << "Making a " << big << " sized list : " << std::flush;
  timer.start();
  for(i=0;i<big/2;++i) {
    l += 0;
    l += 1;
  }
  timer.stop();
  a = timer.getTime();
  std::cout << a << " s." << std::endl;

  std::cout << "Clearing it : " << std::flush;
  timer.start();
  l.clear();
  timer.stop();
  b = timer.getTime();
  std::cout << b << " s." << std::endl;

  std::cout << "Reallocate it : " << std::flush;
  timer.start();
  for(i=0;i<big/2;++i) {
    l += 0;
    l += 1;
  }
  timer.stop();
  c = timer.getTime();
  std::cout << c << " s." << std::endl;


  std::cout << std::endl
	    << "##### with std::list ##### " << std::endl;

  std::cout << "Making a " << big << " sized std::list : " << std::flush;
  std::list<int> ll;
  timer.start();
  for(i=0;i<big/2;++i) {
    ll.push_front(0);
    ll.push_front(1);
  }
  timer.stop();
  aa = timer.getTime();
  std::cout << aa << " s." << std::endl;

  std::cout << "Clearing it : " << std::flush;
  timer.start();
  ll.clear();
  timer.stop();
  bb = timer.getTime();
  std::cout << bb << " s." << std::endl;

  std::cout << "Reallocate it : " << std::flush;
  timer.start();
  for(i=0;i<big/2;++i) {
    ll.push_front(0);
    ll.push_front(1);
  }
  timer.stop();
  cc = timer.getTime();
  std::cout << cc << " s." << std::endl;

  
  

  std::cout << std::endl
	    << "##### acceleration ##### " << std::endl;

  std::cout << "Making a " << big << " sized list : " << aa/a << std::endl;
  std::cout << "Clearing it : " << bb/b << std::endl;
  std::cout << "Reallocate it : " << cc/c << std::endl;
  
}
