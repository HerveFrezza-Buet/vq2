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
#include <iomanip>
#include <cstdlib>

typedef vq2::Graph<double,char>          Graph;
typedef Graph::vertex_type                 Vertex;
typedef Graph::ref_vertex_type             RefVertex;

#define WIDTH  20
#define HEIGHT 15
#define GRID_SIZE ((WIDTH)*(HEIGHT))

int at(int w, int h) {
  return h*WIDTH+w;
}

void torus(int& w, int& h) {
  if(w<0)
    w += WIDTH;
  else if(w >= WIDTH)
    w -= WIDTH;
  if(h<0)
    h += HEIGHT;
  else if(h >= HEIGHT)
    h -= HEIGHT;
}

void neighbours(int w, int h,
		int& wS, int& hS,
		int& wE, int& hE) {
  wS = w;
  hS = h+1;

  wE = w+1;
  hE = h;

  torus(wS,hS);
  torus(wE,hE);
}


int main(int argc, char* argv[]) {

  Graph g;
  RefVertex grid[GRID_SIZE];
  int w,h,k;
  int wS, hS;
  int wE, hE;
  
  // Let us create vertices with silly values
  for(k=0;k<GRID_SIZE;++k) 
    grid[k] = (g += 0);

  // Let us connect vertices
  for(w=0;w<WIDTH;++w) 
    for(h=0;h<HEIGHT;++h) {
      neighbours(w,h,
		 wS,hS,
		 wE,hE);
      g.connect(' ', grid[at(w,h)], grid[at(wS,hS)]);
      g.connect(' ', grid[at(w,h)], grid[at(wE,hE)]);
    }
      
  
  // Let us computes distances to the vertex at (3,4).
  RefVertex refN = grid[at(3,4)];
  Vertex* n = &(*refN);
  vq2::algo::distance(g,n);

  // Now, we display it.
  
  for(h=0,k=0;
      h<HEIGHT;
      ++h,std::cout << std::endl)
    for(w=0;w<WIDTH;++w,++k)
      std::cout << std::setw(2) << (*(grid[k])).stuff.distance << ' ';
}
