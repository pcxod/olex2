/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ptr_H
#define __olx_sdl_ptr_H

template <typename ptr> struct olx_ptr_  {
  ptr* p;
  int ref_cnt;
  olx_ptr_(ptr* _p) : p(_p), ref_cnt(1)  {}
  olx_ptr_* inc_ref()  {  ref_cnt++;  return this;  }
  template <bool is_array> int dec_ref() {
    int rc = --ref_cnt;
    if (rc <= 0) {
      if (p != NULL) {
        if (is_array)
          delete [] p;
        else
          delete p;
      }
      delete this;
    }
    return rc;
  }
};

template <typename ptr> struct olx_object_ptr  {
  olx_ptr_<ptr>* p;
  olx_object_ptr(ptr* _p) {  p = new olx_ptr_<ptr>(_p);  }
  olx_object_ptr(const olx_object_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_object_ptr()  { p->dec_ref<false>(); }
  olx_object_ptr& operator = (const olx_object_ptr& _p)  {
    p->dec_ref<false>();
    p = _p.p->inc_ref();
    return *this;
  }
  olx_object_ptr& operator = (ptr *p_)  {
    if (--p->ref_cnt <= 0 && p->p != NULL)
      delete p->p;
    p->p = p_;
    p->ref_cnt = 1;
    return *this;
  }
  ptr& operator ()() const {  return *p->p;  }
  operator ptr& () const {  return *p->p;  }
  // releases the object from ALL references
  ptr &release() const {
    ptr *p_ = p->p;
    p->p = NULL;
    return *p_;
  }
};

template <typename ptr> struct olx_array_ptr  {
  olx_ptr_<ptr>* p;
  olx_array_ptr(ptr* _p) {  p = new olx_ptr_<ptr>(_p);  }
  olx_array_ptr(const olx_array_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_array_ptr() { p->dec_ref<true>(); }
  olx_array_ptr& operator = (const olx_array_ptr& _p)  {
    p->dec_ref<true>();
    p = _p.p->inc_ref();
    return *this;
  }
  olx_array_ptr& operator = (ptr *p_)  {
    if (--p->ref_cnt <= 0 && p->p != NULL)
      delete [] p->p;
    p->p = p_;
    p->ref_cnt = 1;
    return *this;
  }
  bool is_null() const { return p->p == NULL; }
  ptr* operator ()() const {  return p->p;  }
  operator ptr* () const {  return p->p;  }
  // releases the array from ALL references
  ptr *release() const {
    ptr *p_ = p->p;
    p->p = NULL;
    return p_;
  }
};

struct olx_ref {
  template <class P>
  static P& get(P* p)  {  return *p;  }
  static char *get(char *p)  {  return p;  }
  static wchar_t *get(wchar_t *p)  {  return p;  }
  template <class P>
  static const P& get(const P* p)  {  return *p;  }
  static const char *get(const char *p)  {  return p;  }
  static const wchar_t *get(const wchar_t *p)  {  return p;  }
  template <class P>
  static P& get(P& p)  {  return p;  }
  template <class P>
  static const P& get(const P& p)  {  return p;  }
};

struct olx_ptr {
  template <class P>
  static P* get(P* p)  {  return p;  }
  template <class P>
  static const P* get(const P* p)  {  return p;  }
  template <class P>
  static P* get(P& p)  {  return &p;  }
  template <class P>
  static const P* get(const P& p)  {  return &p;  }
};

#endif
