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
    double max_qd = 0;
    size_t m_i = 0;
    for (size_t i=0; i < points.Count(); i++) {
      double qd = (accessor(points[i])-center).QLength();
      if (qd > max_qd) {
        m_i = i;
        max_qd = qd;
      }
    }
    const vec_t origin = accessor(points[m_i])-center;
    pvs[m_i] = 0; // origin value
    for( size_t i=0; i < points.Count(); i++ )  {
      if (i==m_i) continue;
      vec_t vec = accessor(points[i]) - center;
      if (vec.QLength() < 1e-6) // atom on the center!
        continue;
      double ca = origin.CAngle(vec);
      vec = origin.XProdVec(vec);
      double vo;
      if (vec.IsNull(1e-6)) { // same direction as the origin
        vo = 0;
      }
      else {
        vo = (ca == -1 ? 0 : vec.CAngle(normal));
      }
      /* negative - vec is on the right, positive - on the left, if ca == 0,
      vec == (0,0,0)
      */
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
    QuickSorter::Sort(pvs, TPrimitiveComparator(),
      SyncSwapListener::Make(points));
  }
};  //end namespace olx_plane

EndEsdlNamespace()
#endif

