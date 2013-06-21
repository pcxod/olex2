/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_wrldraw_H
#define __olx_gxl_pwrldraw_H
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
  static olxstr to_str(const TVector3<NumT> &v)  {
    return olxstr(EmptyString(), 64).stream(' ') << v[0] << v[1] << v[2];
  }
  template <typename NumT>
  static olxstr to_str(const TMatrix33<NumT> &m,
    const TVector3<NumT> &t)
  {
    return olxstr("matrix", 120).stream(' ')
      << m[0][0] << m[0][1] << m[0][2] << '0'
      << m[1][0] << m[1][1] << m[1][2] << '0'
      << m[2][0] << m[2][1] << m[2][2] << '0'
      << t[0] << t[1] << t[2] << '1';
  }
  static olxstr to_str(const TGlOption &v) {
    return olxstr(EmptyString(), 64).stream(' ') << v[0] << v[1] << v[2];
  }
  /* Decomposes a rotation matrix into the rotation vector (axis) and the angle
  */
  static double decompose(const mat3d &m, vec3d &rv) {
    double ca = (m.Trace()-1)/2,
      C = 1-ca;
    if (olx_abs(C) < 1e-3) {
      rv[0] = m[2][1];
      rv[1] = m[0][2];
      rv[2] = m[1][0];
      return 0;
    }
    else { // do it stupid way
      rv[0] = (m[0][0]-ca)/C;
      rv[1] = (m[1][1]-ca)/C;
      rv[2] = (m[2][2]-ca)/C;
      rv.Abs().Sqrt();
      static int p[8][3] = {
        {1,1,1},
        {1,1,-1},
        {1,-1,1},
        {1,-1,-1},
        {-1,1,1},
        {-1,1,-1},
        {-1,-1,1},
        {-1,-1,-1}
      };
      mat3d rm;
      int perm=0;
      for (int i=0; i < 8; i++) {
        olx_create_rotation_matrix(rm,
          vec3d(p[i][0]*rv[0], p[i][1]*rv[1], p[i][2]*rv[2]), ca);
        double dev=0;
        for (int x=0; x < 3; x++) {
          for (int y=0; y < 3; y++)
            dev += olx_abs(m[x][y]-rm[x][y]);
        }
        if (dev < 1e-3) {
          perm = i;
          break;
        }
      }
      rv[0] *= p[perm][0];
      rv[1] *= p[perm][1];
      rv[2] *= p[perm][2];
      return acos(ca);
    }
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

EndGlNamespace()
#endif
