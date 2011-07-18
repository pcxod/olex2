/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_powdraw_H
#define __olx_gxl_powdraw_H
#include "gxbase.h"
#include "gloption.h"
#include "ebasis.h"
BeginGxlNamespace()

namespace pov {
  template <typename NumT>
  olxstr to_str(const TVector3<NumT> &v)  {
    return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' << v[2] << '>';
  }
  olxstr to_str(const TGlOption &v, bool color=true)  {
    if( color )
      //return olxstr::FromCStr(olxT("rgb<"), 64) << v[0] << ',' << v[1] << ',' << v[2] << '>';
      return olxstr(EmptyString(), 64) << "rgb<" << v[0] << ',' << v[1] << ',' << v[2] << '>';
    else
      return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' << v[2] << '>';
  }
  struct CrdTransformer  {
    TEBasis basis;
    double scale;
    CrdTransformer(const TEBasis &_basis, double _scale=1.0)
      : basis(_basis), scale(_scale) {}
    vec3d crd(const vec3d &v) const {
      return (v + basis.GetCenter())*basis.GetMatrix();
    }
    mat3d matr(const mat3d &m) const {
      return m*basis.GetMatrix();
    }
    vec3d normal(const vec3d &v) const {
      return v*basis.GetMatrix();
    }
  };
};

EndGxlNamespace()
#endif
