/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_option_H
#define __olx_gl_option_H
#include "glbase.h"
#include "emath.h"
#ifdef __WXWIDGETS__
#include <wx/colour.h>
#endif
BeginGlNamespace()

class TGlOption : public IOlxObject {
  float data[4];
public:
  TGlOption() {
    data[0] = data[1] = data[2] = 0;
    data[3] = 1;
  }
  template <class ft> TGlOption(ft r, ft g, ft b, ft a = 1.0) {
    data[0] = (float)r;
    data[1] = (float)g;
    data[2] = (float)b;
    data[3] = (float)a;
  }
  TGlOption(const TGlOption& o) {
    *this = o;
  }
  TGlOption(uint32_t rgba) {
    *this = rgba;
  }

#ifdef __WXWIDGETS__
  TGlOption(const wxColour &cl) {
    *this = cl.GetRGBA();
  }

  operator wxColor () const {
    return wx();
  }

  TGlOption& operator = (const wxColor &cl) {
    this->operator = (cl.GetRGBA());
    return *this;
  }
#endif

  TGlOption& operator = (const TGlOption& o) {
    memcpy(&data[0], &o.data[0], 4 * sizeof(data[0]));
    return *this;
  }

  TGlOption& operator = (uint32_t c) {
    data[0] = (float)OLX_GetRValue(c) / 255;
    data[1] = (float)OLX_GetGValue(c) / 255;
    data[2] = (float)OLX_GetBValue(c) / 255;
    data[3] = (float)OLX_GetAValue(c) / 255;
    return *this;
  }

  template <class ft> TGlOption& operator = (const ft* c) {
    data[0] = (float)c[0];
    data[1] = (float)c[1];
    data[2] = (float)c[2];
    data[3] = (float)c[3];
    return *this;
  }

  // use the FromString instead if error handling is needed
  TGlOption& operator = (const olxstr& v) {
    FromString(v);
    return *this;
  }

  template <class ft> TGlOption& operator *= (ft v) {
    data[0] = (float)(data[0] * v);
    data[1] = (float)(data[1] * v);
    data[2] = (float)(data[2] * v);
    data[3] = (float)(data[3] * v);
    return *this;
  }

  template <class ft> TGlOption operator * (ft v) const {
    return TGlOption(data[0] * v, data[1] * v, data[2] * v, data[3] * v);
  }

  TGlOption operator + (const TGlOption& v) const {
    return TGlOption(data[0] + v[0],
      data[1] + v[1],
      data[2] + v[2],
      data[3] + v[3]);
  }

  TGlOption& operator -= (const TGlOption& v) {
    data[0] -= v[0];
    data[1] -= v[1];
    data[2] -= v[2];
    data[3] -= v[3];
    return *this;
  }

  TGlOption& operator += (const TGlOption& v) {
    data[0] += v[0];
    data[1] += v[1];
    data[2] += v[2];
    data[3] += v[3];
    return *this;
  }

  uint32_t GetRGB() const {
    return (uint32_t)OLX_RGBA(
      255 * data[0],
      255 * data[1],
      255 * data[2],
      255 * data[3]);
  }

  const float* Data() const { return data; }

  float* _Data() { return data; }

  bool IsBlack() const {
    return (data[0] == 0 && data[1] == 0 && data[2] == 0);
  }
  
  void Clear() {
    data[0] = data[1] = data[2] = data[3] = 0;
  }

  float& operator[] (size_t i) { return data[i]; }
  const float& operator[] (size_t i) const { return data[i]; }

  bool operator == (const TGlOption& v) const {
    return EqualsRGBA(v);
  }

  bool operator != (const TGlOption& v) const {
    return !EqualsRGBA(v);
  }

  bool EqualsRGBA(const TGlOption& v, float eps=3.0e-3f) const {
    return EqualsRGB(v) && olx_abs(data[3] - v[3]) < eps;
  }

  bool EqualsRGB(const TGlOption& v, float eps=3.0e-3f) const {
    return
      olx_abs(data[0] - v[0]) < eps &&
      olx_abs(data[1] - v[1]) < eps &&
      olx_abs(data[2] - v[2]) < eps;
  }

  int Compare(const TGlOption &o) const {
    for (int i = 0; i < 4; i++) {
      int v = olx_cmp_float(data[i], o.data[i], 3.0e-3f);
      if (v != 0) {
        return v;
      }
    }
    return 0;
  }

  bool IsGrey() const {
    return olx_abs(data[0] - data[1]) < 3e-3f &&
      olx_abs(data[0] - data[2]) < 3e-3f;
  }

  float GetMean() const { return (data[0] + data[1] + data[2]) / 3.0f; }

  // scales the values to max of 1
  TGlOption &Scale() {
    float mx = olx_max(data[0], data[1]);
    mx = olx_max(data[2], mx);
    mx = olx_max(data[3], mx);
    if (mx != 0) {
      data[0] /= mx; data[1] /= mx; data[2] /= mx; data[3] /= mx;
    }
    return *this;
  }

  TIString ToString() const;
  bool FromString(const olxstr& S);

#ifdef __WXWIDGETS__
  wxColour wx() const {
    return wxColour(GetRGB());
  }
#endif

};

EndGlNamespace()
#endif
