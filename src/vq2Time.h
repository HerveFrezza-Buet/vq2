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

#ifndef vq2TIME_H
#define vq2TIME_H

#include <time.h>
#include <sys/time.h>

namespace vq2 {
  /**
   * @short Class for dealing with time. From mirage library.
   */
  class Time {
  
  private:
    
    struct timeval _starttime, _stoptime;
    struct timezone tz;  
  
  public:
  
    Time(void){}
    
    ~Time(void){}

    /**
     * Starting the timer
     */
    void start(void) {
      gettimeofday(&_starttime,&tz);
    }

    /**
     * Stoping the timer
     */
    void stop(void) {
      gettimeofday(&_stoptime,&tz);
    }

    /**
     * Get the duration between start and stop in second.
     * @return double
     */
    double getTime(void){
      return _stoptime.tv_sec - _starttime.tv_sec + (_stoptime.tv_usec - _starttime.tv_usec)*1E-6;
    }
  };
}

#endif
