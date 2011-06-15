#ifndef __olx_gl_option_H
#define __olx_gl_option_H

#include "glbase.h"
#include "emath.h"

BeginGlNamespace()

class TGlOption: public IEObject  {
  float data[4];
public:
  TGlOption()  {
    data[0] = data[1] = data[2] = 0;
    data[3] = 1;
  }
  template <class ft> TGlOption(ft r, ft g, ft b, ft a=1.0)  {  
    data[0] = (float)r;  
    data[1] = (float)g;  
    data[2] = (float)b;  
    data[3] = (float)a;  
  }
  TGlOption(const TGlOption& o)  {  *this = o;  }
  TGlOption(uint32_t rgba)  {  *this = rgba;  }

  TGlOption& operator = (const TGlOption& o)  {
    data[0] = o.data[0];  
    data[1] = o.data[1];
    data[2] = o.data[2];
    data[3] = o.data[3];
    return *this;
  }

  TGlOption& operator = (uint32_t c)  {
    data[0] = (float)GetRValue(c)/255;
    data[1] = (float)GetGValue(c)/255;
    data[2] = (float)GetBValue(c)/255;
    data[3] = (float)GetAValue(c)/255;
    return *this;
  }

  template <class ft> TGlOption& operator = (const ft* c)  {
    data[0] = (float)c[0];
    data[1] = (float)c[1];
    data[2] = (float)c[2];
    data[3] = (float)c[3];
    return *this;
  }
  // us the FromString instead if error handling is needed
  TGlOption& operator = (const olxstr& v)  {
    FromString(v);
    return *this;
  }

  template <class ft> TGlOption& operator *= (ft v)  {
    data[0] = (float)(data[0]*v);
    data[1] = (float)(data[1]*v);
    data[2] = (float)(data[2]*v);
    data[3] = (float)(data[3]*v);
    return *this;
  }
  template <class ft> TGlOption operator * (ft v) const {
    return TGlOption(data[0]*v, data[1]*v, data[2]*v, data[3]*v); 
  }
  TGlOption operator + (const TGlOption& v) const {
    return TGlOption(data[0]+v[0], data[1]+v[1], data[2]+v[2], data[3]+v[3]);
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

  uint32_t GetRGB() const {  return (uint32_t)RGBA(255*data[0], 255*data[1], 255*data[2], 255*data[3]);  }
  inline const float* Data() const {  return data;  }
  float* _Data()  {  return data;  }

  bool IsEmpty() const {  return (data[0] == 0 && data[1] == 0 && data[2] == 0);  }
  void Clear()  {  data[0] = data[1] = data[2] = data[3] = 0;  }
  
  float& operator[] (size_t i) {  return data[i]; }
  const float& operator[] (size_t i) const {  return data[i]; }
  
  bool operator == (const TGlOption& v) const {
    return !(olx_abs(data[0]-v[0]) > 1e-5 || olx_abs(data[1]-v[1]) > 1e-5 ||
             olx_abs(data[2]-v[2]) > 1e-5 || olx_abs(data[3]-v[3]) > 1e-5);
  }
  bool operator != (const TGlOption& v) const {  return !(*this == v);  }
  bool IsGrey() const {
    return olx_abs(data[0]-data[1]) < 1e-5 && olx_abs(data[0]-data[2]);
  }
  float GetMean() const {  return (data[0]+data[1]+data[2])/3.0f;  }
  TIString ToString() const;
  bool FromString(const olxstr& S);
};

EndGlNamespace()
#endif

