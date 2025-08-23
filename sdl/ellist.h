/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* primitive directional lists, optimised for sequential access, the right
cleanup class must be chosen to avoid memory leaks!
*/
#ifndef __olx_sdl_linked_indexable_list_H
#define __olx_sdl_linked_indexable_list_H
#include "linked_list.h"
#include "constlist.h"
BeginEsdlNamespace()

template <typename> class ConstLList;

template <typename T, class cleanupClass=DummyCleanup>
class TUDTypeList : public TLinkedList<T, cleanupClass> {
  typedef TLinkedList<T, cleanupClass> parent_t;
  mutable size_t pos;
  mutable typename parent_t::Entry* cur;
public:
  TUDTypeList()
    : pos(InvalidIndex), cur(0)
  {}

  TUDTypeList(const ConstLList<T>& list)
    : pos(InvalidIndex), cur(0)
  {
    TakeOver(list.Release(), true);
  }

  virtual ~TUDTypeList() { Clear(); }

  void Clear() {
    parent_t::Clear();
    cur = 0;
    pos = InvalidIndex;
  }

  TUDTypeList& Reset() {
    cur = 0;
    pos = InvalidIndex;
    return *this;
  }

  TUDTypeList& operator = (const ConstLList<T>& l) {
    return TakeOver(l.Release(), true);
  }

  T& operator [] (size_t ind) const {
    if (ind >= parent_t::count) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid index");
    }
    if (pos == (ind - 1)) {
      return Next();
    }
    else if (ind == 0) {
      cur = parent_t::first;
      pos = 0;
      return cur->data;
    }
    pos = 0;
    cur = parent_t::first;
    while (ind-- != 1) {
      cur = cur->next;
      pos++;
    }
    return cur->data;
  }

  T& Add(const T &v) {
    pos++;
    return parent_t::Add(v);
  }

  TUDTypeList& TakeOver(TUDTypeList& src, bool do_delete=false) {
    pos = src.pos;
    cur = src.cur;

    src.cur = 0;
    src.pos = InvalidIndex;
    parent_t::TakeOver(src, do_delete);
    return *this;
  }

protected:
  bool HasNext() const {
    return (cur == 0 ? parent_t::first : cur->next) != 0;
  }
  T& Next() const {
    if (cur != 0) {
      cur = cur->next;
    }
    else {
      cur = parent_t::first;
    }
    if (cur == 0) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
    }
    pos++;
    return cur->data;
  }
public:
  typedef ConstLList<T> const_list_type;
  typedef T list_item_type;
  olx_list_2_std;
};

template <typename T>
class ConstLList : public const_list<TUDTypeList<T> > {
  typedef TUDTypeList<T> lst_t;
  typedef const_list<lst_t> parent_t;
public:
  ConstLList(const ConstLList& l) : parent_t(l) {}
  ConstLList(lst_t* lst) : parent_t(lst) {}
  ConstLList(lst_t& lst) : parent_t(lst) {}
  ConstLList& operator = (const ConstLList& l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef T list_item_type;
};

EndEsdlNamespace()
#endif
