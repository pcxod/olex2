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
};

template <typename ptr> struct olx_object_ptr  {
  olx_ptr_<ptr>* p;
  olx_object_ptr(ptr* _p) {  p = new olx_ptr_<ptr>(_p);  }
  olx_object_ptr(const olx_object_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_object_ptr()  {
    if( --p->ref_cnt <= 0 )  {
      delete p->p;
      delete p;
    }
  }
  olx_object_ptr& operator = (const olx_object_ptr& _p)  {
    if( --p->ref_cnt <= 0 )  {
      delete p->p;
      delete p;
    }
    p = _p.p->inc_ref();
    return *this;
  }
  ptr& operator ()()  {  return *p->p;  }
  operator ptr& ()  {  return *p->p;  }
};

template <typename ptr> struct olx_array_ptr  {
  olx_ptr_<ptr>* p;
  olx_array_ptr(ptr* _p) {  p = new olx_ptr_<ptr>(_p);  }
  olx_array_ptr(const olx_array_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_array_ptr()  {
    if( --p->ref_cnt <= 0 )  {
      delete [] p->p;
      delete p;
    }
  }
  olx_array_ptr& operator = (const olx_array_ptr& _p)  {
    if( --p->ref_cnt <= 0 )  {
      delete [] p->p;
      delete p;
    }
    p = _p.p->inc_ref();
    return *this;
  }
  ptr* operator ()()  {  return p->p;  }
  operator ptr* ()  {  return p->p;  }
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
