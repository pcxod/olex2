/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_math_plane_H
#define __olx_math_plane_H
#include "talist.h"

BeginEsdlNamespace()

namespace olx_plane {
  // sorts points in plane given by center and normal
  template <class array_t, class accessor_t, class vec_t>
  void Sort(array_t &points, const accessor_t &accessor,
    const vec_t &center, const vec_t &normal)
  {
    if( points.IsEmpty() )  return;
    TArrayList<double> pvs(points.Count());
    const vec_t origin = accessor(points[0])-center;
    pvs[0] = 0; // origin value
    for( size_t i=1; i < points.Count(); i++ )  {
      vec_t vec = accessor(points[i]) - center;
      double ca = origin.CAngle(vec);
      vec = origin.XProdVec(vec);
      // negative - vec is on the right, positive - on the left, if ca == 0, vec == (0,0,0)
      double vo = (ca == -1 ? 0 : vec.CAngle(normal));
      if( ca >= 0 )  { // -90 to 90
        if( vo < 0 )  // -90 to 0 3->4
          pvs[i] = 3.0 + ca;
        else  // 0 to 90 0->1
          pvs[i] = 1.0 - ca;
      }
      else if( ca > -1 ) {  // 90-270
        if( vo < 0 )  // 180 to 270 2->3
          pvs[i] = 3.0 + ca;
        else  // 90 to 180 1->2
          pvs[i] = 1.0 - ca;
      }
      else  {  //-1, special case
        pvs[i] = 2;
      }
    }
    pvs.QuickSorter.Sort(pvs, TPrimitiveComparator(),
      SyncSwapListener<array_t>(points));
  }
};  //end namespace olx_plane

EndEsdlNamespace()
#endif

