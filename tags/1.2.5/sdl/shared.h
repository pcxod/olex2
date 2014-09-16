/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_shared_H
#define __olx_sdl_shared_H
#include "ebase.h"
BeginEsdlNamespace()

template <class cont_t, typename item_t>
class shared_base {
protected:
  mutable olx_ptr_<cont_t> *p;
  static void throw_invalid(const char* file, const char* function, int line) {
    TExceptionBase::ThrowFunctionFailed(file, function, line,
      "uninitialised object");
  }
  void on_modify()  {
    if( p->ref_cnt > 1 ) {
      p->ref_cnt--;
      p = new olx_ptr_<cont_t>(new cont_t(*p->p));
    }
  }
public:
  shared_base() : p(new olx_ptr_<cont_t>(new cont_t))  {}
  shared_base(const shared_base &l) : p(l.p == NULL ? NULL : l.p->inc_ref())  {}
  shared_base(cont_t *l) : p (new olx_ptr_<cont_t>(l)) {}
  shared_base(cont_t &l) : p(new olx_ptr_<cont_t>(new cont_t)) {
    p->p->TakeOver(l);
  }
  ~shared_base()  {
    if( p != NULL && --p->ref_cnt == 0 ) {
      delete p->p;
      delete p;
    }
  }
  operator const cont_t &() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    return *p->p;
  }
  const cont_t& GetObject() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    return *p->p;
  }
  shared_base& operator = (const shared_base &a) {
    if( p != NULL && --p->ref_cnt == 0 )  {
      delete p->p;
      delete p;
    }
    p = a.p->inc_ref();
    return *this;
  }
  // the caller must delete the container after this call
  cont_t &Release() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    cont_t *rv;
    if( --p->ref_cnt == 0 )  {
      rv = p->p;
      delete p;
      p = NULL;
    }
    else  {
      rv = new cont_t(*p->p);
      p = NULL;
    }
    return *rv;
  }
  bool IsValid() const {  return p != NULL;  }
};

template <class cont_t, typename item_t>
class shared_list_base : public shared_base<cont_t, item_t> {
public:
  class Item {
    size_t index;
    shared_list_base<cont_t,item_t> &instance;
  public:
    Item(size_t idx, shared_list_base<cont_t,item_t> &inst)
      : index(idx), instance(inst) {}
    Item& operator = (const item_t &v) {
      instance.Set(index, v);
      return *this;
    }
    Item& operator = (const Item &v) {
      instance->Set(index, v.instance.Get(v.index));
      return *this;
    }
    operator const item_t & () {  return instance.Get(index);  }
  };
  void Set(size_t i, const item_t &v)  {
    typedef shared_base<cont_t, item_t> p_t;
    if( p_t::p->ref_cnt > 1 ) {
      p_t::p->ref_cnt--;
      p_t::p = new olx_ptr_<cont_t>(new cont_t(*p_t::p->p));
      (*p_t::p->p)[i] = v;
    }
    else
      (*p_t::p->p)[i] = v;
  }
  const item_t &Get(size_t i) const {
    return (*shared_base<cont_t, item_t>::p->p)[i];
  }
public:
  shared_list_base() {}
  shared_list_base(const shared_list_base &l)
    : shared_base<cont_t, item_t>(l)  {}
  shared_list_base(cont_t *l)
    : shared_base<cont_t, item_t>(l) {}
  shared_list_base(cont_t &l)
    : shared_base<cont_t, item_t>(l) {}
  Item operator [] (size_t i)  {  return Item(i, *this);  }
  const item_t &operator [] (size_t i) const {
    return (*shared_base<cont_t,item_t>::p->p)[i];
  }
  size_t Count() const {
    return shared_base<cont_t,item_t>::p == NULL ? 0
      : shared_base<cont_t,item_t>::p->p->Count();
  }
  bool IsEmpty() const {  return Count() == 0;  }
  void Delete(size_t i)  {
    shared_base<cont_t,item_t>::on_modify();
    shared_base<cont_t,item_t>::p->p->Delete(i);  
  }
  void IncCapacity(size_t v)  {
    shared_base<cont_t,item_t>::on_modify();
    shared_base<cont_t,item_t>::p->p->Setcapcity(
      shared_base<cont_t,item_t>::p->p->GetCount()+v);
  }
};

template <class list_t, typename item_t>
class shared_list : public shared_list_base<list_t,item_t> {
  typedef shared_list_base<list_t,item_t> _parent_t;
public:
  shared_list() {}
  shared_list(const shared_list &l) : _parent_t(l) {}
  shared_list(list_t *l) : _parent_t(l) {}
  shared_list(list_t &l) : _parent_t(l) {}
  item_t &Add(item_t *i)  {
    if( _parent_t::p == NULL )
      _parent_t::throw_invalid(__POlxSourceInfo);
    _parent_t::on_modify();
    return _parent_t::p->p->Add(i);
  }
  item_t &Add(item_t &i)  {  return Add(&i);  }
  shared_list& operator = (const shared_list &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

template <class list_t, typename item_t>
class shared_ptr_list : public shared_list_base<list_t,item_t*> {
  typedef shared_list_base<list_t,item_t*> _parent_t;
public:
  shared_ptr_list() {}
  shared_ptr_list(const shared_ptr_list &l) : _parent_t(l) {}
  shared_ptr_list(list_t *l) : _parent_t(l) {}
  shared_ptr_list(list_t &l) : _parent_t(l) {}
  item_t *Add(item_t *i)  {
    if( _parent_t::p == NULL )
      _parent_t::throw_invalid(__POlxSourceInfo);
    _parent_t::on_modify();
    return _parent_t::p->p->Add(i);
  }
  item_t *Add(item_t &i)  {  return Add(&i);  }
  shared_ptr_list& operator = (const shared_ptr_list &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

template <class array_t, typename item_t>
class shared_array : public shared_list_base<array_t,item_t> {
  typedef shared_list_base<array_t,item_t> _parent_t;
public:
  shared_array() {}
  shared_array(const shared_array &l) : _parent_t(l) {}
  shared_array(array_t *l) : _parent_t(l) {}
  shared_array(array_t &l) : _parent_t(l) {}
  item_t &Add(const item_t &i)  {
    if( _parent_t::p == NULL )
      _parent_t::throw_invalid(__POlxSourceInfo);
    _parent_t::on_modify();
    return _parent_t::p->p->Add(i);
  }
  shared_array& operator = (const shared_array &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

EndEsdlNamespace()
#endif
