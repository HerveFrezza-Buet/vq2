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

#ifndef vq2BY_DEFAULT_H
#define vq2BY_DEFAULT_H

#include <vq2Concept.h>

namespace vq2 {
  namespace by_default {

    /**
     * This provides a class for basic vector operation functor when
     * the type is an actual vector (v = 0 have to work).
     */
    template<typename ANY>
    class VectorOp : public vq2::fits<vq2::concept::VectorOp> {
    public:
      void raz(ANY& v)                {v = 0;}
      void add(ANY& v, const ANY& w)  {v += w;}
      void div(ANY& v, double coef)   {v /= coef;}
      void mul(ANY& v, double coef)   {v *= coef;}
    };
  }
}

#endif
