/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
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
#include "esort.h"
BeginEsdlNamespace()

template <typename, typename> class ConstSortedObjectList;
template <typename, typename> class ConstSortedPointerList;
// generic sorted list
template <class ListClass, class Comparator, typename TypeClass>
class TTSortedListBase {
protected:
  ListClass list;
  Comparator cmp;
public:
  TTSortedListBase() {}
  TTSortedListBase(const Comparator &cmp)
    : cmp(cmp)
  {}
  TTSortedListBase(const olx_capacity_t &cap)
    : list(cap)
  {}
  TTSortedListBase(const Comparator& cmp, const olx_capacity_t& cap)
    : list(cap), cmp(cmp)
  {}
  TTSortedListBase(const TTSortedListBase& l)
    : list(l.list), cmp(l.cmp)
  {}
  ~TTSortedListBase() {
  }
  void TakeOver(TTSortedListBase &l, bool do_delete=false)  {
    list.TakeOver(l.list);
    if (do_delete) {
      delete& l;
    }
  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  size_t Count() const {  return list.Count();  }
  const TypeClass& operator [] (size_t i) const {  return list[i];  }
  TTSortedListBase& operator = (const TTSortedListBase& _list)  {
    list = _list.list;
    return *this;
  }
  template <class KeyC>
  size_t IndexOf(const KeyC& entity) const {
    return sorted::FindIndexOf(list, cmp, entity);
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
    if (ind == InvalidIndex) {
      return false;
    }
    list.Delete(ind);
    return true;
  }
  void Delete(size_t ind)  {  list.Delete(ind);  }
  void Clear()  {  list.Clear();  }
  void SetCapacity(const olx_capacity_t &cap) { list.SetCapacity(cap); }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
  // may be useful for copy constructors, etc
  const ListClass& GetList() const {  return list;  }
  // allows to remove multiple items using a condition
  template <class PackAnalyser>
  void Pack(const PackAnalyser& pa)  {  list.Pack(pa);  }
  template <class Functor>
  void ForEach(const Functor& f) const {  list.ForEach(f);  }
  template <class Analyser> size_t Count(const Analyser& a) const {
    return list.Count(a);
  }
  //..............................................................................
};

template <class ListClass, class Comparator, typename TypeClass>
class TTSortedList
  : public TTSortedListBase<ListClass, Comparator, TypeClass>
{
  typedef TTSortedListBase<ListClass, Comparator, TypeClass> __parent_t;
public:
  TTSortedList() {}
  TTSortedList(const Comparator &cmp)
    : __parent_t(cmp)
  {}
  TTSortedList(const olx_capacity_t& cap)
    : __parent_t(cap)
  {}
  TTSortedList(const Comparator& cmp, const olx_capacity_t& cap)
    : __parent_t(cmp, cap)
  {}
  TTSortedList(const TTSortedList& l)
    : __parent_t(l)
  {}
  // adds an item to the list and returns it's index
  size_t Add(const TypeClass& entry)  {
    return sorted::Add(__parent_t::list, __parent_t::cmp, entry);
  }
  /* adds an item only if not already in the list, returns index of the item
  and if the new one was added
  */
  olx_pair_t<size_t, bool> AddUnique(const TypeClass& entry) {
    return sorted::AddUnique(__parent_t::list, __parent_t::cmp, entry);
  }
  TTSortedList& operator = (const TTSortedList& _list) {
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
  SortedTypeList(const Comparator & cmp)
    : _parent_t(cmp)
  {}
  SortedTypeList(const olx_capacity_t& cap)
    :_parent_t(cap)
  {}
  SortedTypeList(const Comparator& cmp, const olx_capacity_t& cap)
    : _parent_t(cmp, cap)
  {}
  SortedTypeList(const SortedTypeList& l)
    : _parent_t(l)
  {}
  // adds an item to the list and returns it's index
  size_t Add(TypeClass* entry)  {
    return sorted::Add(_parent_t::list, _parent_t::cmp, *entry);
  }
  size_t Add(TypeClass& entry)  {
    return sorted::Add(_parent_t::list, _parent_t::cmp, entry);
  }
  /* adds an item only if not already in the list, returns true if the item is
  added, pos is is initialised with the item index, if item is already in the
  list - it is deleted and the list will not be modified
  */
  bool AddUnique(TypeClass* entry, size_t* pos = 0) {
    return AddUnique(*entry, pos);
  }
  bool AddUnique(TypeClass& entry, size_t* pos = 0) {
    if (sorted::AddUnique(_parent_t::list, _parent_t::cmp, entry, pos)) {
      return true;
    }
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
  TObjectList(const olx_capacity_t &cap)
    : list(cap)
  {}
  TObjectList(const TObjectList& li) {
    SetCapacity(li.Count());
    for (size_t i = 0; i < li.Count(); i++) {
      Add(li[i]);
    }
  }
  ~TObjectList()  {  list.DeleteItems();  }
  void TakeOver(TObjectList &l, bool do_delete=false)  {
    list.TakeOver(l.list);
    if (do_delete) {
      delete& l;
    }
  }
  ObjectClass& operator [] (size_t i) const {  return *list[i];  }
  ObjectClass& GetLast() const {  return *list.GetLast();  }
  size_t Count() const {  return list.Count();  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  TObjectList& operator = (const TObjectList<ObjectClass>& li) {
    Clear();
    SetCapacity(li.Count());
    for (size_t i = 0; i < li.Count(); i++) {
      Add(li[i]);
    }
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
    olx_del_obj(list[ind]);
    list.Delete(ind);
  }
  void SetCapacity(const olx_capacity_t &cap) { list.SetCapacity(cap); }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
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
  SortedObjectList(const Comparator &cmp) :
    _parent_t(cmp)
  {}
  SortedObjectList(const olx_capacity_t& cap)
  : _parent_t(cap)
  {}
  SortedObjectList(const Comparator& cmp, const olx_capacity_t& cap) :
    _parent_t(cmp, cap)
  {}
  SortedObjectList(const SortedObjectList& l)
    : _parent_t(l)
  {}
  SortedObjectList(const ConstSortedObjectList<ObjectClass,Comparator>& l)
    : _parent_t(l.obj().cmp)
  {
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
    for (size_t i = 0; i < sz; i++) {
      rv.Add(a[i]);
    }
    return rv;
  }
public:
  typedef ObjectClass list_item_type;
  typedef ConstSortedObjectList<ObjectClass, Comparator> cons_list_type;
};
//.............................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for sorting Objects
  TComparableComparator - for sorting objects having Compare method returning
  -,+,0
  TPointerComparator - for sorting pointer adresses
*/
template <class ObjectClass, class Comparator>
class SortedPointerList
  : public TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*> {
  typedef TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>
    _parent_t;
public:
  SortedPointerList() {}
  SortedPointerList(const Comparator &cmp)
    : _parent_t(cmp)
  {}
  SortedPointerList(const SortedPointerList& l)
    : _parent_t(l)
  {}
  SortedPointerList(const ConstSortedPointerList<ObjectClass,Comparator>& l)
  : _parent_t(l.obj().cmp)
  {
    _parent_t::TakeOver(l.Release(), true);
  }
  SortedPointerList& operator = (const SortedPointerList& l)  {
    _parent_t::operator = (l);
    return *this;
  }
  SortedPointerList& operator = (
    const ConstSortedPointerList<ObjectClass,Comparator>& l)
  {
    _parent_t::TakeOver(l.Release(), true);
    return *this;
  }
  template <class LT> static
    ConstSortedPointerList<ObjectClass, Comparator> FromList(const LT &l)
  {
    SortedPointerList rv;
    rv.SetCapacity(l.Count());
    for (size_t i = 0; i < l.Count(); i++) {
      rv.Add(l[i]);
    }
    return rv;
  }
  template <class LT, class accessor_t> static
    ConstSortedPointerList<ObjectClass, Comparator> FromList(const LT &l,
    const accessor_t &accessor, const Comparator &cmp)
  {
    SortedPointerList rv(cmp);
    rv.SetCapacity(l.Count());
    for (size_t i = 0; i < l.Count(); i++) {
      rv.Add(accessor(l[i]));
    }
    return rv;
  }
  template <class LT, class accessor_t> static
    ConstSortedPointerList<ObjectClass, Comparator> FromList(const LT &l,
    const accessor_t &accessor)
  {
      SortedPointerList rv;
      rv.SetCapacity(l.Count());
      for (size_t i = 0; i < l.Count(); i++) {
        rv.Add(accessor(l[i]));
      }
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

// ConstSortedPointerList
template <typename obj_t, class Comparator>
class ConstSortedPointerList
  : public const_list<SortedPointerList<obj_t,Comparator> >
{
  typedef SortedPointerList<obj_t,Comparator> list_t;
  typedef const_list<list_t> parent_t;
public:
  ConstSortedPointerList(const ConstSortedPointerList &l) : parent_t(l) {}
  ConstSortedPointerList(list_t &l) : parent_t(l) {}
  ConstSortedPointerList(list_t *l) : parent_t(l) {}
  ConstSortedPointerList &operator = (const ConstSortedPointerList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef obj_t list_item_type;
};

namespace sorted {
  template <typename obj_t>
  class ObjectPrimitive : public SortedObjectList<obj_t, TPrimitiveComparator>
  {
    typedef SortedObjectList<obj_t, TPrimitiveComparator> parent_t;
  public:
    ObjectPrimitive() {}
    ObjectPrimitive(const ObjectPrimitive &l)
      : parent_t(l)
    {}
    ObjectPrimitive(const ConstSortedObjectList<obj_t, TPrimitiveComparator> &l)
      : parent_t(l)
    {}
    ObjectPrimitive & operator = (const ObjectPrimitive &l) {
      parent_t::operator = (l);
      return *this;
    }
  };
  template <typename obj_t>
  class ObjectComparable : public SortedObjectList<obj_t, TComparableComparator>
  {
    typedef SortedObjectList<obj_t, TComparableComparator> parent_t;
  public:
    ObjectComparable() {}
    ObjectComparable(const ConstSortedObjectList<obj_t, TComparableComparator> &l)
      : parent_t(l)
    {}
    ObjectComparable(const ObjectComparable &l) : parent_t(l) {}
    ObjectComparable & operator = (const ObjectComparable &l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename ptr_t>
  class PointerPointer : public SortedPointerList<ptr_t, TPointerComparator>
  {
    typedef SortedPointerList<ptr_t, TPointerComparator> parent_t;
  public:
    PointerPointer() {}
    PointerPointer(const PointerPointer &l) : parent_t(l) {}
    PointerPointer(const ConstSortedPointerList<ptr_t, TPointerComparator> &l)
      : parent_t(l)
    {}
    PointerPointer & operator = (const PointerPointer &l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename ptr_t>
  class PointerComparable : public SortedPointerList<ptr_t, TComparableComparator>
  {
    typedef SortedPointerList<ptr_t, TComparableComparator> parent_t;
  public:
    PointerComparable() {}
    PointerComparable(const PointerComparable &l) : parent_t(l) {}
    PointerComparable(const ConstSortedPointerList<ptr_t, TComparableComparator> &l)
      : parent_t(l)
    {}
    PointerComparable & operator = (const PointerComparable &l) {
      parent_t::operator = (l);
      return *this;
    }
  };

  template <typename ptr_t>
  class PointerPrimitive : public SortedPointerList<ptr_t, TPrimitiveComparator>
  {
    typedef SortedPointerList<ptr_t, TPrimitiveComparator> parent_t;
  public:
    PointerPrimitive() {}
    PointerPrimitive(const PointerPrimitive &l) : parent_t(l) {}
    PointerPrimitive(const ConstSortedPointerList<ptr_t, TPrimitiveComparator> &l)
      : parent_t(l)
    {}
    PointerPrimitive & operator = (const PointerPrimitive &l) {
      parent_t::operator = (l);
      return *this;
    }
  };

}

EndEsdlNamespace()
#endif
