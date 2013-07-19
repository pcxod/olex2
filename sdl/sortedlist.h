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

template <typename, typename> class ConstSortedObjectList;
template <typename, typename> class ConstSortedPtrList;
// generic sorted list
template <class ListClass, class Comparator, typename TypeClass>
class TTSortedListBase {
protected:
  ListClass list;
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
  template <class KeyC>
  bool Contains(const KeyC& entity) const {
    return IndexOf(entity) != InvalidIndex;
  }
  /* removes specified entry from the list and returns true if the entry was
  in the list
  */
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
  template <class PackAnalyser>
  void Pack(const PackAnalyser& pa)  {  list.Pack(pa);  }
  template <class PackAnalyser>
  void PackEx(const PackAnalyser& pa)  {  list.PackEx(pa);  }
  template <class Functor>
  void ForEach(const Functor& f) const {  list.ForEach(f);  }
  template <class Functor>
  void ForEachEx(const Functor& f) const {  list.ForEachEx(f);  }
};

template <class ListClass, class Comparator, typename TypeClass>
class TTSortedList
  : public TTSortedListBase<ListClass, Comparator, TypeClass>
{
  typedef TTSortedListBase<ListClass, Comparator, TypeClass> __parent_t;
public:
  TTSortedList() {}
  TTSortedList(const TTSortedList& l) : __parent_t(l)  {}
  // adds an item to the list and returns it's index
  size_t Add(const TypeClass& entry)  {
    return sorted::Add(__parent_t::list, Comparator(), entry);
  }
  /* adds an item only if not already in the list, returns true if the item is
  added, pos is is initialised with the item index
  */
  bool AddUnique(const TypeClass& entry, size_t* pos = NULL)  {
    return sorted::AddUnique(__parent_t::list, Comparator(), entry, pos);
  }
  TTSortedList& operator = (const TTSortedList& _list)  {
    __parent_t::operator = (_list);
    return *this;
  }
};

template <typename TypeClass, class Comparator>
class SortedTypeList
  : public TTSortedListBase<TTypeList<TypeClass>, Comparator, TypeClass> {
  typedef TTSortedListBase<TTypeList<TypeClass>, Comparator, TypeClass>
    _parent_t;
public:
  SortedTypeList() {}
  SortedTypeList(const SortedTypeList& l) : _parent_t(l)  {}
  // adds an item to the list and returns it's index
  size_t Add(TypeClass* entry)  {
    return sorted::Add(_parent_t::list, Comparator(), *entry);
  }
  size_t Add(TypeClass& entry)  {
    return sorted::Add(_parent_t::list, Comparator(), entry);
  }
  /* adds an item only if not already in the list, returns true if the item is
  added, pos is is initialised with the item index, if item is already in the
  list - it is deleted and the list will not be modified
  */
  bool AddUnique(TypeClass* entry, size_t* pos = NULL)  {
    return AddUnique(*entry, pos);
  }
  bool AddUnique(TypeClass& entry, size_t* pos = NULL)  {
    if( sorted::AddUnique(_parent_t::list, Comparator(), entry, pos) )
      return true;
    delete &entry;
    return false;
  }
  SortedTypeList& operator = (const SortedTypeList& _list)  {
    _parent_t::operator = (_list);
    return *this;
  }
};
//.............................................................................
// a simple object list to use with sorted list
//.............................................................................
template <class ObjectClass> class TObjectList {
  TPtrList<ObjectClass> list;
public:
  TObjectList() {}
  TObjectList(const TObjectList& li) {
    SetCapacity(li.Count());
    for( size_t i=0; i < li.Count(); i++ )
      Add(li[i]);
  }
  ~TObjectList()  {  list.DeleteItems();  }
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
    list.DeleteItems();
    list.Clear();
  }
  void Delete(size_t ind)  {
    if( list[ind] != NULL )
      delete list[ind];
    list.Delete(ind);
  }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
public:
  typedef ObjectClass list_item_type;
};
//.............................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for objects having < and > operators only
  TComparableComparator - for objects having Compare method returning -,+,0
*/
template <class ObjectClass, class Comparator>
class SortedObjectList
  : public TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass> {
  typedef TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>
    _parent_t;
public:
  SortedObjectList() {}
  SortedObjectList(const SortedObjectList& l) :
    TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>(l) {}
  SortedObjectList(const ConstSortedObjectList<ObjectClass,Comparator>& l)  {
    _parent_t::TakeOver(l.Release(), true);
  }
  SortedObjectList& operator = (const SortedObjectList& l)  {
    _parent_t::operator = (l);
    return *this;
  }
  SortedObjectList& operator = (
    const ConstSortedObjectList<ObjectClass,Comparator>& l)
  {
    _parent_t::TakeOver(l.Release, true);
    return *this;
  }
  template <class LT> static
    ConstSortedObjectList<ObjectClass, Comparator> FromList(const LT &l)
  {
    SortedObjectList rv;
    rv.SetCapacity(l.Count());
    for (size_t i=0; i < l.Count(); i++) rv.Add(l[i]);
    return rv;
  }
  template <class LT, class accessor_t> static
    ConstSortedObjectList<ObjectClass, Comparator> FromList(const LT &l,
    const accessor_t &accessor)
  {
    SortedObjectList rv;
    rv.SetCapacity(l.Count());
    for (size_t i=0; i < l.Count(); i++) rv.Add(accessor(l[i]));
    return rv;
  }
  static ConstSortedObjectList<ObjectClass, Comparator>
    FromArray(const ObjectClass *a, size_t sz)
  {
    SortedObjectList rv;
    rv.SetCapacity(sz);
    for (size_t i=0; i < sz; i++) rv.Add(a[i]);
    return rv;
  }
public:
  typedef ObjectClass list_item_type;
};
//.............................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for sorting Objects 
  TComparableComparator - for sorting objects having Compare method returning
  -,+,0
  TPointerComparator - for sorting pointer adresses
*/
template <class ObjectClass, class Comparator>
class SortedPtrList
  : public TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*> {
  typedef TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>
    _parent_t;
public:
  SortedPtrList() {}
  SortedPtrList(const SortedPtrList& l) :
    TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>(l) {}
  SortedPtrList(const ConstSortedPtrList<ObjectClass,Comparator>& l)  {
    _parent_t::TakeOver(l.Release(), true);
  }
  SortedPtrList& operator = (const SortedPtrList& l)  {
    _parent_t::operator = (l);
    return *this;
  }
  SortedPtrList& operator = (
    const ConstSortedPtrList<ObjectClass,Comparator>& l)
  {
    _parent_t::TakeOver(l.Release(), true);
    return *this;
  }
  template <class LT> static
    ConstSortedPtrList<ObjectClass, Comparator> FromList(const LT &l)
  {
    SortedPtrList rv;
    rv.SetCapacity(l.Count());
    for (size_t i=0; i < l.Count(); i++) rv.Add(l[i]);
    return rv;
  }
  template <class LT, class accessor_t> static
    ConstSortedPtrList<ObjectClass, Comparator> FromList(const LT &l,
    const accessor_t &accessor)
  {
    SortedPtrList rv;
    rv.SetCapacity(l.Count());
    for (size_t i=0; i < l.Count(); i++) rv.Add(accessor(l[i]));
    return rv;
  }
public:
  typedef ObjectClass *list_item_type;
};

// ConstSortedObjectList
template <typename obj_t, class Comparator>
class ConstSortedObjectList
  : public const_list<SortedObjectList<obj_t,Comparator> >
{
  typedef SortedObjectList<obj_t,Comparator> list_t;
  typedef const_list<list_t> parent_t;
public:
  ConstSortedObjectList(const ConstSortedObjectList &l) : parent_t(l) {}
  ConstSortedObjectList(list_t &l) : parent_t(l) {}
  ConstSortedObjectList(list_t *l) : parent_t(l) {}
  ConstSortedObjectList &operator = (const ConstSortedObjectList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef obj_t list_item_type;
};

// ConstSortedPtrList
template <typename obj_t, class Comparator>
class ConstSortedPtrList
  : public const_list<SortedPtrList<obj_t,Comparator> >
{
  typedef SortedPtrList<obj_t,Comparator> list_t;
  typedef const_list<list_t> parent_t;
public:
  ConstSortedPtrList(const ConstSortedPtrList &l) : parent_t(l) {}
  ConstSortedPtrList(list_t &l) : parent_t(l) {}
  ConstSortedPtrList(list_t *l) : parent_t(l) {}
  ConstSortedPtrList &operator = (const ConstSortedPtrList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef obj_t list_item_type;
};

EndEsdlNamespace()
#endif
