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
#include "esort.h"
#include "etraverse.h"
#include "exception.h"
#include "shared.h"
#include "constlist.h"
BeginEsdlNamespace()

template <typename> class SharedArrayList;
template <typename> class ConstArrayList;

template <class T> class TArrayList : public IEObject {
private:
  size_t FCount, FCapacity;
  size_t FIncrement;
  T *Items;
  void init(size_t size)  {
    FCapacity = FCount = size;
    FIncrement = 5;
    if( FCapacity != 0 )
      Items = new T[FCapacity];
    else
      Items = NULL;
  }
public:
  // creates a new empty objects
  TArrayList()  {  init(0);  }
  // allocates size elements (can be accessed directly)
  TArrayList(size_t size)  {  init(size);  }
//..............................................................................
  template <class init_t>
  TArrayList(size_t size, const init_t &initialiser)  {
    init(size);
    for (size_t i=0; i < size; i++)
      initialiser.OnItem(Items[i], i);
  }
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the
  assignement operator must exist for nonpointer objects
  */
  TArrayList(const TArrayList& list)  {
    init(list.Count());
    for( size_t i=0; i < FCount; i++ )
      Items[i] = list.Items[i];
  }
//..............................................................................
  TArrayList(const SharedArrayList<T>& list) : Items(NULL)  {
    TakeOver(list.Release(), true);
  }
//..............................................................................
  TArrayList(const ConstArrayList<T>& list) : Items(NULL)  {
    TakeOver(list.Release(), true);
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TArrayList(size_t size, const T* array)  {
   init(size);
   for( size_t i=0; i < FCount; i++ )
     Items[i] = array[i];
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TArrayList(size_t size, T* array)  {
   init(size);
   for( size_t i=0; i < FCount; i++ )
     Items[i] = array[i];
  }
//..............................................................................
  template <class List, class Accessor>
  static ConstArrayList<T> FromList(const List& list,
    const Accessor& accessor)
  {
    TArrayList rv(list.Count());
    for (size_t i=0; i < list.Count(); i++)
      rv[i] = accessor(list[i]);
    return rv;
  }
//..............................................................................
  template <class List>
  static ConstArrayList<T> FromList(const List& list) {
    TArrayList rv(list.Count());
    for (size_t i=0; i < list.Count(); i++)
      rv[i] = accessor(list[i]);
    return rv;
  }
//..............................................................................
  //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TArrayList()  {
    if (Items != NULL) {
      delete [] Items;
      Items = NULL;
      FCount = FCapacity = 0;
    }
  }
//..............................................................................
  //deletes the objects and clears the list
  void Clear()  {  SetCount(0);  }
//..............................................................................
  TArrayList &TakeOver(TArrayList& l, bool do_delete=false)  {
    if( Items != NULL )  delete [] Items;
    FCount = l.FCount;
    FCapacity = l.FCapacity;
    FIncrement = l.FIncrement;
    Items = l.Items;
    l.FCount = l.FCapacity = 0;
    l.Items = NULL;
    if( do_delete )  delete &l;
    return *this;
  }
//..............................................................................
  virtual IEObject* Replicate() const {  return new TArrayList(*this);  }
//..............................................................................
  template <class List> TArrayList& Assign(const List& list)  {
    if( (void*)this == (void*)&list )  return *this;
    SetCount(list.Count());
    for( size_t i=0; i < FCount; i++ )
      Items[i] = list[i];
    return *this;
  }
//..............................................................................
  template <class List> TArrayList& operator = (const List& list)  {
    return Assign(list);
  }
//..............................................................................
  TArrayList& operator = (const SharedArrayList<T>& list)  {
    return TakeOver(list.Release(), true);
  }
//..............................................................................
  TArrayList& operator = (const ConstArrayList<T>& list)  {
    return TakeOver(list.Release(), true);
  }
//..............................................................................
  TArrayList& operator = (const TArrayList& list)  {
    return Assign(list);
  }
//..............................................................................
  template <class List> TArrayList& AddList(const List& list)  {
    SetCapacity(list.Count() + FCount);
    for( size_t i=0; i < list.Count(); i++ )
      Items[FCount+i] = list[i];
    FCount += list.Count();
    return *this;
  }
//..............................................................................
  TArrayList& AddList(const SharedArrayList<T>& list)  {
    return AddList(list.GetObject());
  }
//..............................................................................
  TArrayList& AddList(const ConstArrayList<T>& list)  {
    return AddList(list.GetObject());
  }
//..............................................................................
  template <class List> TArrayList& operator += (const List& list)  {
    return AddList(list);
  }
//..............................................................................
  T& Add(const T& Obj)  {
    if( FCapacity == FCount )  SetCapacity((size_t)(1.5*FCount + FIncrement));
    return (Items[FCount++] = Obj);
  }
//..............................................................................
  TArrayList& operator << (const T& o) {  Add(o);  return *this;  }
//..............................................................................
  TArrayList& operator << (const TArrayList& l) {  return AddList(l);  }
//..............................................................................
  T& Insert(size_t index, const T& Obj)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, index, 0, FCount+1);
#endif
    if( FCapacity == FCount )  SetCapacity((size_t)(1.5*FCount + FIncrement));
    const size_t diff = FCount - index;
    for( size_t i=0; i < diff; i++ )  {
      const size_t ind = FCount -i;
      Items[ind] = Items[ind-1];
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
    __POlxSourceInfo, FCount-1, 0, FCount);
#endif
    return Items[FCount-1];
  }
//..............................................................................
  const T* GetData() const {  return Items;  }
//..............................................................................
  template <class Functor> const TArrayList& ForEach(const Functor& f) const {
    for( size_t i=0; i < FCount; i++ )
      f.OnItem(Items[i], i);
    return *this;
  }
//..............................................................................
  TArrayList& SetCapacity(size_t v)  {
    if( v <= FCapacity )  return *this;
    FCapacity = v;
    T* Bf = new T[v];
    for( size_t i=0; i < FCount; i++ )
      Bf[i] = Items[i];
    if( Items != NULL )
      delete [] Items;
    Items = Bf;
    return *this;
  }
//..............................................................................
  TArrayList& SetIncrement(size_t v)  {
    FIncrement = v;
    return *this;
  }
//..............................................................................
  void Delete(size_t index)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, index, 0, FCount);
#endif
    for( size_t i=index+1; i < FCount; i++ )
      Items[i-1] = Items[i];
    FCount--;
  }
//..............................................................................
  void DeleteRange(size_t from, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from+count, 0, FCount+1);
#endif
    const size_t copy_cnt = FCount-from-count;
    for( size_t i=0; i < copy_cnt; i++ )
      Items[from+i] = Items[from+count+i];
    FCount -= count;
  }
//..............................................................................
  bool Remove(const T& pObj)  {
    const size_t i = IndexOf(pObj);
    if( i == InvalidIndex )  return false;
    Delete(i);
    return true;
  }
//..............................................................................
  // cyclic shift to the left
  TArrayList& ShiftL(size_t cnt)  {
    if( FCount == 0 )  return *this;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return *this;
    if( sv == 1 )  {  // special case
      T D = Items[0];
      for( size_t i=1; i <= FCount-1; i++ )
        Items[i-1] = Items[i];
      Items[FCount-1] = D;
    }
    else  {
      T* D = new T[sv];
      for( size_t i=0; i < sv; i++ )
        D[i] = Items[i];
      for( size_t i=sv; i <= FCount-1; i++ )
        Items[i-sv] = Items[i];
      for( size_t i=0; i < sv; i++ )
        Items[FCount-sv+i] = D[i];
      delete [] D;
    }
    return *this;
  }
//..............................................................................
  // cyclic shift to the right
  TArrayList& ShiftR(size_t cnt)  {
    if( FCount == 0 )  return *this;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return *this;
    if( sv == 1 )  {  // special case
      T D = Items[FCount-1];
      for( size_t i=1; i < FCount; i++ )
        Items[FCount-i] = Items[FCount-i-1];
      Items[0] = D;
    }
    else  {
      T* D = new T[sv];
      for( size_t i=0; i < sv; i++ )
        D[i] = Items[FCount-sv+i];
      const size_t diff = FCount-sv;
      for( size_t i=1; i <= diff; i++ )
        Items[FCount-i] = Items[diff-i];
      for( size_t i=0; i < sv; i++ )
        Items[i] = D[i];
      delete [] D;
    }
    return *this;
  }
//..............................................................................
  void Swap(size_t i, size_t j)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, j, 0, FCount);
#endif
    T D = Items[i];
    Items[i] = Items[j];
    Items[j] = D;
  }
//..............................................................................
  void Move(size_t from, size_t to)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, to, 0, FCount);
#endif
    T D = Items[from];
    if( from > to )  {
      const size_t diff = from-to;
      for( size_t i=0; i < diff; i++ )
        Items[from-i] = Items[from-i-1];
    }
    else  {
      for( size_t i=from+1; i <= to; i++ )
        Items[i-1] = Items[i];
    }
    Items[to] = D;
  }
//..............................................................................
  size_t Count() const {  return FCount;  }
//..............................................................................
  bool IsEmpty() const {  return FCount == 0;  }
//..............................................................................
  template <class init_t>
  TArrayList& SetCount(size_t v, const init_t &initialiser)  {
    const size_t cnt = FCount;
    SetCount(v);
    for (size_t i=cnt; i < FCount; i++)
      initialiser.OnItem(Items[i], i);
    return *this;
  }
//..............................................................................
  TArrayList& SetCount(size_t v)  {
    if( v > FCount )  {
      if( v > FCapacity )
        SetCapacity(v);
    }
    else if( v == 0 )  {
      if( Items != NULL )  {
        delete [] Items;
        Items = NULL;
        FCapacity = 0;
      }
    }
    else  {
      T* Bf = new T[v+FIncrement];
      for( size_t i=0; i < v; i++ )
        Bf[i] = Items[i];
      if( Items != NULL )
        delete [] Items;
      Items = Bf;
      FCapacity = v + FIncrement;
    }
    FCount = v;
    return *this;
  }
//..............................................................................
  size_t IndexOf(const T& val) const {
    for( size_t i=0; i < FCount; i++ )
      if( Items[i] == val )
        return i;
    return InvalidIndex;
  }
//..............................................................................
  bool Contains(const T &v) const { return IndexOf(v) != InvalidIndex; }
//..............................................................................
  static TListTraverser<TArrayList<T> > Traverser;
public:
  struct InternalAccessor {
    TArrayList &list;
    InternalAccessor(TArrayList &l) : list(l) {}
    T &operator [](size_t i) { return list.Items[i]; }
    const T &operator [](size_t i) const { return list.Items[i]; }
    typedef T list_item_type;
  };
  typedef T list_item_type;
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
