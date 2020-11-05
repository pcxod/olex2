/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_constlist_H
#define __olx_sdl_constlist_H
#include "ebase.h"
#undef GetObject
BeginEsdlNamespace()

template <class obj_t>
class const_obj {
protected:
  mutable olx_ptr_<obj_t>* p;
  static void throw_invalid(const char* file, const char* function, int line) {
    TExceptionBase::ThrowFunctionFailed(file, function, line,
      "uninitialised object");
  }
public:
  const_obj(const const_obj& l)
    : p(l.p == 0 ? 0 : l.p->inc_ref())
  {}
  const_obj(obj_t* l)
    : p(new olx_ptr_<obj_t>(l))
  {}
  const_obj(obj_t& l)
    : p(new olx_ptr_<obj_t>(new obj_t))
  {
    p->p->TakeOver(l);
  }
  virtual ~const_obj() {
    if (p != 0 && --p->ref_cnt == 0) {
      delete p->p;
      delete p;
    }
  }
  //operator const obj_t &() const {  return GetObject();  }
  const obj_t& GetObject() const {
    if (p == 0) {
      throw_invalid(__POlxSourceInfo);
    }
    return *p->p;
  }
  const_obj& operator = (const const_obj& a) {
    if (p != 0 && --p->ref_cnt == 0) {
      delete p->p;
      delete p;
    }
    p = a.p->inc_ref();
    return *this;
  }
  // the caller must delete the container after this call
  obj_t& Release() const {
    if (p == 0) {
      throw_invalid(__POlxSourceInfo);
    }
    obj_t* rv;
    if (--p->ref_cnt == 0) {
      rv = p->p;
      delete p;
    }
    else {
      rv = new obj_t(*p->p);
    }
    p = 0;
    return *rv;
  }
  bool IsValid() const { return p != 0; }
public:
  typedef obj_t list_item_type;
};

template <class cont_t>
class const_list
  : public const_obj<cont_t>
{
  typedef const_obj<cont_t> _parent_t;
public:
  typedef typename cont_t::list_item_type list_item_type;

  const_list(const const_list &l)
    : _parent_t(l)
  {}
  const_list(cont_t *l)
    : _parent_t(l)
  {}
  const_list(cont_t &l)
    : _parent_t(l)
  {}
  const list_item_type &operator [] (size_t i) const {
    return (*_parent_t::p->p)[i];
  }
  const list_item_type &operator () (size_t i) const {
    return (*_parent_t::p->p)[i];
  }
  size_t Count() const {
    return _parent_t::p == 0 ? 0 : _parent_t::p->p->Count();
  }
  bool IsEmpty() const { return Count() == 0; }
  const_list& operator = (const const_list &a) {
    _parent_t:: operator = (a);
    return *this;
  }
};

template <class dict_t>
class const_dict : public const_obj<dict_t> {
  typedef const_obj<dict_t> _parent_t;
public:
  typedef typename dict_t::key_item_type key_item_type;
  typedef typename dict_t::value_item_type value_item_type;

  const_dict(const const_dict &l)
    : _parent_t(l)
  {}
  const_dict(dict_t *l)
    : _parent_t(l)
  {}
  const_dict(dict_t &l)
    : _parent_t(l)
  {}
  const key_item_type &GetKey(size_t i) const {
    return _parent_t::p->p->GetKey(i);
  }
  const value_item_type &GetValue(size_t i) const {
    return _parent_t::p->p->GetValue(i);
  }
  size_t Count() const {
    return _parent_t::p == 0 ? 0 : _parent_t::p->p->Count();
  }
  bool IsEmpty() const {  return Count() == 0;  }
  const_dict& operator = (const const_dict &a) {
    _parent_t:: operator = (a);
    return *this;
  }
};

template <class cont_t>
class const_mat : public const_obj<cont_t> {
  typedef const_obj<cont_t> _parent_t;
public:
  typedef typename cont_t::number_type number_type;

  const_mat(const const_mat &l) : _parent_t(l) {}
  const_mat(cont_t *l) : _parent_t(l) {}
  const_mat(cont_t &l) : _parent_t(l) {}
  const number_type &operator () (size_t i, size_t j) const {
    return (*_parent_t::p->p)(i, j);
  }
  size_t ColCount() const {
    return _parent_t::p == 0 ? 0 : _parent_t::p->p->ColCount();
  }
  size_t RowCount() const {
    return _parent_t::p == 0 ? 0 : _parent_t::p->p->RowCount();
  }
  bool IsEmpty() const {  return ColCount() == 0 || RowCount() == 0;  }
  const_mat& operator = (const const_mat &a) {
    _parent_t:: operator = (a);
    return *this;
  }
};
EndEsdlNamespace()
#endif
