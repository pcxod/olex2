/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_array_list_H
#define __olx_sdl_array_list_H
#include "ebase.h"
#include "etraverse.h"
#include "exception.h"
#include "shared.h"
#include "constlist.h"
BeginEsdlNamespace()

template <typename> class SharedArrayList;
template <typename> class ConstArrayList;

template <class T> class TArrayList : public IOlxObject {
private:
  size_t FCount;
  olx_capacity_t cap;
  T* Items;
  void init(size_t size) {
    cap.capacity = FCount = size;
    if (cap.capacity != 0) {
      Items = new T[cap.capacity];
    }
    else {
      Items = 0;
    }
  }
public:
  // creates a new empty objects
  TArrayList() { init(0); }
  explicit TArrayList(const olx_capacity_t& cap)
    : cap(cap)
  {
    init(cap.capacity);
    FCount = 0;
  }

  // allocates size elements (can be accessed directly)
  TArrayList(size_t size) { init(size); }
  //..............................................................................
  template <class init_t>
  TArrayList(size_t size, const init_t& initialiser) {
    init(size);
    for (size_t i = 0; i < size; i++) {
      initialiser.OnItem(Items[i], i);
    }
  }
  //..............................................................................
    /* copy constuctor - creates new copies of the objest, be careful as the
    assignement operator must exist for nonpointer objects
    */
  TArrayList(const TArrayList& list) {
    init(list.Count());
    for (size_t i = 0; i < FCount; i++) {
      Items[i] = list.Items[i];
    }
  }
  //..............................................................................
  TArrayList(const SharedArrayList<T>& list) : Items(0) {
    TakeOver(list.Release(), true);
  }
  //..............................................................................
  TArrayList(const ConstArrayList<T>& list) : Items(0) {
    TakeOver(list.Release(), true);
  }
  //..............................................................................
    /* copies values from an array of size elements  */
  TArrayList(size_t size, const T* array) {
    init(size);
    for (size_t i = 0; i < FCount; i++) {
      Items[i] = array[i];
    }
  }
  //..............................................................................
    /* copies values from an array of size elements  */
  TArrayList(size_t size, T* array) {
    init(size);
    for (size_t i = 0; i < FCount; i++) {
      Items[i] = array[i];
    }
  }
  //..............................................................................
  template <class List, class Accessor>
  static ConstArrayList<T> FromList(const List& list,
    const Accessor& accessor)
  {
    TArrayList rv(list.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      rv[i] = accessor(list[i]);
    }
    return rv;
  }
  //..............................................................................
  template <class List>
  static ConstArrayList<T> FromList(const List& list) {
    TArrayList rv(list.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      rv[i] = list[i];
    }
    return rv;
  }
  //..............................................................................
    //destructor - beware t40: error: expec the objects are deleted!
  virtual ~TArrayList() {
    olx_del_arr(Items);
  }
  //..............................................................................
    //deletes the objects and clears the list
  void Clear() { SetCount(0); }
  //..............................................................................
  TArrayList& TakeOver(TArrayList& l, bool do_delete = false) {
    if (Items != 0) {
      delete[] Items;
    }
    FCount = l.FCount;
    cap = l.cap;
    Items = l.Items;
    l.FCount = l.cap.capacity = 0;
    l.Items = 0;
    if (do_delete) {
      delete& l;
    }
    return *this;
  }
  //..............................................................................
  virtual IOlxObject* Replicate() const { return new TArrayList(*this); }
  //..............................................................................
  template <class List> TArrayList& Assign(const List& list) {
    if ((const void*)this == (const void*)&list) {
      return *this;
    }
    SetCount(list.Count());
    for (size_t i = 0; i < FCount; i++) {
      Items[i] = list[i];
    }
    return *this;
  }
  //..............................................................................
  template <class List> TArrayList& operator = (const List& list) {
    return Assign(list);
  }
  //..............................................................................
  TArrayList& operator = (const SharedArrayList<T>& list) {
    return TakeOver(list.Release(), true);
  }
  //..............................................................................
  TArrayList& operator = (const ConstArrayList<T>& list) {
    return TakeOver(list.Release(), true);
  }
  //..............................................................................
  TArrayList& operator = (const TArrayList& list) {
    return Assign(list);
  }
  //..............................................................................
  template <class List> TArrayList& AddAll(const List& list) {
    SetCapacity(list.Count() + FCount);
    for (size_t i = 0; i < list.Count(); i++) {
      Items[FCount + i] = list[i];
    }
    FCount += list.Count();
    return *this;
  }
  //..............................................................................
  TArrayList& AddAll(const SharedArrayList<T>& list) {
    return AddAll(list.obj());
  }
  //..............................................................................
  TArrayList& AddAll(const ConstArrayList<T>& list) {
    return AddAll(list.obj());
  }
  //..............................................................................
  template <class List> TArrayList& operator += (const List& list) {
    return AddAll(list);
  }
  //..............................................................................
  T& Add(const T& Obj) {
    if (cap.capacity == FCount) {
      SetCapacity(cap.new_size(FCount));
    }
    return (Items[FCount++] = Obj);
  }
  //..............................................................................
  T& Add() {
    if (cap.capacity == FCount) {
      SetCapacity(cap.new_size(FCount));
    }
    return Items[FCount++];
  }
  //..............................................................................
  TArrayList& operator << (const T& o) { Add(o);  return *this; }
  //..............................................................................
  TArrayList& operator << (const TArrayList& l) { return AddAll(l); }
  //..............................................................................
  T& Insert(size_t index, const T& Obj) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, index, 0, FCount + 1);
#endif
    if (cap.capacity == FCount) {
      SetCapacity(cap.new_size(FCount));
    }
    const size_t diff = FCount - index;
    for (size_t i = 0; i < diff; i++) {
      const size_t ind = FCount - i;
      Items[ind] = Items[ind - 1];
    }
    FCount++;
    return (Items[index] = Obj);
  }
  //..............................................................................
  T& operator [] (size_t index) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
  //..............................................................................
  T& GetItem(size_t index) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
  //..............................................................................
  T& GetLast() const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, FCount - 1, 0, FCount);
#endif
    return Items[FCount - 1];
  }
  //..............................................................................
  const T* GetData() const { return Items; }
  //..............................................................................
  template <class Functor> const TArrayList& ForEach(const Functor& f) const {
    for (size_t i = 0; i < FCount; i++)
      f.OnItem(Items[i], i);
    return *this;
  }
  //..............................................................................
  template <class Functor> ConstArrayList<T> Filter(const Functor& f) const {
    TArrayList rv;
    rv.SetCapacity(Count());
    for (size_t i = 0; i < FCount; i++) {
      if (f.OnItem(GetItem(i), i)) {
        rv.Add(GetItem(i));
      }
    }
    return rv;
  }
  //..............................................................................
  template <class Analyser> size_t Count(const Analyser& a) const {
    size_t cnt = 0;
    for (size_t i = 0; i < FCount; i++) {
      if (a.OnItem(GetItem(i), i)) {
        cnt++;
      }
    }
    return cnt;
  }
  //..............................................................................
  olx_capacity_t& GetCapacity() { return cap; }
  const olx_capacity_t& GetCapacity() const { return cap; }
  //..............................................................................
  TArrayList& SetCapacity(const olx_capacity_t& c) {
    cap.inc = c.inc;
    cap.inc_k = c.inc_k;
    return SetCapacity(c.capacity);
  }
  //..............................................................................
  TArrayList& SetCapacity(size_t v) {
    if (v <= cap.capacity) {
      return *this;
    }
    cap.capacity = v;
    T* Bf = new T[v];
    if (Items != 0) {
      for (size_t i = 0; i < FCount; i++) {
        Bf[i] = Items[i];
      }
      delete[] Items;
    }
    Items = Bf;
    return *this;
  }
  //..............................................................................
  template <class init_t>
  TArrayList& SetCapacity(size_t v, const init_t& inz) {
    if (v <= cap.capacity) {
      return *this;
    }
    size_t old_c = cap.capacity;
    cap.capacity = v;
    T* Bf = new T[v];
    if (Items != 0) {
      for (size_t i = 0; i < FCount; i++) {
        Bf[i] = Items[i];
      }
      delete[] Items;
    }
    for (size_t i = old_c; i < v; i++) {
      inz.OnItem(Bf[i], i);
    }
    Items = Bf;
    return *this;
  }
  //..............................................................................
  void Delete(size_t index) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, index, 0, FCount);
#endif
    for (size_t i = index + 1; i < FCount; i++) {
      Items[i - 1] = Items[i];
    }
    FCount--;
  }
  //..............................................................................
  void DeleteRange(size_t from, size_t count) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from + count, 0, FCount + 1);
#endif
    const size_t copy_cnt = FCount - from - count;
    for (size_t i = 0; i < copy_cnt; i++) {
      Items[from + i] = Items[from + count + i];
    }
    FCount -= count;
  }
  //..............................................................................
  bool Remove(const T& pObj) {
    const size_t i = IndexOf(pObj);
    if (i == InvalidIndex) {
      return false;
    }
    Delete(i);
    return true;
  }
  //..............................................................................
    // cyclic shift to the left
  TArrayList& ShiftL(size_t cnt) {
    if (FCount == 0) {
      return *this;
    }
    const size_t sv = cnt % FCount;
    if (sv == 0) {
      return *this;
    }
    if (sv == 1) {  // special case
      T D = Items[0];
      for (size_t i = 1; i <= FCount - 1; i++) {
        Items[i - 1] = Items[i];
      }
      Items[FCount - 1] = D;
    }
    else {
      T* D = new T[sv];
      for (size_t i = 0; i < sv; i++) {
        D[i] = Items[i];
      }
      for (size_t i = sv; i <= FCount - 1; i++) {
        Items[i - sv] = Items[i];
      }
      for (size_t i = 0; i < sv; i++) {
        Items[FCount - sv + i] = D[i];
      }
      delete[] D;
    }
    return *this;
  }
  //..............................................................................
    // cyclic shift to the right
  TArrayList& ShiftR(size_t cnt) {
    if (FCount == 0) {
      return *this;
    }
    const size_t sv = cnt % FCount;
    if (sv == 0) {
      return *this;
    }
    if (sv == 1) {  // special case
      T D = Items[FCount - 1];
      for (size_t i = 1; i < FCount; i++) {
        Items[FCount - i] = Items[FCount - i - 1];
      }
      Items[0] = D;
    }
    else {
      T* D = new T[sv];
      for (size_t i = 0; i < sv; i++) {
        D[i] = Items[FCount - sv + i];
      }
      const size_t diff = FCount - sv;
      for (size_t i = 1; i <= diff; i++) {
        Items[FCount - i] = Items[diff - i];
      }
      for (size_t i = 0; i < sv; i++) {
        Items[i] = D[i];
      }
      delete[] D;
    }
    return *this;
  }
  //..............................................................................
  void Swap(size_t i, size_t j) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, j, 0, FCount);
#endif
    T D = Items[i];
    Items[i] = Items[j];
    Items[j] = D;
  }
  //..............................................................................
  void Move(size_t from, size_t to) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, to, 0, FCount);
#endif
    T D = Items[from];
    if (from > to) {
      const size_t diff = from - to;
      for (size_t i = 0; i < diff; i++) {
        Items[from - i] = Items[from - i - 1];
      }
    }
    else {
      for (size_t i = from + 1; i <= to; i++) {
        Items[i - 1] = Items[i];
      }
    }
    Items[to] = D;
  }
  //..............................................................................
  size_t Count() const { return FCount; }
  //..............................................................................
  bool IsEmpty() const { return FCount == 0; }
  //..............................................................................
  template <class init_t>
  TArrayList& SetCount(size_t v, const init_t& initialiser) {
    const size_t cnt = FCount;
    SetCount(v);
    for (size_t i = cnt; i < FCount; i++) {
      initialiser.OnItem(Items[i], i);
    }
    return *this;
  }
  //..............................................................................
  TArrayList& SetCount(size_t v) {
    if (v > FCount) {
      if (v > cap.capacity) {
        SetCapacity(v);
      }
    }
    else if (v == 0) {
      if (Items != 0) {
        delete[] Items;
        Items = 0;
        cap.capacity = 0;
      }
    }
    else {
      cap.capacity = v + cap.inc;
      T* Bf = new T[cap.capacity];
      for (size_t i = 0; i < v; i++) {
        Bf[i] = Items[i];
      }
      if (Items != 0) {
        delete[] Items;
      }
      Items = Bf;
    }
    FCount = v;
    return *this;
  }
  //..............................................................................
  size_t IndexOf(const T& val) const {
    for (size_t i = 0; i < FCount; i++) {
      if (Items[i] == val)
        return i;
    }
    return InvalidIndex;
  }
  //..............................................................................
  bool Contains(const T& v) const { return IndexOf(v) != InvalidIndex; }
  //..............................................................................
  template <class size_t_list_t>
  TArrayList& Rearrange(const size_t_list_t& indices, bool inplace=true) {
    if (FCount < 2) {
      return *this;
    }
    if (FCount != indices.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "indices list size");
    }
    if (inplace) {
      olx_list_rearrange(*this, indices);
      return *this;
    }
    T* ni = olx_malloc<T>(cap.capacity = FCount);
    for (size_t i = 0; i < FCount; i++) {
#ifdef _DEBUG
      TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo,
        indices[i], 0, FCount);
#endif
      ni[i] = Items[indices[i]];
    }
    olx_free(Items);
    Items = ni;
    return *this;
  }
  //..............................................................................
  static TListTraverser<TArrayList<T> > Traverser;
public:
  struct InternalAccessor {
    TArrayList& list;
    InternalAccessor(TArrayList& l) : list(l) {}
    T& operator [](size_t i) { return list.Items[i]; }
    const T& operator [](size_t i) const { return list.Items[i]; }
    typedef T list_item_type;
  };
  typedef T list_item_type;
  typedef ConstArrayList<T> const_list_type;
  olx_list_2_std;
};

#ifndef __BORLANDC__
template <class T>
  TListTraverser<TArrayList<T> > TArrayList<T>::Traverser;
#endif

typedef TArrayList<int> TIntList;
typedef TArrayList<unsigned int> TUIntList;
typedef TArrayList<size_t> TSizeList;
typedef TArrayList<index_t> TIndexList;
typedef TArrayList<double> TDoubleList;

template <typename item_t>
class SharedArrayList : public shared_array<TArrayList<item_t>, item_t> {
  typedef TArrayList<item_t> arr_t;
  typedef shared_array<arr_t, item_t> parent_t;
public:
  SharedArrayList() {}
  SharedArrayList(const SharedArrayList &l) : parent_t(l) {}
  SharedArrayList(arr_t *arr) : parent_t(arr) {}
  SharedArrayList(arr_t &arr) : parent_t(arr) {}
  SharedArrayList &operator = (const SharedArrayList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t list_item_type;
};

template <typename item_t>
class ConstArrayList : public const_list<TArrayList<item_t> > {
  typedef TArrayList<item_t> arr_t;
  typedef const_list<arr_t> parent_t;
public:
  ConstArrayList(const ConstArrayList &l) : parent_t(l) {}
  ConstArrayList(arr_t *arr) : parent_t(arr) {}
  ConstArrayList(arr_t &arr) : parent_t(arr) {}
  ConstArrayList &operator = (const ConstArrayList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t list_item_type;
};

EndEsdlNamespace()
#endif
