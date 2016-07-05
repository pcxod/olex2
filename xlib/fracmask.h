/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_fractional_mask_H
#define __olx_xl_fractional_mask_H
#include "xbase.h"
#include "arrays.h"
#include "threex3.h"
#include "dataitem.h"

class FractMask {
  TArray3D<bool>* Mask;
  vec3d Norm;
public:
  FractMask() : Mask(NULL)  {  }
  ~FractMask()  {
    if( Mask != NULL )
      delete Mask;
  }
  /* min and max - fractional coordinates, norm - length of the sides,
  resolution - the mask resolution in anstrems */
  void Init(const vec3d& _min, const vec3d& _max, const vec3d& norms,
    double resolution=1.0);
  // takes fractional coordinates
  inline void Set(const vec3d& fc, bool v)  {
    const vec3d ind = fc * Norm;
    if( Mask->IsInRange(ind) )
      Mask->Value(ind) = v;
  }
  // takes fractional coordinates
  template <class vec> inline bool Get(const vec& fc) const {
    const vec3d ind = fc*Norm;
    return Mask->IsInRange(ind) ? Mask->Value(ind) : false;
  }
  inline TArray3D<bool>* GetMask() const {  return Mask;  }

  void ToDataItem(TDataItem& di, IOutputStream& os) const;
  void FromDataItem(const TDataItem& di, IInputStream& is);
};


#endif
