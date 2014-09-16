/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_dict_H
#define __olx_sdl_dict_H
#include "sortedlist.h"
BeginEsdlNamespace()

template <typename, typename, typename> class const_olxdict;

template <typename key_c, typename val_c, class ComparatorType>
struct DictEntry {
  key_c key;
  mutable val_c val;
  DictEntry(const DictEntry& v) : key(v.key), val(v.val) {}
  DictEntry(const key_c& k, const val_c& v) : key(k), val(v) {}
  DictEntry(const key_c& k) : key(k) {}
  template <class T> DictEntry(const T& k) : key(k) {}
  DictEntry& operator = (const DictEntry& v) {
    key = v.key;
    val = v.val;
    return *this;
  }

  struct Comparator {
    ComparatorType cmp;
    Comparator() {}
    Comparator(const ComparatorType &cmp)
      : cmp(cmp)
    {}
    int Compare(const DictEntry& e1, const DictEntry& e2) const {
      return cmp.Compare(e1.key, e2.key);
    }
    template <class T>
    int Compare(const DictEntry& e1, const T& key) const {
      return cmp.Compare(e1.key, key);
    }
  };
};

template <typename KType, typename VType, class Comparator> class olxdict
  : protected SortedObjectList
      <DictEntry<KType,VType,Comparator>,
      typename DictEntry<KType, VType, Comparator>::Comparator>
{
  typedef DictEntry<KType,VType,Comparator> EntryType;
  typedef typename EntryType::Comparator cmpt_t;
  typedef SortedObjectList<EntryType, typename EntryType::Comparator> SortedL;
public:
  struct Entry  {
    KType key;
    VType value;
    Entry(const KType& _key, const VType& _value)
      : key(_key), value(_value)
    {}
  };

  olxdict() {}

  olxdict(const Comparator &cmp)
    : SortedL(cmpt_t(cmp))
  {}
  olxdict(const Entry _values[], size_t cnt, const Comparator &cmp)
    : SortedL(cmpt_t(cmp))
  {
    for( size_t i=0; i < cnt; i++ )
      Add(_values[i].key, _values[i].value);
  }
  olxdict(const Entry _values[], size_t cnt) {
    for (size_t i = 0; i < cnt; i++)
      Add(_values[i].key, _values[i].value);
  }
  olxdict(const olxdict& ad) : SortedL(ad) {}
  olxdict(const const_olxdict<KType,VType,Comparator>& ad)
    : SortedL(ad.GetObject().cmp)
  {
    SortedL::TakeOver(ad.Release(), true);
  }
  void TakeOver(olxdict &d)  {  SortedL::TakeOver(d);  }
  olxdict& operator = (const olxdict& ad)  {
    SortedL::operator = (ad);
    return *this;
  }
  olxdict& operator = (const const_olxdict<KType,VType,Comparator>& ad)  {
    SortedL::TakeOver(ad.Release(), true);
    return *this;
  }
  template <class T> VType& Get(const T& key) const {
    size_t ind = SortedL::IndexOf(key);
    if (ind == InvalidIndex)
      throw TInvalidArgumentException(__OlxSourceInfo, "key");
    return SortedL::operator[] (ind).val;
  }
  template <class T> VType& operator [] (const T& key) const {
    return Get(key);
  }
  template <class T> const VType& Find(const T& key, const VType& def) const {
    size_t ind = SortedL::IndexOf(key);
    if( ind == InvalidIndex )
      return def;
    return SortedL::operator[] (ind).val;
  }
  void Clear()  {  SortedL::Clear();  }
  size_t Count() const {  return SortedL::Count();  }
  bool IsEmpty() const {  return SortedL::IsEmpty();  }
  void SetCapacity(size_t c)  { SortedL::SetCapacity(c); }
  void SetIncrement(size_t inc) { SortedL::SetIncrement(inc);  }
  VType& GetValue(size_t ind)  {  return SortedL::operator[] (ind).val;  }
  const VType& GetValue(size_t ind) const {
    return SortedL::operator[] (ind).val;
  }
  const KType& GetKey(size_t ind) const {
    return SortedL::operator[] (ind).key;
  }
  const EntryType& GetEntry(size_t ind) const {
    return SortedL::operator[] (ind);
  }
  template <class T> bool HasKey(const T& key) const {
    return SortedL::IndexOf(key) != InvalidIndex;
  }

  template <typename T> VType& operator () (const T& key, const VType& def) {
    return Add(key, def);
  }
  template <typename T> VType& Add(const T& key, const VType& def, bool update=false) {
    olx_pair_t<size_t, bool> ip = SortedL::AddUnique(key);
    if (ip.b || update) // new entry?
      SortedL::operator[] (ip.a).val = def;
    return SortedL::operator[] (ip.a).val;
  }
  template <typename T> VType& Add(const T& key) {
    olx_pair_t<size_t, bool> ip = SortedL::AddUnique(key);
    return SortedL::operator[] (ip.a).val;
  }
  template <class T> size_t IndexOf(const T& key) const {
    return SortedL::IndexOf(key);
  }
  template <class T> size_t IndexOfValue(const T& val) const {
    for( size_t i=0; i < SortedL::Count(); i++ )
      if( SortedL::operator [] (i).val == val )
        return i;
    return InvalidIndex;
  }
  template <class T> bool Remove(const T& key)  {
    size_t ind = SortedL::IndexOf(key);
    if (ind != InvalidIndex) {
      SortedL::Delete(ind);
      return true;
    }
    return false;
  }
  void Delete(size_t ind)  {  SortedL::Delete(ind);  }

  void Merge(const olxdict &d, bool replace=true) {
    for (size_t i = 0; i < d.Count(); i++) {
      size_t idx = IndexOf(d.GetKey(i));
      if (idx == InvalidIndex) {
        Add(d.GetKey(i), d.GetValue(i));
      }
      else if (replace) {
        GetValue(idx) = d.GetValue(idx);
      }
    }
  }
public:
  typedef KType key_item_type;
  typedef VType value_item_type;
  typedef const_olxdict<KType, VType, Comparator> const_dict_type;
};

// a string to type association....
template <typename VType, bool case_insensitive=false>
class olxstr_dict
  : public olxdict<olxstr, VType, olxstrComparator<case_insensitive> >
{
  typedef olxdict < olxstr, VType, olxstrComparator<case_insensitive> >
    parent_t;
public:
  olxstr_dict()
    : parent_t(olxstrComparator<case_insensitive>())
  {}
};

// const_dict
template <typename key_t, typename val_t, class Comparator>
class const_olxdict : public const_dict<
  olxdict<key_t,val_t,Comparator> >
{
  typedef olxdict<key_t,val_t,Comparator> dict_t;
  typedef const_dict<dict_t> parent_t;
public:
  const_olxdict(const const_olxdict &d) : parent_t(d) {}
  const_olxdict(dict_t &d) : parent_t(d) {}
  const_olxdict(dict_t *d) : parent_t(d) {}
  const_olxdict &operator = (const const_olxdict &d) {
    parent_t::operator = (d);
    return *this;
  }
};

EndEsdlNamespace()
#endif
