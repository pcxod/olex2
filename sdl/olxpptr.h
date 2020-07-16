/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_pptr_H
#define __olx_sdl_pptr_H
#include "olxvptr.h"

/* a virtual pointer implementation for perishable objects - objects which can
be deleted before this pointer container. Use is_valid() to check if the
underlying object exists
*/
template <typename ptr> struct olx_perishable_ptr
  : public olx_object_ptr<olx_virtual_ptr<ptr> >, public virtual IOlxObject
{
  typedef olx_object_ptr<olx_virtual_ptr<ptr> > parent_t;
  typedef olx_vptr<ptr> vptr_t;

  void on_object_delete(APerishable* o) {
    parent_t::operator = (0);
  }
  void add_handler(APerishable& o) {
    o.AddDestructionObserver(DestructionObserver::MakeNew(this,
      &olx_perishable_ptr::on_object_delete));
  }
  void remove_handler() {
    if (ok()) {
      parent_t::p->p->get().RemoveDestructionObserver(
        DestructionObserver::Make(this, &olx_perishable_ptr::on_object_delete));
    }
  }
  olx_perishable_ptr(ptr *_p)
    : parent_t(new typename vptr_t::template actual_ptr<ptr>(_p))
  {
    if (_p != 0) {
      add_handler(*_p);
    }
  }
  olx_perishable_ptr(olx_virtual_ptr<ptr> *_p)
    : parent_t(_p)
  {
    if (_p->get_ptr() != 0) {
      add_handler(_p->get());
    }
  }
  template <class dptr>
  olx_perishable_ptr(olx_virtual_ptr<dptr> *_p)
    : parent_t(new typename vptr_t:: template ptr_proxy<dptr>(_p))
  {
    if (_p->get_ptr() != 0) {
      add_handler(_p->get());
    }
  }
  olx_perishable_ptr(const olx_perishable_ptr& _p)
    : parent_t(_p)
  {}
  virtual ~olx_perishable_ptr() {
    remove_handler();
  }
  olx_perishable_ptr& operator = (const olx_perishable_ptr& _p) {
    if (_p.self_ptr() == this) {
      return *this;
    }
    if (parent_t::p->template dec_ref<true, false>() != 0) {
      remove_handler();
    }
    parent_t::p = _p.p->inc_ref();
    if (ok()) {
      add_handler(parent_t::p->p->get());
    }
    return *this;
  }
  olx_perishable_ptr& operator = (ptr *_p) {
    remove_handler();
    parent_t::operator = (new typename vptr_t:: template actual_ptr<ptr>(_p));
    if (ok()) {
      add_handler(parent_t::p->p->get());
    }
    return *this;
  }

  bool ok() const {
    return (parent_t::ok() && parent_t::p->p->get_ptr() != 0);
  }

  const ptr* operator ->() const { return &parent_t::p->p->get(); }
  ptr* operator ->() { return &parent_t::p->p->get(); }

  const ptr* operator &() const { return &parent_t::p->p->get(); }
  ptr* operator &() { return &parent_t::p->p->get(); }

  const ptr& operator *() const { return parent_t::p->p->get(); }
  ptr& operator *() { return parent_t::p->p->get(); }

  operator const ptr& () const { return parent_t::p->p->get(); }
  operator ptr& () { return parent_t::p->p->get(); }

  bool operator == (const olx_perishable_ptr &ap) const {
    return parent_t::p->p->get_ptr() == ap.p->p->get_ptr();
  }
  bool operator == (const ptr *p) const {
    return parent_t::p->p->get_ptr() == p;
  }
  // releases the object from ALL references
  ptr *release() {
    if (!ok()) {
      return 0;
    }
    ptr &p_ = parent_t::p->p->get();
    remove_handler();
    parent_t::operator = (0);
    return &p_;
  }
protected:
  const olx_perishable_ptr* self_ptr() const { return this; }
};

#endif
