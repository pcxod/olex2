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
#include "gloption.h"
#include "ebasis.h"
#include "edict.h"
BeginGlNamespace()

class TGraphicsStyle;
class TGlMaterial;
class AGDrawObject;

struct pov {
  template <typename NumT>
  static olxstr to_str(const TVector3<NumT> &v)  {
    return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' <<
      v[2] << '>';
  }
  template <typename NumT>
  static olxstr to_str(const TMatrix33<NumT> &m,
    const TVector3<NumT> &t)
  {
    return olxstr(EmptyString(), 120) << '<'
      << m[0][0] << ',' << m[0][1] << ',' << m[0][2] << ',' 
      << m[1][0] << ',' << m[1][1] << ',' << m[1][2] << ',' 
      << m[2][0] << ',' << m[2][1] << ',' << m[2][2] << ',' 
      << t[0] << ',' << t[1] << ',' << t[2] << '>';
  }
  static olxstr to_str(const TGlOption &v, bool color=true)  {
    if( color ) {
      return olxstr(EmptyString(), 64) << "rgb<" << v[0] << ',' << v[1] <<
        ',' << v[2] << '>';
    }
    else {
      return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' <<
        v[2] << '>';
    }
  }
  static olxstr get_mat_name(const olxstr &primitive_name,
    TGraphicsStyle &style, olxdict<TGlMaterial, olxstr,
    TComparableComparator> &materials,
    const AGDrawObject *sender=NULL);

  static olxstr get_mat_name(const TGlMaterial& glm,
    olxdict<TGlMaterial, olxstr, TComparableComparator> &materials,
    const AGDrawObject *sender=NULL);

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

EndGlNamespace()
#endif
