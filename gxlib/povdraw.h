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
  static olxstr to_str(const TVector3<NumT> &v)  {
    return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' << v[2] << '>';
  }
  template <typename NumT>
  static olxstr to_str(const TMatrix33<NumT> &m, const TVector3<NumT> &t)  {
    return olxstr(EmptyString(), 120) << '<'
      << m[0][0] << ',' << m[0][1] << ',' << m[0][2] << ',' 
      << m[1][0] << ',' << m[1][1] << ',' << m[1][2] << ',' 
      << m[2][0] << ',' << m[2][1] << ',' << m[2][2] << ',' 
      << t[0] << ',' << t[1] << ',' << t[2] << '>';
  }
  static olxstr to_str(const TGlOption &v, bool color=true)  {
    if( color )
      //return olxstr::FromCStr(olxT("rgb<"), 64) << v[0] << ',' << v[1] << ',' << v[2] << '>';
      return olxstr(EmptyString(), 64) << "rgb<" << v[0] << ',' << v[1] << ',' << v[2] << '>';
    else
      return olxstr(EmptyString(), 64) << '<' << v[0] << ',' << v[1] << ',' << v[2] << '>';
  }
  static olxstr get_mat_name(const olxstr &primitive_name, TGraphicsStyle &style,
    olxdict<const TGlMaterial*, olxstr, TPrimitiveComparator> &materials)
  {
    olxstr mat_name;
    size_t lmi = style.IndexOfMaterial(primitive_name);
    if( lmi != InvalidIndex )  {
      TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
      lmi = materials.IndexOf(&glm);
      if( lmi == InvalidIndex )
        mat_name = materials.Add(&glm, olxstr("mat") << (materials.Count()+1));
      else
        mat_name = materials.GetValue(lmi);
    }
    return mat_name;
  }
  static olxstr get_mat_name(const TGlMaterial& glm,
    olxdict<const TGlMaterial*, olxstr, TPrimitiveComparator> &materials)
  {
    size_t lmi = materials.IndexOf(&glm);
    return (lmi == InvalidIndex ? materials.Add(&glm, olxstr("mat") << (materials.Count()+1))
      : materials.GetValue(lmi));
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
