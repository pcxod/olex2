/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_set_H
#define __olx_sdl_set_H
#include "sortedlist.h"
BeginEsdlNamespace()

template <typename, typename> class const_olxset;

template <typename object_t, class cmp_t> class olxset
: protected SortedObjectList<object_t, cmp_t>
{
  typedef SortedObjectList<object_t, cmp_t> list_t;
public:
  olxset() {}

  olxset(const cmp_t &cmp)
    : list_t(cmp)
  {}
  olxset(const object_t _values[], size_t cnt, const cmp_t &cmp)
    : list_t(cmp)
  {
    for (size_t i = 0; i < cnt; i++)
      Add(_values[i]);
  }
  olxset(const object_t _values[], size_t cnt) {
    for (size_t i = 0; i < cnt; i++)
      Add(_values[i]);
  }
  olxset(const olxset& ad) : list_t(ad) {}
  olxset(const const_olxset<object_t, cmp_t>& ad)
    : list_t(ad.GetObject().cmp)
  {
    list_t::TakeOver(ad.Release(), true);
  }
  void TakeOver(olxset &d)  { list_t::TakeOver(d); }
  olxset& operator = (const olxset& ad) {
    list_t::operator = (ad);
    return *this;
  }
  olxset& operator = (const const_olxset<object_t, cmp_t>& ad)  {
    list_t::TakeOver(ad.Release(), true);
    return *this;
  }
  size_t Count() const { return list_t::Count(); }
  const object_t& operator [] (size_t i) const {
    return list_t::operator[] (i);
  }
  template <class T> bool Contains(const T& key) const {
    return list_t::IndexOf(key) != InvalidIndex;
  }
  template <typename T> bool Add(const T& obj) {
    return list_t::AddUnique(obj).b;
  }
  template <typename T> olxset &operator << (const T& obj) {
    list_t::AddUnique(obj);
    return *this;
  }
  template <class T> size_t IndexOf(const T& key) const {
    return list_t::IndexOf(key);
  }
  template <class T> bool Remove(const T& v) {
    size_t ind = list_t::IndexOf(v);
    if (ind != InvalidIndex) {
      list_t::Delete(ind);
      return true;
    }
    return false;
  }
  void Delete(size_t ind)  { list_t::Delete(ind); }

  void Merge(const olxset &s) {
    for (size_t i = 0; i < s.Count(); i++) {
      list_t::AddUnique(s[i]);
    }
  }
public:
  typedef object_t list_item_type;
  typedef object_t set_item_type;
  typedef const_olxset<object_t, cmp_t> const_set_type;
};

// a string set
template <bool case_insensitive = false>
class olxstr_set
  : public olxset<olxstr, olxstrComparator<case_insensitive> >
{
  typedef olxset<olxstr, olxstrComparator<case_insensitive> > parent_t;
public:
  olxstr_set()
    : parent_t(olxstrComparator<case_insensitive>())
  {}
};

// a primitive set
template <typename obj_t>
class olx_pset
  : public olxset<obj_t, TPrimitiveComparator>
{
public:
  olx_pset() : olxset<obj_t, TPrimitiveComparator>()
  {}
};

// a comparable set
template <typename obj_t>
class olx_cset
  : public olxset<obj_t, TComparableComparator>
{
public:
  olx_cset() : olxset<obj_t, TComparableComparator>()
  {}
};

// const_set
template <typename obj_t, class cmp_t>
class const_olxset : public const_list<olxset<obj_t, cmp_t> > {
  typedef olxset<obj_t, cmp_t> set_t;
  typedef const_list<set_t> parent_t;
public:
  const_olxset(const const_olxset &s) : parent_t(s) {}
  const_olxset(set_t &s) : parent_t(s) {}
  const_olxset(set_t *s) : parent_t(s) {}
  const_olxset&operator = (const const_olxset &s) {
    parent_t::operator = (s);
    return *this;
  }
};

EndEsdlNamespace()
#endif
