/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_sortedlist_H
#define __olx_sdl_sortedlist_H
#include "typelist.h"
#include "sorted.h"
BeginEsdlNamespace()

// generic sorted list
template <class ListClass, class Comparator, typename TypeClass> class TTSortedListBase {
protected:
  ListClass list;
  struct Proxy  {
    TypeClass& value;
    Proxy(TypeClass& val) : value(val) {}
    operator TypeClass& () {  return value;  }
    operator const TypeClass& () const {  return value;  }
    TypeClass& operator = (const TypeClass& v)  {
      throw TFunctionFailedException(__OlxSourceInfo, "cannot modify constant object");
    }
  };
  // to be used by dirived objcts if necessary
  Proxy GetProxyObject(int i)  {  return Proxy(list[i]);  }
public:
  TTSortedListBase() {}
  TTSortedListBase(const TTSortedListBase& l) : list(l.list)  {}
  void TakeOver(TTSortedListBase &l, bool do_delete=false)  {
    list.TakeOver(l.list);
    if( do_delete )
      delete &l;
  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  size_t Count() const {  return list.Count();  }
  const TypeClass& operator [] (size_t i)  const {  return list[i];  }
  TTSortedListBase& operator = (const TTSortedListBase& _list)  {
    list = _list.list;
    return *this;
  }
  template <class KeyC>
  size_t IndexOf(const KeyC& entity) const {
    return sorted::FindIndexOf(list, Comparator(), entity);
  }
  // removes specified entry from the list and returns true if the entry was in the list
  bool Remove(const TypeClass& entity)  {
    size_t ind = IndexOf(entity);
    if( ind == InvalidIndex )  return false;
    list.Delete(ind);
    return true;
  }
  void Delete(size_t ind)  {  list.Delete(ind);  }
  void Clear()  {  list.Clear();  }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
  // may be useful for copy constructors, etc
  const ListClass& GetList() const {  return list;  }
  // allows to remove multiple items using a condition
  template <class PackAnalyser> void Pack(const PackAnalyser& pa)  {  list.Pack(pa);  }
  template <class PackAnalyser> void PackEx(const PackAnalyser& pa)  {  list.PackEx(pa);  }
  template <class Functor> void ForEach(const Functor& f) const {  list.ForEach(f);  }
  template <class Functor> void ForEachEx(const Functor& f) const {  list.ForEachEx(f);  }
};

template <class ListClass, class Comparator, typename TypeClass>
class TTSortedList : public TTSortedListBase<ListClass, Comparator, TypeClass> {
  typedef TTSortedListBase<ListClass, Comparator, TypeClass> parent_t;
public:
  TTSortedList() {}
  TTSortedList(const TTSortedList& l) : parent_t(l)  {}
  // adds an item to the list and returns it's index
  size_t Add(const TypeClass& entry)  {
    return sorted::Add(parent_t::list, Comparator(), entry);
  }
  /* adds an item only if not already in the list, returns true if the item is added, pos is is 
  initialised with the item index */
  bool AddUnique(const TypeClass& entry, size_t* pos = NULL)  {
    return sorted::AddUnique(parent_t::list, Comparator(), entry, pos);
  }
  TTSortedList& operator = (const TTSortedList& _list)  {
    parent_t::operator = (_list);
    return *this;
  }
};

template <typename TypeClass, class Comparator>
class SortedTypeList
  : public TTSortedListBase<TTypeList<TypeClass>, Comparator, TypeClass> {
  typedef TTSortedListBase<TTypeList<TypeClass>, Comparator, TypeClass> parent_t;
public:
  SortedTypeList() {}
  SortedTypeList(const SortedTypeList& l) : parent_t(l)  {}
  // adds an item to the list and returns it's index
  size_t Add(TypeClass* entry)  {
    return sorted::Add(parent_t::list, Comparator(), *entry);
  }
  size_t Add(TypeClass& entry)  {
    return sorted::Add(parent_t::list, Comparator(), entry);
  }
  /* adds an item only if not already in the list, returns true if the item is added, pos is is 
  initialised with the item index, if item is already in the list - it is deleted and the list
  will not be modified */
  bool AddUnique(TypeClass* entry, size_t* pos = NULL)  {  return AddUnique(*entry, pos);  }
  bool AddUnique(TypeClass& entry, size_t* pos = NULL)  {
    if( sorted::AddUnique(parent_t::list, Comparator(), entry, pos) )
      return true;
    delete &entry;
    return false;
  }
  SortedTypeList& operator = (const SortedTypeList& _list)  {
    parent_t::operator = (_list);
    return *this;
  }
};
//............................................................................................
// a simple object list to use with sorted list
//............................................................................................
template <class ObjectClass> class TObjectList {
  TPtrList<ObjectClass> list;
public:
  TObjectList() {}
  TObjectList(const TObjectList& li) {  
    SetCapacity(li.Count());
    for( size_t i=0; i < li.Count(); i++ )
      Add(li[i]);
  }
  ~TObjectList() {  Clear();  }
  void TakeOver(TObjectList &l, bool do_delete=false)  {
    list.TakeOver(l.list);
    if( do_delete )
      delete &l;
  }
  ObjectClass& operator [] (size_t i) const {  return *list[i];  }
  ObjectClass& GetLast() const {  return *list.GetLast();  }
  size_t Count() const {  return list.Count();  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  TObjectList<ObjectClass>& operator = (const TObjectList<ObjectClass>& li)  {
    Clear();
    SetCapacity(li.Count());
    for( size_t i=0; i < li.Count(); i++ )
      Add(li[i]);
    return *this;
  }
  void Add(const ObjectClass& obj)  {  list.Add(new ObjectClass(obj));  }
  void Insert(size_t index, const ObjectClass& obj)  {
    list.Insert(index, new ObjectClass(obj));
  }
  void Clear()  {
    for( size_t i=0; i < list.Count(); i++ )
      delete list[i];
    list.Clear();
  }
  void Delete(size_t ind)  {
    if( list[ind] != NULL )
      delete list[ind];
    list.Delete(ind);
  }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
};
//............................................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for objects having < and > operators only
  TComparableComparator - for objects having Compare method returning -,+,0
*/
template <class ObjectClass, class Comparator> 
class SortedObjectList : public TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass> {
public:
  SortedObjectList() {}
  SortedObjectList(const SortedObjectList& l) : 
    TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>(l) {}
  SortedObjectList& operator = (const SortedObjectList& l)  {
    TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>::operator = (l);
    return *this;
  }
};
//............................................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for sorting Objects 
  TComparableComparator - for sorting objects having Compare method returning -,+,0
  TPointerComparator - for sorting pointer adresses
*/
template <class ObjectClass, class Comparator> 
class SortedPtrList : public TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*> {
public:
  SortedPtrList() {}
  SortedPtrList(const SortedPtrList& l) : 
    TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>(l) {}
  SortedPtrList& operator = (const SortedPtrList& l)  {
    TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>::operator = (l);
    return *this;
  }
};

EndEsdlNamespace()
#endif
