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

template <class ComparableClass, class ObjectClass, class Comparator>
struct TSortedListEntry {
  ComparableClass Comparable;
  mutable ObjectClass Object;
  TSortedListEntry(const ComparableClass& c, const ObjectClass& o) :
  Comparable(c), 
    Object(o) {}
  TSortedListEntry(const TSortedListEntry& entry)
    : Comparable(entry.Comparable), Object(entry.Object)  {}
  virtual ~TSortedListEntry()  {}
  TSortedListEntry& operator = (const TSortedListEntry& entry)  {
    Comparable = entry.Comparable;
    Object = entry.Object;
    return *this;
  }
  int Compare(const TSortedListEntry& entry) const {
    return Comparator::Compare(Comparable, entry.Comparable);  
  }
  template <class T> int Compare(const T& key) const {
    return Comparator::Compare(Comparable, key);
  }
};
//..............................................................................
template <class A, class B, class ComparatorType>
class TSTypeList : public IEObject  {
  // not an ArrayList - inserts are too 'heavy' 
  typedef TSortedListEntry<A,B,ComparatorType> EntryType;
  TPtrList<EntryType> Data;
protected:
  template <class Analyser> struct PackItemActor  {
    const Analyser& analyser;
    PackItemActor(const Analyser& _analyser) : analyser(_analyser)  {}
    bool OnItem(EntryType* o, size_t i) const {
      if( analyser.OnItem(o, i) )  {
        delete o;
        return true;
      }
      return false;
    }
  };
  template <class T>
  size_t FindInsertIndex(const T& key)  {
    return sorted::FindInsertIndex(Data, TComparableComparator(), key);
  }
  template <class T> size_t FindIndexOf(const T& key) const {
    return sorted::FindIndexOf(Data, TComparableComparator(), key);
  }
//..............................................................................
public:
  TSTypeList()  {}
//..............................................................................
  /* copy constructor */
  TSTypeList(const TSTypeList& list)  {
    Data.SetCount(list.Count());
    for( size_t i=0; i < list.Data.Count(); i++ )
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
  size_t IndexOfObject(const B& v) const {
    for( size_t i=0; i < Data.Count(); i++ )
      if( Data[i]->Object == v )  
        return i;
    return InvalidIndex;
  }
//..............................................................................
  // retrieves indexes of all entries with same key and returns the number
  // of added entries
  template <class T, class size_t_list_t>
  size_t GetIndices(const T& key, size_t_list_t& il)  {
    if( Data.IsEmpty() )  return 0;
    if( Data.Count() == 1 )  {
      if( Data[0]->Compare(key) != 0 )  return 0;
      il.Add(0);
      return 1;
    }
    const size_t index =  IndexOf(key);
    if( index == InvalidIndex )  return 0;
    il.Add(index);
    size_t i = index+1, addedc = 1;
    // go forward
    while( i < Data.Count() && (Data[i]->Compare(key) == 0) )  {
      il.Add(i);
      i++;
      addedc++;
    }
    // go backwards
    if( index == 0 )  return addedc;
    i = index-1;
    while( i > 0 && (Data[i]->Compare(key) == 0) )  {
      il.Add(i);
      addedc++;
      if( i == 0 )  break;
      i--;
    }
    return addedc;
  }
//..............................................................................
  bool IsNull(size_t index) const {  return Data[index] == NULL;  }
//..............................................................................
  void NullItem(size_t i)  {
    if( Data[i] != NULL )  {
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
    return Data[index]->Comparable;
  }
//..............................................................................
  B& GetObject(size_t index) const {  return Data[index]->Object;  }
//..............................................................................
  const EntryType& GetLast() const {  return *Data.GetLast();  }
//..............................................................................
  const A& GetLastKey() const {  return Data.GetLast()->Comparable;  }
//..............................................................................
  const B& GetLastObject() const {  return Data.GetLast()->Object;  }
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
  template <class T> B& operator [] (const T& key) const {
    const size_t ind = IndexOf(key);
    if( ind != InvalidIndex )
      return Data[ind]->Object;
    throw TFunctionFailedException(__OlxSourceInfo,
      "no object at specified location");
  }
//..............................................................................
  const EntryType& Add(const A& key, const B& Object)  {
    EntryType *entry = new EntryType(key, Object);
    if( Data.IsEmpty() )
      Data.Add( entry);
    else if( Data.Count() == 1 )  {
      if(  Data[0]->Compare(*entry) < 0 )  
        Data.Add(entry);
      else                                 
        Data.Insert(0, entry);
    }
    else  {
      if( Data[0]->Compare(*entry) >= 0 ) // smaller than the first
        Data.Insert(0, entry);
      else if( Data.GetLast()->Compare(*entry) <= 0 ) // larger than the last 
        Data.Add(entry);  
      else if( Data.Count() == 2 ) // an easy case then with two items 
        Data.Insert(1, entry);
      else  {
        const size_t pos = FindInsertIndex(key);
        if( pos == InvalidIndex )  {
          delete entry;
          throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
        }
        Data.Insert(pos, entry);
      }
    }
    return *entry;
  }
//..............................................................................
  TSTypeList &Merge(const TSTypeList &l) {
    for (size_t i=0; i < l.Count(); i++) {
      size_t idx = IndexOf(l.GetKey(i));
      if (idx == InvalidIndex)
        Add(l.GetKey(i), l.GetObject(i));
      else
        Data[idx]->Object = l.GetObject(i);
    }
    return *this;
  }
};
// partial specialisation
//..............................................................................
  // to be used with objects, having Compare operator
template <typename ComparableClass, typename ObjectClass>
 class TCSTypeList
   : public TSTypeList<ComparableClass, ObjectClass, TComparableComparator>
{
  typedef TSTypeList<ComparableClass, ObjectClass, TComparableComparator>
    parent_t;
public:
  TCSTypeList() {}
  TCSTypeList(const TCSTypeList& l) : parent_t(l) {}
  TCSTypeList &operator = (const TCSTypeList& l) {
    parent_t::operator = (l);
    return *this;
  }
};
//..............................................................................
// to be used with objects, having >, < operators, or primitive types
template <typename ComparableClass, typename ObjectClass>
class TPSTypeList
  : public TSTypeList<ComparableClass, ObjectClass, TPrimitiveComparator>
{
  typedef TSTypeList<ComparableClass, ObjectClass, TPrimitiveComparator>
    parent_t;
public:
  TPSTypeList() {}
  TPSTypeList(const TPSTypeList& l) : parent_t(l) {}
  TPSTypeList &operator = (const TPSTypeList& l) {
    parent_t::operator = (l);
    return *this;
  }
};
//..............................................................................
// string specialisation ... special overriding for [] operator - returns NULL
// if no specified comparable exist, beware it returns '0' for primitive types
template <class SC, typename ObjectClass, bool caseinsensitive>
class TSStrPObjList
  : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >
{
  typedef TSTypeList<SC,ObjectClass, olxstrComparator<caseinsensitive> >
    PList;
  typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> >
    PListEntry;
public:
  TSStrPObjList() {}
  TSStrPObjList(const TSStrPObjList& l) : PList(l) {}
  TSStrPObjList &operator = (const TSStrPObjList& l) {
    PList::operator = (l);
    return *this;
  }
  const olxstr& GetString(size_t i) const {  return PList::GetKey(i);  }
  const PListEntry& Add(const SC& s,
    const ObjectClass& v=*(ObjectClass*)NULL)
  {
    return PList::Add(s, v);
  }
  template <class T>
  ObjectClass operator [] (const T& key) const {
    const size_t ind = PList::IndexOf(key);
    return (ind != InvalidIndex)  ? PList::GetObject(ind) : NULL;
  }
};
//..............................................................................
// just a string to obj specialisation
template <class SC, typename ObjectClass, bool caseinsensitive>
class TSStrObjList
  : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >
{
  typedef TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >
    PList;
  typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> >
    PListEntry;
public:
  TSStrObjList() {}
  TSStrObjList(const TSStrObjList& l) : PList(l) {}
  TSStrObjList &operator = (const TSStrObjList& l) {
    PList::operator = (l);
    return *this;
  }
  const SC& GetString(size_t i) const {  return PList::GetKey(i);  }
  const PListEntry& Add(const SC& s,
    const ObjectClass& v = *(ObjectClass*)NULL )
  {
    return PList::Add(s, v);
  }
};
//..............................................................................
 // string - string map specialisation
template <class SC, bool caseinsensitive>
class TSStrStrList
  : public TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >  {
    typedef TSTypeList<SC, SC, olxstrComparator<caseinsensitive> > PList;
    typedef TSortedListEntry<SC,SC,olxstrComparator<caseinsensitive> >
      PListEntry;
public:
  TSStrStrList() {}
  TSStrStrList(const TSStrStrList& l) : PList(l) {}
  TSStrStrList &operator = (const TSStrStrList& l) {
    PList::operator = (l);
    return *this;
  }
  const SC& GetString(size_t i) const {  return PList::GetKey(i);  }
  template <class T>
  const PListEntry& Add(const T& key, const SC& v=EmptyString())  {
    return PList::Add(key, v);
  }
};

EndEsdlNamespace()
#endif
