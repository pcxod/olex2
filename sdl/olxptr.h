/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
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
  olx_ptr_(ptr* _p)
    : p(_p), ref_cnt(1)
  {}
  olx_ptr_* inc_ref()  {
    ref_cnt++;
    return this;
  }
  template <bool do_del, bool is_array> int dec_ref() {
    int rc = --ref_cnt;
    if (rc <= 0) {
      if (p != 0 && do_del) {
        if (is_array) {
          delete[] p;
        }
        else {
          delete p;
        }
      }
      delete this;
    }
    return rc;
  }
  bool operator == (const olx_ptr_ &ap) const {
    return p == ap.p;
  }
  bool operator != (const olx_ptr_& ap) const {
    return p != ap.p;
  }
};

template <class heir_t, typename ptr>
struct olx_ptr_base {
  typedef olx_ptr_base<heir_t, ptr> this_t;
  heir_t& self() { return *static_cast<heir_t*>(this); }
  const heir_t& self() const {
    return *static_cast<const heir_t*>(this);
  }
protected:
  olx_ptr_<ptr>* p;
public:
  olx_ptr_base() {
    p = new olx_ptr_<ptr>(0);
  }
  olx_ptr_base(ptr *p_) {
    p = new olx_ptr_<ptr>(p_);
  }
  olx_ptr_base(const heir_t& _p)
    : p(_p.p->inc_ref())
  {}

  // use this to prevent the pointer from the removal
  void inc_ref() {
    p->inc_ref();
  }

  virtual ~olx_ptr_base() {
    self().dec_ref();
  }

  this_t& operator = (const this_t& _p) {
    self().dec_ref();
    p = _p.p->inc_ref();
    return *this;
  }
  bool ok() const { return p->p != 0; }
  ptr* release() {
    ptr* p_ = p->p;
    p->p = 0;
    return p_;
  }
  void reset(ptr *def=0) {
    if (self().dec_ref() == 0) {
      p = new olx_ptr_<ptr>(def);
    }
    else {
      p->p = def;
    }
  }
  bool operator == (const this_t& ap) const {
    return *p == *ap.p;
  }
  bool operator != (const this_t& ap) const {
    return !(operator == (ap));
  }

  bool operator == (uintptr_t pv) const {
    return reinterpret_cast<uintptr_t>(p->p) == pv;
  }
  bool operator != (uintptr_t pv) const {
    return reinterpret_cast<uintptr_t>(p->p) != pv;
  }
};

template <typename ptr> struct olx_object_ptr
  : public olx_ptr_base<olx_object_ptr<ptr>, ptr >
{
  typedef olx_ptr_base<olx_object_ptr<ptr>, ptr> parent_t;
  olx_object_ptr() {}
  olx_object_ptr(ptr* p)
    : parent_t(p)
  {}
  olx_object_ptr(const olx_object_ptr &p)
    : parent_t(p)
  {}
  olx_object_ptr& operator = (const olx_object_ptr& _p) {
    parent_t::p->template dec_ref<true, false>();
    parent_t::p = _p.p->inc_ref();
    return *this;
  }
  olx_object_ptr& operator = (ptr* p_) {
    if (--parent_t::p->ref_cnt <= 0 && parent_t::p->p != 0) {
      delete parent_t::p->p;
    }
    parent_t::p->p = p_;
    parent_t::p->ref_cnt = 1;
    return *this;
  }
  olx_object_ptr& operator = (const ptr &p_) {
    if (--parent_t::p->ref_cnt <= 0 && parent_t::p->p != 0) {
      delete parent_t::p->p;
    }
    parent_t::p->p = new ptr(p_);
    parent_t::p->ref_cnt = 1;
    return *this;
  }
 
  const ptr* operator &() const { return parent_t::p->p; }
  ptr* operator &() { return parent_t::p->p; }

  const ptr& operator *() const { return *parent_t::p->p; }
  ptr& operator *() { return *parent_t::p->p; }

  const ptr* operator ->() const { return parent_t::p->p; }
  ptr* operator ->() { return parent_t::p->p; }

  operator const ptr& () const { return *parent_t::p->p; }
  operator ptr& () { return *parent_t::p->p; }

protected:
  friend parent_t;
  int dec_ref() {
    return parent_t::p->template dec_ref<true, false>();
  }
};

template <typename ptr> struct olx_array_ptr
  : public olx_ptr_base<olx_array_ptr<ptr>, ptr>
{
  typedef olx_ptr_base<olx_array_ptr<ptr>, ptr> parent_t;
  olx_array_ptr() {}
  olx_array_ptr(ptr* p)
  : parent_t(p)
  {}
  olx_array_ptr(const olx_array_ptr &p)
    : parent_t(p)
  {}
  olx_array_ptr(size_t sz)
    : parent_t(new ptr[sz])
  {}

  olx_array_ptr& operator = (ptr* p_) {
    if (--parent_t::p->ref_cnt <= 0 && parent_t::p->p != 0) {
      delete[] parent_t::p->p;
    }
    parent_t::p->p = p_;
    parent_t::p->ref_cnt = 1;
    return *this;
  }

  const ptr* operator *() const { return parent_t::p->p; }
  ptr* operator *() { return parent_t::p->p; }

  const ptr* operator &() const { return parent_t::p->p; }
  ptr* operator &() { return parent_t::p->p; }

  operator const ptr* () const {  return parent_t::p->p;  }
  operator ptr* () { return parent_t::p->p; }
  
  template <typename idx_t_>
  const ptr& operator [] (idx_t_ i) const { return parent_t::p->p[i]; }
  template <typename idx_t_>
  ptr& operator [] (idx_t_ i) { return parent_t::p->p[i]; }

  static olx_array_ptr copy(const ptr *p, size_t sz) {
    ptr *rv = new ptr[sz];
    memcpy(rv, p, sz*sizeof(ptr));
    return olx_array_ptr(rv);
  }

  typedef ptr object_t;
protected:
  friend parent_t;
  int dec_ref() {
    return parent_t::p->template dec_ref<true, true>();
  }
};

struct olx_ref {
  template <class P>
  static P& get(P* p) { return *p; }
  static char *get(char *p) { return p; }
  static wchar_t *get(wchar_t *p) { return p; }
  template <class P>
  static const P& get(const P *p) { return *p; }
  static const char *get(const char *p) { return p; }
  static const wchar_t *get(const wchar_t *p) { return p; }
  template <class P>
  static P& get(P& p) { return p; }
  template <class P>
  static const P& get(const P& p) { return p; }
};

struct olx_ptr {
  template <class P>
  static P* get(P* p) { return p; }
  template <class P>
  static const P* get(const P* p) { return p; }
  template <class P>
  static P* get(P& p) { return &p; }
  template <class P>
  static const P* get(const P& p) { return &p; }

  template <typename T>
  static olx_array_ptr<T> copy(const T *d, size_t sz) {
    return olx_array_ptr<T>::copy(d, sz);
  }
};

// this object could be safelly passed to/from dll function calls
template <typename ptr> struct olx_dll_ptr {
protected:
  struct olx_dll_ptr_ {
    void *(*allocator_func)(size_t sz);
    void(*deallocator_func)(void *p);
    ptr* p;
    int ref_cnt;

    void init() {
      allocator_func = &olx_dll_ptr_::alloc;
      deallocator_func = &olx_dll_ptr_::dealloc;
    }
    olx_dll_ptr_()
      : p(0), ref_cnt(1)
    {
      init();
    }

    olx_dll_ptr_(size_t cnt)
      : p(alloc(cnt)), ref_cnt(1)
    {
      init();
    }

    olx_dll_ptr_(ptr *p_)
      : p(p_), ref_cnt(1)
    {
      init();
    }

    olx_dll_ptr_* inc_ref() {
      ref_cnt++;
      return this;
    }
    int dec_ref() {
      int rc = --ref_cnt;
      if (rc <= 0) {
        if (p != 0) {
          (*deallocator_func)(p);
        }
        delete this;
      }
      return rc;
    }
    static void *alloc(size_t sz) { return malloc(sz); }
    static void dealloc(void *p) { free(p); }
    static void *operator new(size_t n){ return alloc(n); }
      static void operator delete(void *p) {
      (*((olx_dll_ptr_*)p)->deallocator_func)(p);
    }
  };
public:
  olx_dll_ptr_* p;
  olx_dll_ptr() { p = new olx_dll_ptr_(); }
  olx_dll_ptr(size_t sz) { p = new olx_dll_ptr_(sz); }
  olx_dll_ptr(ptr *p_) { p = new olx_dll_ptr_(p_); }
  olx_dll_ptr(const olx_dll_ptr& _p)
    : p(_p.p->inc_ref())
  {}
  ~olx_dll_ptr() { p->dec_ref(); }
  olx_dll_ptr& operator = (const olx_dll_ptr& p_) {
    p->dec_ref();
    p = p_.p->inc_ref();
    return *this;
  }
  olx_dll_ptr& operator = (ptr *p_) {
    p->dec_ref();
    p = new olx_dll_ptr(p_);
    return *this;
  }
  bool ok() const { return p->p != 0; }

  const ptr* operator ->() const { return p->p; }
  ptr* operator ->() { return p->p; }

  const ptr* operator *() const { return p->p; }
  ptr* operator *() { return p->p; }

  //operator const ptr* () const { return p->p; }
  //operator ptr* () { return p->p; }

  virtual const ptr* get_ptr() const { return p->p; }

  static olx_dll_ptr copy(const ptr *p, size_t sz) {
    size_t tsz = sz*sizeof(ptr);
    ptr *rv = (ptr *)olx_dll_ptr_::alloc(tsz);
    memcpy(rv, p, tsz);
    return olx_dll_ptr(rv);
  }
};

#endif
