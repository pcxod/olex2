/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_linked_operators
#define __olx_sdl_linked_operators
#include "ebase.h"
BeginEsdlNamespace()

template<typename h_t, class heir_t, typename a_t>
class linked_operators {
public:
  h_t GetValue() const {  return static_cast<const heir_t*>(this)->GetValue();  }
  heir_t &SetValue(h_t v) {
    heir_t *h = static_cast<heir_t*>(this);
    h->SetValue(v);
    return *h;
  }

  operator h_t () const {  return GetValue();  }

  heir_t& operator = (h_t v)  {  return SetValue(v);  }
  heir_t& operator = (a_t v)  {  return SetValue(v);  }
  heir_t& operator = (const heir_t &v)  {  return SetValue(v.GetValue());  }
  template <class aheir_t>
  heir_t& operator = (const linked_operators<a_t, aheir_t, h_t> &v)  {
    return SetValue(v.GetValue());
  }
  
  bool operator == (a_t v) const {  return v == GetValue();  }
  bool operator != (a_t v) const {  return v != GetValue();  }
  bool operator > (a_t v) const {  return v > GetValue();  }
  bool operator >= (a_t v) const {  return v >= GetValue();  }
  bool operator < (a_t v) const {  return v < GetValue();  }
  bool operator <= (a_t v) const {  return v <= GetValue();  }
  h_t operator + (a_t v) const {  return h_t(GetValue()+v);  }
  h_t operator - (a_t v) const {  return h_t(GetValue()-v);  }
  h_t operator / (a_t v) const {  return h_t(GetValue()/v);  }
  h_t operator * (a_t v) const {  return h_t(GetValue()*v);  }
  heir_t &operator += (a_t v) {  return SetValue(GetValue()+v);  }
  heir_t &operator -= (a_t v) {  return SetValue(GetValue()-v);  }
  heir_t &operator *= (a_t v) {  return SetValue(GetValue()*v);  }
  heir_t &operator /= (a_t v) {  return SetValue(GetValue()/v);  }

  bool operator == (h_t v) const {  return v == GetValue();  }
  bool operator != (h_t v) const {  return v != GetValue();  }
  bool operator > (h_t v) const {  return v > GetValue();  }
  bool operator >= (h_t v) const {  return v >= GetValue();  }
  bool operator < (h_t v) const {  return v < GetValue();  }
  bool operator <= (h_t v) const {  return v <= GetValue();  }
  h_t operator + (h_t v) const {  return h_t(GetValue()+v);  }
  h_t operator - (h_t v) const {  return h_t(GetValue()-v);  }
  h_t operator / (h_t v) const {  return h_t(GetValue()/v);  }
  h_t operator * (h_t v) const {  return h_t(GetValue()*v);  }
  heir_t &operator += (h_t v) {  return SetValue(GetValue()+v);  }
  heir_t &operator -= (h_t v) {  return SetValue(GetValue()-v);  }
  heir_t &operator *= (h_t v) {  return SetValue(GetValue()*v);  }
  heir_t &operator /= (h_t v) {  return SetValue(GetValue()/v);  }

  template <class aheir_t>
  bool operator == (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() == GetValue();
  }
  template <class aheir_t>
  bool operator != (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() != GetValue();
  }
  template <class aheir_t>
  bool operator > (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() > GetValue();
  }
  template <class aheir_t>
  bool operator >= (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() >= GetValue();
  }
  template <class aheir_t>
  bool operator < (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() < GetValue();
  }
  template <class aheir_t>
  bool operator <= (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue() <= GetValue();
  }
  template <class aheir_t>
  h_t operator + (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue()+GetValue();
  }
  template <class aheir_t>
  h_t operator - (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue()-GetValue();
  }
  template <class aheir_t>
  h_t operator * (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue()*GetValue();
  }
  template <class aheir_t>
  h_t operator / (const linked_operators<a_t, aheir_t, h_t> &v) const {
    return v.GetValue()/GetValue();
  }
  template <class aheir_t>
  heir_t &operator += (const linked_operators<a_t, aheir_t, h_t> &v)  {
    return SetValue(v.GetValue()+GetValue());
  }
  template <class aheir_t>
  heir_t &operator -= (const linked_operators<a_t, aheir_t, h_t> &v)  {
    return SetValue(v.GetValue()-GetValue());
  }
  template <class aheir_t>
  heir_t &operator *= (const linked_operators<a_t, aheir_t, h_t> &v)  {
    return SetValue(v.GetValue()*GetValue());
  }
  template <class aheir_t>
  heir_t &operator /= (const linked_operators<a_t, aheir_t, h_t> &v)  {
    return SetValue(v.GetValue()/GetValue());
  }
};

template <typename a_t, class heir_t, typename h_t>
static bool operator == (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v == h.GetValue();
}
template <typename a_t, class heir_t, typename h_t>
static bool operator != (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v != h.GetValue();
}
template <typename a_t, class heir_t, typename h_t>
static bool operator > (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v > h.GetValue();
}
template <typename a_t, class heir_t, typename h_t>
static bool operator >= (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v >= h.GetValue();
}
template <typename a_t, class heir_t, typename h_t>
static bool operator < (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v < h.GetValue();
}
template <typename a_t, class heir_t, typename h_t>
static bool operator <= (a_t v, const linked_operators<heir_t,h_t,a_t> &h) {
  return v <= h.GetValue();
}

EndEsdlNamespace()
#endif
