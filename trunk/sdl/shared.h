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
public:
  class Item {
    size_t index;
    shared_base &instance;
  public:
    Item(size_t idx, shared_base &inst) : index(idx), instance(inst) {}
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
protected:
  mutable olx_ptr<cont_t> *p;
  void Set(size_t i, const item_t &v)  {
    if( p->ref_cnt > 1 ) {
      p->ref_cnt--;
      p = new olx_ptr<cont_t>(new cont_t(*p->p));
      (*p->p)[i] = v;
    }
    else
      (*p->p)[i] = v;
  }
  const item_t &Get(size_t i) const {  return (*p->p)[i];  }
  static void throw_invalid(const char* file, const char* function, int line) {
    TExceptionBase::ThrowFunctionFailed(file, function, line, "uninitialised object");
  }
  void on_modify()  {
    if( p->ref_cnt > 1 ) {
      p->ref_cnt--;
      p = new olx_ptr<cont_t>(new cont_t(*p->p));
    }
  }
public:
  shared_base() : p(new olx_ptr<cont_t>(new cont_t))  {}
  shared_base(const shared_base &l)  : p(l.p->inc_ref())  {}
  shared_base(cont_t *l) : p (new olx_ptr<cont_t>(l)) {}
  shared_base(cont_t &l) : p(new olx_ptr<cont_t>(&l)) {}
  ~shared_base()  {
    if( p != NULL && --p->ref_cnt == 0 ) {
      delete p->p;
      delete p;
    }
  }
  Item operator [] (size_t i)  {  return Item(i, *this);  }
  const item_t &operator [] (size_t i) const {  return (*p->p)[i];  }
  size_t Count() const {  return p == NULL ? 0 : p->p->Count();  }
  bool IsEmpty() const {  return Count() == 0;  }
  void Delete(size_t i)  {
    on_modify();
    p->p->Delete(i);  
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

template <class list_t, typename item_t>
class shared_list : public shared_base<list_t,item_t> {
  typedef shared_base<list_t,item_t> _parent_t;
public:
  shared_list() {}
  shared_list(const shared_list &l) : _parent_t(l) {}
  shared_list(list_t *l) : _parent_t(l) {}
  shared_list(list_t &l) : _parent_t(l) {}
  item_t &Add(item_t *i)  {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    on_modify();
    return p->p->Add(i);
  }
  item_t &Add(item_t &i)  {  return Add(&i);  };
  shared_list& operator = (const shared_list &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

template <class list_t, typename item_t>
class shared_ptr_list : public shared_base<list_t,item_t*> {
  typedef shared_base<list_t,item_t*> _parent_t;
public:
  shared_ptr_list() {}
  shared_ptr_list(const shared_ptr_list &l) : _parent_t(l) {}
  shared_ptr_list(list_t *l) : _parent_t(l) {}
  shared_ptr_list(list_t &l) : _parent_t(l) {}
  item_t *Add(item_t *i)  {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    on_modify();
    return p->p->Add(i);
  }
  item_t *Add(item_t &i)  {  return Add(&i);  }
  shared_ptr_list& operator = (const shared_ptr_list &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

template <class array_t, typename item_t>
class shared_array : public shared_base<array_t,item_t> {
  typedef shared_base<array_t,item_t> _parent_t;
public:
  shared_array() {}
  shared_array(const shared_array &l) : _parent_t(l) {}
  shared_array(array_t *l) : _parent_t(l) {}
  shared_array(array_t &l) : _parent_t(l) {}
  item_t &Add(const item_t &i)  {
    if( _parent_t::p == NULL )
      throw_invalid(__POlxSourceInfo);
    on_modify();
    return _parent_t::p->p->Add(i);
  }
  shared_array& operator = (const shared_array &a) {
    _parent_t::operator = (a);
    return *this;
  }
};

EndEsdlNamespace()
#endif
