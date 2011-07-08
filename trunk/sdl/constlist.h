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
BeginEsdlNamespace()

template <class cont_t, typename item_t>
class const_list {
protected:
  mutable olx_ptr<cont_t> *p;
  static void throw_invalid(const char* file, const char* function, int line) {
    TExceptionBase::ThrowFunctionFailed(file, function, line, "uninitialised object");
  }
public:
  const_list(const const_list &l) : p(l.p == NULL ? NULL : l.p->inc_ref())  {}
  const_list(cont_t *l) : p (new olx_ptr<cont_t>(l)) {}
  const_list(cont_t &l) : p(new olx_ptr<cont_t>(new cont_t)) {
    p->p->TakeOver(l);
  }
  ~const_list()  {
    if( p != NULL && --p->ref_cnt == 0 ) {
      delete p->p;
      delete p;
    }
  }
  const item_t &operator [] (size_t i) const {  return (*p->p)[i];  }
  size_t Count() const {  return p == NULL ? 0 : p->p->Count();  }
  bool IsEmpty() const {  return Count() == 0;  }
  operator const cont_t &() const {  return GetList();  }
  const cont_t& GetList() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    return *p->p;
  }
  const_list& operator = (const const_list &a) {
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
    }
    else  {
      rv = new cont_t(*p->p);
    }
    p = NULL;
    return *rv;
  }
  bool IsValid() const {  return p != NULL;  }
};

EndEsdlNamespace()
#endif
