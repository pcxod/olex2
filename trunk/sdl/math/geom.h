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
public:
  BoxVolumeValidator(const vec3d_alist &n, const vec3d_alist &c)
    : norms(n), centres(c)
  {
    if (norms.Count() != 6 || norms.Count() != centres.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "volume definition");
    }
  }
  virtual bool IsInside(const vec3d &c) const {
    for (int fi = 0; fi < 6; fi++) {
      if ((c - centres[fi]).DotProd(norms[fi]) > 0) {
        return false;
      }
    }
    return true;
  }
};

class SphereVolumeValidator : public IVolumeValidator {
  vec3d centre;
  double qr;
public:
  SphereVolumeValidator(const vec3d &c, double r)
    : centre(c), qr(r*r)
  {}
  virtual bool IsInside(const vec3d &c) const {
    return (c - centre).QLength() < qr;
  }
};

EndEsdlNamespace();
#endif
