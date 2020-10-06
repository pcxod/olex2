/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_math_geom_H
#define __olx_sdl_math_geom_H
#include "../threex3.h"

BeginEsdlNamespace()

class IVolumeValidator {
public:
  virtual ~IVolumeValidator() {}
  virtual bool IsInside(const vec3d &) const = 0;
};

class BoxVolumeValidator : public IVolumeValidator {
  vec3d_alist norms;
  vec3d_alist centres;
  bool inclusive;
public:
  BoxVolumeValidator(const vec3d_alist &n, const vec3d_alist &c,
    bool inclusive=false)
    : norms(n), centres(c),
    inclusive(inclusive)
  {
    if (norms.Count() != 6 || norms.Count() != centres.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "volume definition");
    }
  }
  virtual bool IsInside(const vec3d &c) const {
    for (int fi = 0; fi < 6; fi++) {
      double t = (c - centres[fi]).DotProd(norms[fi]);
      if (inclusive) {
        if (t >= 0) {
          return false;
        }
      }
      else if (t > 0) {
        return false;
      }
    }
    return true;
  }
};

class BoxValidator : public IVolumeValidator {
  vec3d from, to;
  bool from_inclusive, to_inclusive;
public:
  BoxValidator(const vec3d& from, const vec3d& to,
    bool from_inclusive = false, bool to_inclusive = false)
    : from(from), to(to),
    from_inclusive(from_inclusive), to_inclusive(to_inclusive)
  {}
  virtual bool IsInside(const vec3d& c) const {
    for (int i = 0; i < 3; i++) {
      if (from_inclusive) {
        if (to_inclusive) {
          if (c[i] < from[i] || c[i] > to[i]) {
            return false;
          }
        }
        else {
          if (c[i] < from[i] || c[i] >= to[i]) {
            return false;
          }
        }
      }
      else {
        if (to_inclusive) {
          if (c[i] <= from[i] || c[i] > to[i]) {
            return false;
          }
        }
        else {
          if (c[i] <= from[i] || c[i] >= to[i]) {
            return false;
          }
        }
      }
    }
    return true;
  }
};

class SphereVolumeValidator : public IVolumeValidator {
  vec3d centre;
  double qr;
  bool inclusive;
public:
  SphereVolumeValidator(const vec3d &c, double r, bool inclusive=false)
    : centre(c), qr(r*r),
    inclusive(inclusive)
  {}
  virtual bool IsInside(const vec3d &c) const {
    double t = (c - centre).QLength();
    if (inclusive) {
      return t <= qr;
    }
    return  t < qr;
  }
};

EndEsdlNamespace();
#endif
