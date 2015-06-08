/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_vptr_H
#define __olx_sdl_vptr_H
#include "olxptr.h"

/* a class that represent a virtual pointer */
template <class ptr> struct olx_virtual_ptr {
protected:
  mutable ptr *cached;
public:
  olx_virtual_ptr() : cached(0) {}
  virtual ~olx_virtual_ptr() {}
  virtual IOlxObject *get_ptr() const = 0;
  ptr &get() const {
    IOlxObject *p = get_ptr();
    if (p == cached) {
      return *cached;
    }
    ptr &rv = dynamic_cast<ptr &>(*p);
    cached = &rv;
    return rv;
  }
  ptr &operator()() const {
    return get();
  }
  operator ptr& () const { return get(); }
  bool operator == (const olx_virtual_ptr &ap) const {
    return get_ptr() == ap.get_ptr();
  }
};

// a virtual pointer implementation
template <typename ptr> struct olx_vptr
: public olx_object_ptr<olx_virtual_ptr<ptr> >
{
  typedef olx_object_ptr<olx_virtual_ptr<ptr> > parent_t;
  template <class dptr>
  struct actual_ptr : public olx_virtual_ptr<ptr> {
    dptr *p;
    actual_ptr(dptr *p) : p(p) {}
    virtual IOlxObject *get_ptr() const {
      return p;
    }
  };
  template <class dptr>
  struct ptr_proxy : public olx_virtual_ptr<ptr> {
    olx_object_ptr<olx_virtual_ptr<dptr> > proxied;
    ptr_proxy(olx_virtual_ptr<dptr> *p) : proxied(p) {}
    virtual IOlxObject *get_ptr() const {
      return proxied().get_ptr();
    }
  };
  olx_vptr(ptr *_p) : parent_t(new actual_ptr<ptr>(_p)) {}
  olx_vptr(olx_virtual_ptr<ptr> *_p) : parent_t(_p) {}
  template <class dptr>
  olx_vptr(olx_virtual_ptr<dptr> *_p) : parent_t(new ptr_proxy<dptr>(_p))
  {}
  olx_vptr(const olx_vptr& _p) : parent_t(_p) {}
  olx_vptr& operator = (const olx_vptr& _p) {
    parent_t::p->template dec_ref<true, false>();
    parent_t::p = _p.p->inc_ref();
    return *this;
  }
  IOlxObject *get_ptr() const { return parent_t::p->p->get_ptr(); }
  ptr& get() const { return parent_t::p->p->get(); }
  ptr& operator ()() const { return parent_t::p->p->get(); }
  operator ptr& () const { return parent_t::p->p->get(); }
  bool operator == (const olx_vptr &ap) const {
    return parent_t::p->p->get_ptr() == ap.p->p->get_ptr();
  }
  bool operator == (const ptr *p) const {
    return parent_t::p->p->get_ptr() == p;
  }
  // releases the object from ALL references
  ptr *release() const {
    ptr *p_ = parent_t::p->p;
    parent_t::p->p = NULL;
    return p_;
  }
};

#endif
