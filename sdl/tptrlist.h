/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ptrlist_H
#define __olx_sdl_ptrlist_H
#include "shared.h"
#include "constlist.h"
#include "esort.h"
#include "etraverse.h"
#include "exception.h"
BeginEsdlNamespace()

template <typename> class SharedPtrList;
template <typename> class ConstPtrList;

template <class T> class TPtrList : public IOlxObject  {
  size_t FCount, FCapacity;
  size_t FIncrement;
  T **Items;
  void Allocate() {
    if (FCapacity == 0) {
      olx_free(Items);
      Items = NULL;
    }
    else {
      Items = olx_realloc<T*>(Items, FCapacity);
    }
  }

  void init(size_t size) {
    FCount = size;
    FIncrement = 5;
    FCapacity = FCount;
    Items = NULL;
    if (size != 0) {
      Allocate();
      memset(Items, 0, FCapacity*sizeof(T*));
    }
  }
  void checkSize()  {
    if( FCapacity == FCount )
      SetCapacity((size_t)(1.5*FCount + FIncrement));
  }
  void init_from_array(size_t size, const T** array)  {
    init(size);
    memcpy(Items, array, size*sizeof(T*));
  }
public:
  // creates a new empty objects
  TPtrList()  {  init(0);  }
  // allocates size elements (can be accessed diretly)
  TPtrList(size_t size)  {  init(size);  }
  TPtrList(int size)  {  init(size);  }
//..............................................................................
  TPtrList(const TPtrList& list)  {
    init(list.Count());
    memcpy(Items, list.Items, list.Count()*sizeof(T*));
  }
//..............................................................................
  TPtrList(const SharedPtrList<T>& list) : Items(NULL) {
    TakeOver(list.Release(), true);
  }
//..............................................................................
  TPtrList(const ConstPtrList<T>& list) : Items(NULL) {
    TakeOver(list.Release(), true);
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TPtrList(size_t size, const T** array)  {  init_from_array(size, array);  }
  TPtrList(int size, const T** array)  {  init_from_array(size, array);  }
//..............................................................................
  template <class List, class Accessor>
  TPtrList(const List& list, const Accessor& accessor)  {
    init(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Set(i, accessor(list[i]));
  }
//..............................................................................
  template <class List> TPtrList(const List& list)  {
    init(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Set(i, list[i]);
  }
//..............................................................................
  virtual ~TPtrList()  {
    olx_free(Items);
    Items = NULL;
    FCount = FCapacity = 0;
  }
//..............................................................................
  //deletes the objects and clears the list
  void Clear()  {
    if( Items != NULL )  {
      olx_free(Items);
      Items = NULL;
      FCount = FCapacity = 0;
    }
  }
//..............................................................................
  // calls delete on all items, by default values are not validated to be not null
  const TPtrList& DeleteItems(bool validate=false) const {
    if( !validate )  {
      for( size_t i=0; i < FCount; i++ )
        delete Items[i];
    }
    else  {
      for( size_t i=0; i < FCount; i++ )
        if( Items[i] != NULL )
          delete Items[i];
    }
    return *this;
  }
//..............................................................................
  TPtrList& DeleteItems(bool validate=false)  {
    ((const TPtrList*)this)->DeleteItems(validate);
    return *this;
  }
//..............................................................................
  TPtrList &TakeOver(TPtrList& l, bool do_delete=false)  {
    olx_free(Items);
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
  virtual IOlxObject* Replicate() const {  return new TPtrList(*this);  }
//..............................................................................
  TPtrList& Assign(const TPtrList& list)  {
    if( (void*)this == (void*)&list )  return *this;
    SetCount(list.Count());
    memcpy(Items, list.Items, list.Count()*sizeof(T*));
    FCount = list.Count();
    return *this;
  }
//..............................................................................
  template <class List> TPtrList& Assign(const List& l)  {
    SetCount(l.Count());
    for( size_t i=0; i < l.Count(); i++ )
      Set(i, l[i]);
    return *this;
  }
//..............................................................................
  template <class List, class Accessor>
  TPtrList& Assign(const List& l, const Accessor& accessor)  {
    SetCount(l.Count());
    for( size_t i=0; i < l.Count(); i++ )
      Set(i, accessor(l[i]));
    return *this;
  }
//..............................................................................
  TPtrList& operator = (const TPtrList& l)  {  return Assign(l);  }
//..............................................................................
  template <class List> TPtrList& operator = (const List& l)  {
    return Assign(l);
  }
//..............................................................................
  TPtrList& operator = (const SharedPtrList<T>& l)  {
    return TakeOver(l.Release(), true);
  }
//..............................................................................
  TPtrList& operator = (const ConstPtrList<T>& l)  {
    return TakeOver(l.Release(), true);
  }
//..............................................................................
  TPtrList& AddList(const TPtrList& list)  {
    SetCapacity(list.Count() + FCount);
    memcpy(&Items[FCount], list.Items, list.Count()*sizeof(T*));
    FCount += list.Count();
    return *this;
  }
//..............................................................................
  TPtrList& AddList(const SharedPtrList<T>& list)  {
    return AddList(list.GetObject());
  }
//..............................................................................
  TPtrList& AddList(const ConstPtrList<T>& list)  {
    return AddList(list.GetObject());
  }
//..............................................................................
  template <class List> TPtrList& AddList(const List& l)  {
    const size_t off = FCount;
    SetCount(l.Count() + FCount);
    for( size_t i=0; i < l.Count(); i++ )
      Set(off+i, l[i]);
    return *this;
  }
//..............................................................................
  template <class List, class Accessor> TPtrList& AddList(const List& l,
    const Accessor& accessor)
  {
    const size_t off = FCount;
    SetCount(l.Count() + FCount);
    for( size_t i=0; i < l.Count(); i++ )
      Set(off+i, accessor(l[i]));
    return *this;
  }
//..............................................................................
  TPtrList& operator += (const TPtrList& l)  {  return AddList(l);  }
//..............................................................................
  template <class List> TPtrList& operator += (const List& l)  {
    return AddList(l);
  }
//..............................................................................
  template <class obj_t>
  obj_t *&Add(obj_t* pObj=NULL) {
    checkSize();
    return (obj_t *&)(Items[FCount++] = pObj);
  }
//..............................................................................
  template <class obj_t>
  obj_t *&Add(obj_t& Obj)  { return Add(&Obj); }
//..............................................................................
  T *&Add(T* pObj = NULL) {
    checkSize();
    return (Items[FCount++] = pObj);
  }
  //..............................................................................
  T *&Add(T& Obj)  { return Add(&Obj); }
  //..............................................................................
  TPtrList& operator << (T& o)  { Add(o);  return *this; }
  TPtrList& operator << (T* o)  {  Add(o);  return *this;  }
//..............................................................................
  TPtrList& operator << (const TPtrList& l)  {  return AddList(l);  }
//..............................................................................
  T*& Set(size_t i, T* pObj)  {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, FCount);
#endif
    return (Items[i] = pObj);
  }
//..............................................................................
  T*& Set(size_t i, T& Obj)  {  return Set(i , &Obj);  }
//..............................................................................
  T*& AddUnique(T* pObj)  {
    const size_t ind = IndexOf(pObj);
    if( ind != InvalidIndex )
      return Items[ind];
    return Add(pObj);
  }
//..............................................................................
  T*& AddUnique(T& Obj)  {  return AddUnique(&Obj);  }
//..............................................................................
  T*& Insert(size_t index, T* pObj=NULL)  {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, index, 0, FCount+1);
#endif
    checkSize();
    memmove(&Items[index+1], &Items[index], (FCount-index)*sizeof(T*));
    Items[index] = pObj;
    FCount++;
    return Items[index];
  }
//..............................................................................
  T*& Insert(size_t index, T& Obj)  {  return Insert(index, &Obj);  }
//..............................................................................
  TPtrList& Insert(size_t index, const TPtrList& list)  {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, index, 0, FCount+1);
#endif
    SetCapacity(FCount + FIncrement + list.Count());
    const size_t lc = list.Count();
    memmove(&Items[index+lc], &Items[index], (FCount-index)*sizeof(T*));
    memcpy(&Items[index], list.Items, lc*sizeof(T*));
    FCount += lc;
    return *this;
  }
//..............................................................................
  template <class List> TPtrList& Insert(size_t index, const List& list)  {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, index, 0, FCount+1);
#endif
    SetCapacity(FCount + FIncrement + list.Count());
    const size_t lc = list.Count();
    memmove(&Items[index+lc], &Items[index], (FCount-index)*sizeof(T*));
    for( size_t i=0; i < lc; i++ )
      Items[index+i] = list[i];
    FCount += lc;
    return *this;
  }
//..............................................................................
  TPtrList& Insert(size_t index, size_t cnt)  {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, index, 0, FCount+1);
#endif
    SetCapacity(FCount + FIncrement + cnt);
    memmove(&Items[index+cnt], &Items[index], (FCount-index)*sizeof(T*));
    FCount += cnt;
    return *this;
  }
//..............................................................................
  void NullItem(size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, index, 0, FCount);
#endif
    Items[index] = NULL;
  }
//..............................................................................
  T*& operator [] (size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  T*& GetItem(size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  T*& GetLast() const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(
    __POlxSourceInfo, FCount-1, 0, FCount);
#endif
    return Items[FCount-1];
  }
//..............................................................................
  TPtrList& SetCapacity(size_t v)  {
    if( v < FCapacity )  return *this;
    FCapacity = v;
    Allocate();
     // initialise the rest of items to NULL
    memset(&Items[FCount], 0, (FCapacity-FCount)*sizeof(T*));
    return *this;
  }
//..............................................................................
  TPtrList& SetIncrement(size_t v)  {
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
  ConstPtrList<T> SubList(size_t from, size_t count) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, Count()+1);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from+count, 0, Count()+1);
#endif
    TPtrList rv(count);
    memcpy(rv.Items, &Items[from], count*sizeof(T*));
    return rv;
  }
//..............................................................................
  ConstPtrList<T> SubListFrom(size_t start) const {
    return SubList(start, FCount-start);
  }
//..............................................................................
  ConstPtrList<T> SubListTo(size_t to) const {  return SubList(0, to);  }
//..............................................................................
  /*removes given item from the list, returns if the item existed. If there
  are more than 1 the same item in the list, only the first one will be removed
  */
  bool Remove(const T* pObj)  {
    const size_t i = IndexOf(pObj);
    if( i == InvalidIndex )  return false;
    Delete(i);
    return true;
  }
//..............................................................................
  bool Remove(const T& Obj)  {  return Remove(&Obj);   }
//..............................................................................
  // cyclic shift to the left
  TPtrList& ShiftL(size_t cnt)  {
    if( FCount == 0 )  return *this;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return *this;
    if( sv == 1 )  {  // special case
      T *D = Items[0];
      for( size_t i=1; i < FCount; i++ )
        Items[i-1] = Items[i];
      Items[FCount-1] = D;
    }
    else  {
      T** D = new T*[sv];
      memcpy( D, Items, sv*sizeof(T*));
      for( size_t i=sv; i < FCount; i++ )
        Items[i-sv] = Items[i];
      memcpy(&Items[FCount-sv], D, sv*sizeof(T*));
      delete [] D;
    }
    return *this;
  }
//..............................................................................
  // cyclic shift to the right
  TPtrList& ShiftR(size_t cnt)  {
    if( FCount == 0 )  return *this;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return *this;
    if( sv == 1 )  {  // special case
      T* D = Items[FCount-1];
      for( size_t i=1; i < FCount; i++ )
        Items[FCount-i] = Items[FCount-i-1];
      Items[0] = D;
    }
    else  {
      T** D = new T*[sv];
      memcpy(&D[0], &Items[FCount-sv], sv*sizeof(T*));
      const size_t diff = FCount-sv;
      for( size_t i=1; i <= diff; i++ )
        Items[FCount-i] = Items[diff-i];
      memcpy(&Items[0], &D[0], sv*sizeof(T*));
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
    T *D = Items[i];
    Items[i] = Items[j];
    Items[j] = D;
  }
//..............................................................................
  void Move(size_t from, size_t to)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, to, 0, FCount);
#endif
    T *D = Items[from];
    if( from > to )  {
      const size_t diff = from-to;
      for( size_t i=0; i < diff; i++ )
        Items[from-i] = Items[from-i-1];
    }
    else  {
      for( size_t i=from; i < to; i++ )
        Items[i] = Items[i+1];
    }
    Items[to] = D;
  }
//..............................................................................
  TPtrList& Pack()  {
    size_t nc = 0;  // count null pointers
    for( size_t i=0; i < FCount; i++, nc++ )  {
      if( Items[i] == NULL )  {
        nc--;
        continue;
      }
      Items[nc] = Items[i];
    }
    FCount = nc;
    return *this;
  }
//..............................................................................
  template <class PackAnalyser> TPtrList& Pack(const PackAnalyser& pa)  {
    size_t nc = 0;  // count null pointers
    for( size_t i=0; i < FCount; i++, nc++ )  {
      if( pa.OnItem(*Items[i], i) )  {
        nc--;
        continue;
      }
      Items[nc] = Items[i];
    }
    FCount = nc;
    return *this;
  }
//..............................................................................
  template <class Functor> const TPtrList& ForEach(const Functor& f) const {
    for (size_t i=0; i < FCount; i++)
      f.OnItem(*Items[i], i);
    return *this;
  }
//..............................................................................
  template <class Functor> ConstPtrList<T> Filter(const Functor& f) const {
    TPtrList rv;
    rv.SetCapacity(Count());
    for (size_t i = 0; i < FCount; i++) {
      if (f.OnItem(*Items[i], i)) {
        rv.Add(Items[i]);
      }
    }
    return rv.Fit();
  }
//..............................................................................
  // make list capcity equal to its size
  TPtrList& Fit() {
    FCapacity = FCount;
    Allocate();
    return *this;
  }
//..............................................................................
  size_t Count() const {  return FCount;  }
//..............................................................................
  bool IsEmpty() const {  return (FCount == 0);  }
//..............................................................................
  TPtrList& SetCount(size_t v)  {
    if( v == FCount )  return *this;
    if( v > FCount )  {
      if( v > FCapacity )
        SetCapacity(v + FIncrement);
      FCount = v;
    }
    else  {  // shrinking to exact size
      FCount = v;
      Fit();
    }
    return *this;
  }
//..............................................................................
  size_t IndexOf(const T* val) const {
    for( size_t i=0; i < FCount; i++ )
      if( Items[i] == val )
        return i;
    return InvalidIndex;
  }
//..............................................................................
  size_t IndexOf(const T& val) const { return IndexOf(&val); }
//..............................................................................
  template <typename AT>
  bool Contains(const AT &v) const { return IndexOf(v) != InvalidIndex; }
//..............................................................................
  template <class size_t_list_t>
  TPtrList& Rearrange(const size_t_list_t &indices)  {
    if (FCount < 2)  return *this;
    if (FCount != indices.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "indices list size");
    }
    // allocate the list of NULLs
    T** ni = olx_malloc<T*>(FCapacity = FCount);
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
  static TListTraverser<TPtrList<T> > Traverser;
public:
  struct InternalAccessor {
    TPtrList &list;
    InternalAccessor(TPtrList &l) : list(l) {}
    T *operator [](size_t i) { return list.Items[i]; }
    const T *operator [](size_t i) const { return list.Items[i]; }
    typedef T* list_item_type;
  };
  typedef T *list_item_type;
  typedef ConstPtrList<T> const_list_type;
};

#ifndef __BORLANDC__
template <class T>
  TListTraverser<TPtrList<T> > TPtrList<T>::Traverser;
#endif

template <typename item_t>
class SharedPtrList : public shared_ptr_list<TPtrList<item_t>, item_t> {
  typedef TPtrList<item_t> lst_t;
  typedef shared_ptr_list<lst_t, item_t> parent_t;
public:
  SharedPtrList() {}
  SharedPtrList(const SharedPtrList &l) : parent_t(l) {}
  SharedPtrList(lst_t *lst) : parent_t(lst) {}
  SharedPtrList(lst_t &lst) : parent_t(lst) {}
  SharedPtrList &operator = (const SharedPtrList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t *list_item_type;
};

template <typename item_t>
class ConstPtrList : public const_list<TPtrList<item_t> > {
  typedef TPtrList<item_t> lst_t;
  typedef const_list<lst_t> parent_t;
public:
  ConstPtrList(const ConstPtrList &l) : parent_t(l) {}
  ConstPtrList(lst_t *lst) : parent_t(lst) {}
  ConstPtrList(lst_t &lst) : parent_t(lst) {}
  ConstPtrList &operator = (const ConstPtrList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t *list_item_type;
};

EndEsdlNamespace()
#endif
