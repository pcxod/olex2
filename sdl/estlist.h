/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_sortedTL_H
#define __olx_sdl_sortedTL_H
#include "tptrlist.h"
#include "sorted.h"
#undef GetObject

BeginEsdlNamespace()

template <class key_t, class val_t>
struct TSortedListEntry {
  const key_t Key;
  val_t Value;
  TSortedListEntry(const key_t& c, const val_t& o) :
    Key(c), Value(o)
  {}
  TSortedListEntry(const TSortedListEntry& entry)
    : Key(entry.Key), Value(entry.Value)
  {}
  virtual ~TSortedListEntry()  {}
  TSortedListEntry& operator = (const TSortedListEntry& entry)  {
    Key = entry.Key;
    Value = entry.Value;
    return *this;
  }
};
//..............................................................................
template <class A, class B, class ComparatorType>
class TSTypeList : public IEObject {
  // not an ArrayList - inserts are too 'heavy'
  typedef TSortedListEntry<A,B> EntryType;
  TPtrList<EntryType> Data;
  struct Comparator {
    ComparatorType cmp;
    Comparator() {}
    Comparator(const ComparatorType &cmp)
      : cmp(cmp)
    {}
    template <typename key_t>
    int Compare(const EntryType *e, const key_t &key) const {
      return cmp.Compare(e->Key, key);
    }
  };
  Comparator cmp;
protected:
  template <class Analyser> struct PackItemActor {
    const Analyser& analyser;
    PackItemActor(const Analyser& _analyser) : analyser(_analyser)  {}
    bool OnItem(EntryType* o, size_t i) const {
      if (analyser.OnItem(o, i)) {
        delete o;
        return true;
      }
      return false;
    }
  };
  template <class T>
  size_t FindInsertIndex(const T& key)  {
    return sorted::FindInsertIndex(Data, cmp, key);
  }
  template <class T> size_t FindIndexOf(const T& key) const {
    return sorted::FindIndexOf(Data, cmp, key);
  }
//..............................................................................
public:
  TSTypeList() {}
  TSTypeList(const ComparatorType &cmp) : cmp(cmp) {}
//..............................................................................
  /* copy constructor */
  TSTypeList(const TSTypeList& list) : cmp(list.cmp) {
    Data.SetCount(list.Count());
    for (size_t i=0; i < list.Data.Count(); i++)
      Data[i] = new EntryType(*list.Data[i]);
  }
//..............................................................................
  TSTypeList &operator = (const TSTypeList &l) {
    Data.DeleteItems();
    Data.SetCount(l.Count());
    for (size_t i=0; i < l.Count(); i++)
      Data[i] = new EntryType(*l.Data[i]);
    return *this; 
  }
//..............................................................................
  virtual ~TSTypeList()  {  Data.DeleteItems();  }
//..............................................................................
  void Clear()  {  Data.DeleteItems().Clear();  }
//..............................................................................
  template <class T> size_t IndexOf(const T& key) const {
    return FindIndexOf(key);
  }
//..............................................................................
  size_t IndexOfValue(const B& v) const {
    for (size_t i=0; i < Data.Count(); i++)
      if (Data[i]->Value == v)
        return i;
    return InvalidIndex;
  }
//..............................................................................
  // retrieves indexes of all entries with same key and returns the number
  // of added entries
  template <class T, class size_t_list_t>
  size_t GetIndices(const T& key, size_t_list_t& il)  {
    if (Data.IsEmpty())  return 0;
    if (Data.Count() == 1) {
      if (cmp.Compare(Data[0], key) != 0)
        return 0;
      il.Add(0);
      return 1;
    }
    const size_t index =  IndexOf(key);
    if (index == InvalidIndex)  return 0;
    il.Add(index);
    size_t i = index+1, addedc = il.Count()+1;
    // go forward
    while (i < Data.Count() && (cmp.Compare(Data[i], key) == 0)) {
      il.Add(i++);
    }
    // go backwards
    if (index != 0) {
      i = index - 1;
      while (i > 0 && (cmp.Compare(Data[i], key) == 0)) {
        il.Add(i);
        if (i == 0)  break;
        i--;
      }
    }
    return il.Count() - addedc;
  }
//..............................................................................
  bool IsNull(size_t index) const {  return Data[index] == NULL;  }
//..............................................................................
  void NullItem(size_t i) {
    if (Data[i] != NULL) {
      delete Data[i];
      Data[i] = NULL;
    }
  }
//..............................................................................
  TSTypeList& Pack()  {  Data.Pack();  return *this;  }
//..............................................................................
  template <class PackAnalyser> TSTypeList& Pack(const PackAnalyser& pa) {
    Data.Pack(PackItemActor<PackAnalyser>(pa));
    return *this;
  }
//..............................................................................
  template <class Functor> TSTypeList& ForEach(const Functor& f) const {
    Data.ForEach(f);
    return *this;
  }
//..............................................................................
  void Delete(size_t i)  {
    delete Data[i];
    Data.Delete(i);
  }
//..............................................................................
  size_t Count() const {  return Data.Count(); }
//..............................................................................
  bool IsEmpty() const {  return Data.IsEmpty();  }
//..............................................................................
  const A& GetKey(size_t index) const {
    return Data[index]->Key;
  }
//..............................................................................
  B& GetValue(size_t index) const {  return Data[index]->Value;  }
//..............................................................................
  EntryType& GetLast() const {  return *Data.GetLast();  }
//..............................................................................
  const A& GetLastKey() const {  return Data.GetLast()->Key;  }
//..............................................................................
  B& GetLastValue() const {  return Data.GetLast()->Value;  }
//..............................................................................
  TSTypeList& SetCapacity(size_t v)  {
    Data.SetCapacity(v);
    return *this;
  }
//..............................................................................
  TSTypeList& SetIncrement(size_t v)  {
    Data.SetIncrement(v);
    return *this;
  }
//..............................................................................
  template <class T> B& Find(const T& key) const {
    const size_t ind = IndexOf(key);
    if (ind != InvalidIndex)
      return Data[ind]->Value;
    throw TFunctionFailedException(__OlxSourceInfo,
      "no object at specified location");
  }
//..............................................................................
  template <class T> const B& Find(const T& key, const B& def) const {
    const size_t ind = IndexOf(key);
    if (ind != InvalidIndex)
      return Data[ind]->Value;
    return def;
  }
  //..............................................................................
  EntryType& Add(const A& key, const B& Value) {
    EntryType *entry = new EntryType(key, Value);
    if (Data.IsEmpty())
      Data.Add(entry);
    else if (Data.Count() == 1) {
      if (cmp.Compare(Data[0], key) < 0)
        Data.Add(entry);
      else
        Data.Insert(0, entry);
    }
    else {
      // smaller than the first
      if (cmp.Compare(Data[0], key) >= 0)
        Data.Insert(0, entry);
      // larger than the last
      else if (cmp.Compare(Data.GetLast(), key) <= 0)
        Data.Add(entry);
      else if (Data.Count() == 2) // an easy case then with two items
        Data.Insert(1, entry);
      else {
        const size_t pos = FindInsertIndex(key);
        if (pos == InvalidIndex ) {
          delete entry;
          throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
        }
        Data.Insert(pos, entry);
      }
    }
    return *entry;
  }
//..............................................................................
  TSTypeList &Merge(const TSTypeList &l, bool replace=true) {
    for (size_t i=0; i < l.Count(); i++) {
      size_t idx = IndexOf(l.GetKey(i));
      if (idx == InvalidIndex)
        Add(l.GetKey(i), l.GetValue(i));
      else if (replace)
        Data[idx]->Value = l.GetValue(i);
    }
    return *this;
  }
};

namespace sorted {
  template <typename key_t, typename obj_t>
  class PointerAssociation
    : public TSTypeList<key_t, obj_t, TPointerComparator>
  {
    typedef TSTypeList<key_t, obj_t, TPointerComparator> parent_t;
    typedef TSortedListEntry<key_t, obj_t> PListEntry;
  public:
    PointerAssociation() {}
    PointerAssociation(const PointerAssociation& l) : parent_t(l) {}
    PointerAssociation &operator = (const PointerAssociation& l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename key_t, typename obj_t>
  class PrimitiveAssociation
    : public TSTypeList<key_t, obj_t, TPrimitiveComparator>
  {
    typedef TSTypeList<key_t, obj_t, TPrimitiveComparator> parent_t;
    typedef TSortedListEntry<key_t, obj_t> PListEntry;
  public:
    PrimitiveAssociation() {}
    PrimitiveAssociation(const PrimitiveAssociation& l) : parent_t(l) {}
    PrimitiveAssociation &operator = (const PrimitiveAssociation& l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename key_t, typename obj_t>
  class ComparableAssociation
    : public TSTypeList<key_t, obj_t, TComparableComparator>
  {
    typedef TSTypeList<key_t, obj_t, TComparableComparator> parent_t;
  public:
    ComparableAssociation() {}
    ComparableAssociation(const ComparableAssociation& l) : parent_t(l) {}
    ComparableAssociation &operator = (const ComparableAssociation& l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename obj_t, bool caseinsensitive=false>
  class StringAssociation
    : public TSTypeList<olxstr, obj_t, olxstrComparator<caseinsensitive> >
  {
    typedef TSTypeList<olxstr, obj_t, olxstrComparator<caseinsensitive> >
      parent_t;
  public:
    StringAssociation() {}
    StringAssociation(const StringAssociation& l) : parent_t(l) {}
    StringAssociation &operator = (const StringAssociation& l) {
      parent_t::operator = (l);
      return *this;
    }
  };
}

EndEsdlNamespace()
#endif
