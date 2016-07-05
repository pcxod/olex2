/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_wrldraw_H
#define __olx_gxl_wrldraw_H
#include "gloption.h"
#include "ebasis.h"
#include "edict.h"
#include "emath.h"

BeginGlNamespace()

class TGraphicsStyle;
class TGlMaterial;
class AGDrawObject;

struct wrl {
  template <typename NumT>
  static olxstr to_str(const TVector3<NumT> &v) {
    return olxstr(EmptyString(), 64).stream(' ') << v[0] << v[1] << v[2];
  }
  static olxstr to_str(const TGlOption &v) {
    return olxstr(EmptyString(), 64).stream(' ') << v[0] << v[1] << v[2];
  }

  static bool check_r_matrix(const mat3d &m, const vec3d &v, double ca,
    double sa,
    const int *s)
  {
    const double t = 1-ca;
    if (olx_abs(m[0][1] - (t*v[0]*v[1]*s[0]*s[1] + sa*v[2]*s[2])) > 1e-3 ||
        olx_abs(m[0][2] - (t*v[0]*v[2]*s[0]*s[2] - sa*v[1]*s[1])) > 1e-3 ||
        olx_abs(m[1][0] - (t*v[0]*v[1]*s[0]*s[1] - sa*v[2]*s[2])) > 1e-3 ||
        olx_abs(m[1][2] - (t*v[1]*v[2]*s[1]*s[2] + sa*v[0]*s[0])) > 1e-3 ||
        olx_abs(m[2][0] - (t*v[0]*v[2]*s[0]*s[2] + sa*v[1]*s[1])) > 1e-3 ||
        olx_abs(m[2][1] - (t*v[1]*v[2]*s[1]*s[2] - sa*v[0]*s[0])) > 1e-3)
      return false;
    return true;
  }
  /* Decomposes a rotation matrix into the rotation vector (axis) and the angle
  */
  static double decompose(const mat3d &m, vec3d &rv) {
    double ca = (m.Trace()-1)/2,
      sa = (olx_abs(ca) > 1e-15) ? sqrt(1-ca*ca) : 1,
      C = 1-ca;
    if (olx_abs(C) < 1e-3) {
      rv[0] = m[2][1];
      rv[1] = m[0][2];
      rv[2] = m[1][0];
      return 0;
    }
    else { // do it straightforward way, no fancy decompositions
      rv[0] = (m[0][0]-ca)/C;
      rv[1] = (m[1][1]-ca)/C;
      rv[2] = (m[2][2]-ca)/C;
      rv.Abs().Sqrt();
      static int p[8][3] = {
        {1,1,1}, {1,1,-1}, {1,-1,1}, {1,-1,-1},
        {-1,1,1}, {-1,1,-1}, {-1,-1,1}, {-1,-1,-1}
      };
      int perm=-1;
      for (int i=0; i < 8; i++) {
        if (check_r_matrix(m, rv, ca, sa, &p[i][0])) {
          perm = i;
          break;
        }
      }
      if (perm == -1) {
        throw TFunctionFailedException(__OlxSourceInfo, "assert");
      }
      rv[0] *= p[perm][0];
      rv[1] *= p[perm][1];
      rv[2] *= p[perm][2];
      return acos(ca);
    }
  }

  static olxstr get_mat_str(const olxstr &primitive_name,
    TGraphicsStyle &style, olx_cdict<TGlMaterial, olxstr> &materials,
    const AGDrawObject *sender=NULL);

  static olxstr get_mat_str(const TGlMaterial& glm,
    olx_cdict<TGlMaterial, olxstr> &materials,
    const AGDrawObject *sender=NULL);

  struct CrdTransformer {
    TEBasis basis;
    double scale;
    CrdTransformer(double _scale=1.0)
      : scale (_scale)
    {}
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
