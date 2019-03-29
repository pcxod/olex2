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
BeginEsdlNamespace()

template <class T> class TEComplex  {
  T _A, _B;
public:
  TEComplex()  {  _A = _B = 0;  }

  TEComplex(T _real)  {
    _A = _real;
    _B = 0;
  }
  template <class AT> TEComplex(AT _real)  {
      _A = (T)_real;
      _B = 0;
    }

  TEComplex(T _real, T _compl)  {
    _A = _real;
    _B = _compl;
  }
  template <class AT> TEComplex(AT _real, AT _compl)  {
      _A = (T)_real;
      _B = (T)_compl;
    }

  TEComplex(const TEComplex& cmpl)  {
    _A = cmpl._A;
    _B = cmpl._B;
  }
  template <class AT> TEComplex(const TEComplex<AT>& cmpl)  {
      _A = (T)cmpl._A;
      _B = (T)cmpl._B;
    }

  inline T& A()  {  return _A;  }
  inline T& B()  {  return _B;  }
  inline T& Re()  {  return _A;  }
  inline T& Im()  {  return _B;  }
  inline const T& GetA() const {  return _A;  }
  inline const T& GetB() const {  return _B;  }
  inline const T& GetRe() const {  return _A;  }
  inline const T& GetIm() const {  return _B;  }
  inline void SetA(const T& v)  {  _A = v;  }
  inline void SetB(const T& v)  {  _B = v;  }
  inline void SetRe(const T& v)  {  _A = v;  }
  inline void SetIm(const T& v)  {  _B = v;  }
  inline void Null()  { _A = _B = 0;  }
  inline T abs() const {
    T mx = olx_max(_A,_B);
    if( mx == 0 )  return 0;
    T a = _A/mx,
      b = _B/mx;
    return mx*sqrt(a*a + b*b);
  }

  inline T mod() const {
    if( _B == 0 )  return (_A < 0) ? -_A : _A;
    return sqrt( _A*_A + _B*_B );
  }

  inline T qmod() const {  return _A*_A + _B*_B;  }

  inline TEComplex inv() const {
    TEComplex rv(*this);
    rv._B *= -1;
    rv /= qmod();
    return rv;
  }

  inline TEComplex conj() const {  return TEComplex(_A, -_B);  }

  inline T arg() const {  return atan2(_B, _A);  }
  inline TEComplex exp() const {  return TEComplex<T>( ::exp(_A), _B );  }
  inline TEComplex log() const {  return TEComplex<T>( this->abs(), this->arg());  }

  inline TEComplex pow(const TEComplex& pv) const {
    return (pv * this->log()).exp();
  }

  static TEComplex polar( T r, T theta )  {
    return TEComplex<T>(r*cos(theta), r*sin(theta));
  }

  inline const TEComplex& operator = (const TEComplex& _compl)  {
    _A = _compl._A;
    _B = _compl._B;
    return _compl;
  }
  template <class AT>
    inline const TEComplex& operator = (const TEComplex<AT>& _compl)  {
      _A = (T)_compl._A;
      _B = (T)_compl._B;
      return _compl;
    }

  inline void operator += (const TEComplex& _compl)  {
    _A += _compl._A;
    _B += _compl._B;
  }
  inline void operator -= (const TEComplex& _compl)  {
    _A -= _compl._A;
    _B -= _compl._B;
  }
  inline void operator *= (const TEComplex& _compl)  {
    const T v = _A*_compl._A - _B*_compl._B;
    _B = _B*_compl._A + _A*_compl._B;
    _A = v;
  }
  inline void operator /= (const TEComplex& _compl)  {
    T dv = _compl._A*_compl._A + _compl._B*_compl._B,
      oa = _A,
      ob = _B;
    _A = (oa*_compl._A + ob*_compl._B)/dv;
    _B = (ob*_compl._A - oa*_compl._B)/dv;
  }

  inline void operator += (const T a)  {  _A += a;  }
  inline void operator -= (const T a)  {  _A -= a;  }
  inline void operator *= (const T a)  {  _A *= a;  _B *= a;  }
  inline void operator /= (const T a)  {  _A /= a;  _A /= a;  }

  inline TEComplex operator + (const TEComplex& _compl) const {
    return TEComplex<T>(_A+_compl._A, _B+_compl._B);
  }
  inline TEComplex operator - (const TEComplex& _compl) const {
    return TEComplex<T>(_A-_compl._A, _B-_compl._B);
  }
  inline TEComplex operator * (const TEComplex& _compl) const {
    return TEComplex<T>(_A*_compl._A - _B*_compl._B ,_B*_compl._A + _A*_compl._B);
  }
  inline TEComplex operator / (const TEComplex& _compl) const {
    T dv = _compl._A*_compl._A + _compl._B*_compl._B;
    return TEComplex((_A*_compl._A + _B*_compl._B)/dv, (_B*_compl._A - _A*_compl._B)/dv);
  }

  inline TEComplex operator + (const T a) const {  return TEComplex<T>(_A+a, _B);  }
  inline TEComplex operator - (const T a) const {  return TEComplex<T>(_A-a, _B);  }
  inline TEComplex operator * (const T a) const {  return TEComplex<T>(_A*a, _B*a);  }
  inline TEComplex operator / (const T a) const {  return TEComplex<T>(_A/a, _B/a);  }
  inline bool operator == (const TEComplex& _compl) const {
    return (_A == _compl._A && _B == _compl._B);
  }
  inline bool operator != (const TEComplex& _compl) const {
    return (_A != _compl._A || _B != _compl._B);
  }

};

typedef TEComplex<double> compd;
typedef TEComplex<float> compf;

EndEsdlNamespace()
#endif
