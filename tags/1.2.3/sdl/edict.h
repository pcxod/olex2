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

template <typename key_c, typename val_c, class Comparator> struct DictEntry {
  key_c key;
  mutable val_c val;
  int Compare(const DictEntry& sl) const {
    return Comparator::Compare(this->key, sl.key);
  }
  template <class T> int Compare(const T& key) const {
    return Comparator::Compare(this->key, key);
  }
  DictEntry(const DictEntry& v) : key(v.key), val(v.val) {}
  DictEntry(const key_c& k, const val_c& v) : key(k), val(v) {}
  DictEntry(const key_c& k) : key(k) {}
  template <class T> DictEntry(const T& k) : key(k) {}
  DictEntry& operator = (const DictEntry& v) {
    key = v.key;
    val = v.val;
    return *this;
  }
};

template <typename KType, typename VType, class Comparator> class olxdict
  : protected SortedObjectList
      <DictEntry<KType,VType,Comparator>, TComparableComparator>
{
  typedef DictEntry<KType,VType,Comparator> EntryType;
  typedef SortedObjectList<EntryType, TComparableComparator> SortedL;
public:

  struct Entry  {
    KType key;
    VType value;
    Entry(const KType& _key, const VType& _value)
      : key(_key), value(_value)
    {}
  };

  olxdict() {}
  olxdict(const Entry _values[], size_t cnt) {
    for( size_t i=0; i < cnt; i++ )
      Add(_values[i].key, _values[i].value);
  }
  olxdict(const olxdict& ad) : SortedL(ad) {}
  olxdict(const const_olxdict<KType,VType,Comparator>& ad) {
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
  template <class T> VType& operator [] (const T& key) const {
    size_t ind = SortedL::IndexOf(key);
    if( ind == InvalidIndex )
      throw TInvalidArgumentException(__OlxSourceInfo, "key");
    return SortedL::operator[] (ind).val;
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
  void SetCapacity(size_t c)  {  SortedL::SetCapacity(c);  }
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
  template <typename T> VType& Add(const T& key, const VType& def) {
    size_t ind = InvalidIndex;
    if( SortedL::AddUnique(key, &ind) ) // new entry?
      SortedL::operator[] (ind).val = def;
    return SortedL::operator[] (ind).val;
  }
  template <typename T> VType& Add(const T& key) {
    size_t ind = InvalidIndex;
    SortedL::AddUnique(key, &ind);
    return SortedL::operator[] (ind).val;
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
public:
  typedef KType key_item_type;
  typedef VType value_item_type;

};

// a string to type association....
template <typename VType, bool case_insensitive=false> class olxstr_dict
  : public olxdict<olxstr, VType, olxstrComparator<case_insensitive> > {};

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
