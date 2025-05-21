/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
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
  olxset(const olx_capacity_t &cap)
    : list_t(cap)
  {}
  olxset(const cmp_t& cmp, const olx_capacity_t& cap)
    : list_t(cmp, cap)
  {}
  olxset(const object_t _values[], size_t cnt, const cmp_t &cmp)
    : list_t(cmp)
  {
    for (size_t i = 0; i < cnt; i++) {
      Add(_values[i]);
    }
  }
  olxset(const object_t _values[], size_t cnt) {
    for (size_t i = 0; i < cnt; i++) {
      Add(_values[i]);
    }
  }
  olxset(const olxset &ad)
    : list_t(ad)
  {}

  olxset(const const_olxset<object_t, cmp_t>& ad)
    : list_t(ad.obj().cmp)
  {
    list_t::TakeOver(ad.Release(), true);
  }

  void TakeOver(olxset &d)  { list_t::TakeOver(d); }

  olxset &operator = (const olxset& ad) {
    list_t::operator = (ad);
    return *this;
  }

  olxset &operator = (const const_olxset<object_t, cmp_t> &ad)  {
    list_t::TakeOver(ad.Release(), true);
    return *this;
  }

  size_t Count() const { return list_t::Count(); }

  const object_t &operator [] (size_t i) const {
    return list_t::operator[] (i);
  }

  const object_t &Get(size_t i) const {
    return list_t::operator[] (i);
  }

  const object_t& GetListItem(size_t i) const {
    return list_t::operator[] (i);
  }

  template <class T> bool Contains(const T &key) const {
    return list_t::Contains(key);
  }

  template <typename T> bool Add(const T &obj) {
    return list_t::AddUnique(obj).b;
  }

  bool AddListItem(const object_t& obj) {
    return list_t::AddUnique(obj).b;
  }

  template <class coll_t> olxset& AddAll(const coll_t& l) {
    list_t::SetCapacity(list_t::Count() + l.Count());
    for (size_t i = 0; i < l.Count(); i++) {
      Add(l[i]);
    }
    return *this;
  }

  template <class coll_t, class Accessor> olxset& AddAll(const coll_t& l,
    const Accessor& accessor)
  {
    list_t::SetCapacity(list_t::Count() + l.Count());
    for (size_t i = 0; i < l.Count(); i++) {
      Add(accessor(l[i]));
    }
    return *this;
  }

  template <typename T> olxset &operator << (const  T &obj) {
    list_t::AddUnique(obj);
    return *this;
  }

  template <class T> size_t IndexOf(const T &key) const {
    return list_t::IndexOf(key);
  }

  bool RemoveListItem(const object_t &v) {
    return Remove(v);
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

  void Clear() { list_t::Clear(); }

  bool IsEmpty() const { return list_t::IsEmpty(); }

  olxset &Merge(const olxset &s) {
    for (size_t i = 0; i < s.Count(); i++) {
      list_t::AddUnique(s[i]);
    }
    return *this;
  }

  olxset &operator += (const olxset &s) { return Merge(s); }

  const_olxset<object_t, cmp_t> operator + (const olxset &s) const {
    olxset ts(*this);
    return ts.Merge(s);
  }

  const_olxset<object_t, cmp_t> operator & (const olxset &s) const {
    olxset rv;
    const olxset &a = (s.Count() > Count()) ? *this : s,
      &b = (&a == &s) ? *this : s;
    for (size_t i = 0; i < a.Count(); i++) {
      if (b.Contains(a.Get(i))) {
        rv.Add(a.Get(i));
      }
    }
    return rv;
  }

  olxset operator -= (const olxset &s) const {
    for (size_t i = 0; i < s.Count(); i++) {
      size_t idx = IndexOf(s[i]);
      if (idx != InvalidIndex) {
        Delete(idx);
      }
    }
    return *this;
  }

  const_olxset<object_t, cmp_t> operator - (const olxset &s) const {
    olxset rv;
    for (size_t i = 0; i < Count(); i++) {
      if (!s.Contains(Get(i))) {
        rv.Add(Get(i));
      }
    }
    return rv;
  }

  void SetIncrement(size_t v) { list_t::SetIncrement(v); }
  void SetCapacity(size_t v) { list_t::SetCapacity(v); }
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
  olxstr_set(const olx_capacity_t& cap)
    : parent_t(olxstrComparator<case_insensitive>(), cap)
  {}
};

// a primitive set
template <typename obj_t>
class olx_pset
  : public olxset<obj_t, TPrimitiveComparator>
{
  typedef olxset<obj_t, TPrimitiveComparator> parent_t;
public:
  typedef const_olxset<obj_t, TPrimitiveComparator> const_set_type;

  olx_pset() {}
  olx_pset(const olx_capacity_t& cap) : parent_t(cap) {}
  olx_pset(const olx_pset &s) : parent_t(s) {}
  olx_pset(const const_set_type &s) : parent_t(s) {}
  olx_pset &operator = (const olx_pset &s) {
    parent_t::operator = (s);
    return *this;
  }
  olx_pset &operator = (const const_set_type &s) {
    parent_t::operator = (s);
    return *this;
  }
};

// a comparable set
template <typename obj_t>
class olx_cset
  : public olxset<obj_t, TComparableComparator>
{
  typedef olxset<obj_t, TComparableComparator> parent_t;
public:
  typedef const_olxset<obj_t, TComparableComparator> const_set_type;

  olx_cset() {}
  olx_cset(const olx_capacity_t& cap) : parent_t(cap) {}
  olx_cset(const olx_cset &s) : parent_t(s) {}
  olx_cset(const const_set_type &s) : parent_t(s) {}
  olx_cset &operator = (const olx_cset &s) {
    parent_t::operator = (s);
    return *this;
  }
  olx_cset &operator = (const const_set_type &s) {
    parent_t::operator = (s);
    return *this;
  }
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
