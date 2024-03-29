/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_complex_h
#define __olx_sdl_complex_h
#include "ebase.h"
#include <complex>
BeginEsdlNamespace()

template <class T> class TEComplex {
  T _A, _B;
public:
  TEComplex() { _A = _B = 0; }

  TEComplex(T _real) {
    _A = _real;
    _B = 0;
  }
  template <class AT> TEComplex(AT _real) {
    _A = (T)_real;
    _B = 0;
  }

  TEComplex(T _real, T _compl) {
    _A = _real;
    _B = _compl;
  }
  template <class AT> TEComplex(AT _real, AT _compl) {
    _A = (T)_real;
    _B = (T)_compl;
  }

  TEComplex(const TEComplex& cmpl) {
    _A = cmpl._A;
    _B = cmpl._B;
  }
  template <class AT> TEComplex(const TEComplex<AT>& cmpl) {
    _A = (T)cmpl._A;
    _B = (T)cmpl._B;
  }

  inline T& A() { return _A; }
  inline T& B() { return _B; }
  inline T& Re() { return _A; }
  inline T& Im() { return _B; }
  inline const T& GetA() const { return _A; }
  inline const T& GetB() const { return _B; }
  inline const T& GetRe() const { return _A; }
  inline const T& GetIm() const { return _B; }
  inline void SetA(const T& v) { _A = v; }
  inline void SetB(const T& v) { _B = v; }
  inline void SetRe(const T& v) { _A = v; }
  inline void SetIm(const T& v) { _B = v; }
  inline void Null() { _A = _B = 0; }
  inline T abs() const {
    T mx = olx_max(_A, _B);
    if (mx == 0)  return 0;
    T a = _A / mx,
      b = _B / mx;
    return mx * sqrt(a * a + b * b);
  }

  inline T mod() const {
    if (_B == 0)  return (_A < 0) ? -_A : _A;
    return sqrt(_A * _A + _B * _B);
  }

  inline T qmod() const { return _A * _A + _B * _B; }

  inline TEComplex inv() const {
    TEComplex rv(*this);
    rv._B *= -1;
    rv /= qmod();
    return rv;
  }

  inline TEComplex conj() const { return TEComplex(_A, -_B); }

  inline T arg() const { return atan2(_B, _A); }
  inline TEComplex exp() const { return TEComplex<T>(::exp(_A), _B); }
  inline TEComplex log() const { return TEComplex<T>(this->abs(), this->arg()); }

  inline TEComplex pow(const TEComplex& pv) const {
    return (pv * this->log()).exp();
  }

  static TEComplex polar(T r, T theta) {
    return TEComplex<T>(r * cos(theta), r * sin(theta));
  }

  static TEComplex polar(T theta) {
    return TEComplex<T>(cos(theta), sin(theta));
  }

  inline TEComplex& operator = (const TEComplex& _compl) {
    _A = _compl._A;
    _B = _compl._B;
    return *this;
  }
  template <class AT>
  inline TEComplex& operator = (const TEComplex<AT>& _compl) {
    _A = (T)_compl._A;
    _B = (T)_compl._B;
    return *this;
  }

  inline void operator += (const TEComplex& _compl) {
    _A += _compl._A;
    _B += _compl._B;
  }
  inline void operator -= (const TEComplex& _compl) {
    _A -= _compl._A;
    _B -= _compl._B;
  }
  inline void operator *= (const TEComplex& _compl) {
    const T v = _A * _compl._A - _B * _compl._B;
    _B = _B * _compl._A + _A * _compl._B;
    _A = v;
  }
  inline void operator /= (const TEComplex& _compl) {
    T dv = _compl._A * _compl._A + _compl._B * _compl._B,
      oa = _A,
      ob = _B;
    _A = (oa * _compl._A + ob * _compl._B) / dv;
    _B = (ob * _compl._A - oa * _compl._B) / dv;
  }

  inline void operator += (const T a) { _A += a; }
  inline void operator -= (const T a) { _A -= a; }
  inline void operator *= (const T a) { _A *= a;  _B *= a; }
  inline void operator /= (const T a) { _A /= a;  _A /= a; }

  inline TEComplex operator + (const TEComplex& _compl) const {
    return TEComplex<T>(_A + _compl._A, _B + _compl._B);
  }
  inline TEComplex operator - (const TEComplex& _compl) const {
    return TEComplex<T>(_A - _compl._A, _B - _compl._B);
  }
  inline TEComplex operator - () const {
    return TEComplex<T>(-_A, -_B);
  }
  inline TEComplex operator * (const TEComplex& _compl) const {
    return TEComplex<T>(_A * _compl._A - _B * _compl._B, _B * _compl._A + _A * _compl._B);
  }
  inline TEComplex operator / (const TEComplex& _compl) const {
    T dv = _compl._A * _compl._A + _compl._B * _compl._B;
    return TEComplex((_A * _compl._A + _B * _compl._B) / dv, (_B * _compl._A - _A * _compl._B) / dv);
  }

  inline TEComplex operator + (const T a) const { return TEComplex<T>(_A + a, _B); }
  inline TEComplex operator - (const T a) const { return TEComplex<T>(_A - a, _B); }
  inline TEComplex operator * (const T a) const { return TEComplex<T>(_A * a, _B * a); }
  inline TEComplex operator / (const T a) const { return TEComplex<T>(_A / a, _B / a); }
  inline bool operator == (const TEComplex& _compl) const {
    return (_A == _compl._A && _B == _compl._B);
  }
  inline bool operator != (const TEComplex& _compl) const {
    return (_A != _compl._A || _B != _compl._B);
  }

};

template <typename T> T olx_abs(const TEComplex<T> &n) {
  return n.abs();
}

template <typename T> T olx_abs(const std::complex<T>& n) {
  return std::abs(n);
}

template <typename T1, typename T2>
TEComplex<T2> operator / (T1 n, const TEComplex<T2>& c) {
  T2 dv = c.qmod();
  return TEComplex<T2>((n * c.GetRe()) / dv, (- n * c.GetIm()) / dv);
}

template <typename T> TEComplex<T> olx_exp(const TEComplex<T>& n) {
  T v = exp(n.GetRe());
  return TEComplex<T>(v * cos(n.GetIm()), v*sin(n.GetIm()));
}

template <typename T> T olx_sqr(const TEComplex<T>& n) {
  return n.qmod();
}

template <typename T> T olx_exp(const T& n) {
  return std::exp(n);
}

typedef TEComplex<double> compd;
typedef TEComplex<float> compf;

template <typename T>
static const char* olx_format_modifier(const TEComplex<T>& n) {
  static olxcstr fmt = olxcstr() << olx_format_modifier(n.GetRe())
    << "e%ci%.4";
  return fmt.c_str();
}

/* if the fmt is used - it should take re, char sign of the im and im as
* arguments. Padded to 20 chars, default "%12.4le%ci%.4lf"
*/
template <typename T>
olxstr strof(const TEComplex<T> &n, const char* fmt = 0) {
  if (fmt == 0) {
    return olx_print((olxcstr("%12.4") <<
      olx_format_modifier(n.GetRe()) << "e%ci%.4" << olx_format_modifier(n.GetIm()) << 'f').c_str(),
      n.GetRe(), (char)olx_sign_char(n.GetIm()), olx_abs(n.GetIm()))
      .RightPadding(20, ' ');
  }
  return olx_print(fmt,
    n.GetRe(), (char)olx_sign_char(n.GetIm()), olx_abs(n.GetIm()))
    .RightPadding(20, ' ');
}

template <typename T>
const T& olx_get_primitive_type(const TEComplex<T>& n) {
  return n.GetRe();
}


EndEsdlNamespace()
#endif
