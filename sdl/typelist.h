/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_typelist_H
#define __olx_sdl_typelist_H
#include "etraverse.h"
#include "exception.h"
#include "tptrlist.h"
BeginEsdlNamespace()

template <typename> class SharedTypeList;
template <typename, typename> class ConstTypeListExt;
template <typename> class ConstTypeList;

template <class T, class DestructCast> class TTypeListExt : public IOlxObject {
protected:
  TPtrList<T> List;
  template <class Analyser> struct PackItemActor {
    const Analyser& analyser;
    PackItemActor(const Analyser& _analyser) : analyser(_analyser)
    {}
    bool OnItem(T* o, size_t i) const {
      if (analyser.OnItem(o, i)) {
        delete o;
        return true;
      }
      return false;
    }
  };
public:
  // creates a new empty objects
  TTypeListExt() {}
  // allocates storage for the given capacity
  explicit TTypeListExt(const olx_capacity_t& cap) : List(cap) {}
  TTypeListExt(size_t size, bool do_allocate = true) : List(size) {
    if (do_allocate) {
      for (size_t i = 0; i < size; i++) {
        List[i] = new T();
      }
    }
  }
  TTypeListExt(int size, bool do_allocate = true) : List(size) {
    if (do_allocate) {
      if (size < 0) {
        throw TInvalidArgumentException(__OlxSourceInfo, "size");
      }
      for (size_t i = 0; i < (size_t)size; i++) List[i] = new T();
    }
  }
  //..............................................................................
    /* copy constuctor - creates new copies of the objest, be careful as the copy
     constructor must exist for nonpointer objects */
  TTypeListExt(const TTypeListExt& list) : List(list.Count()) {
    for (size_t i = 0; i < list.Count(); i++) {
      List[i] = new T(list[i]);
    }
  }
  //..............................................................................
  TTypeListExt(const SharedTypeList<T>& list) {
    TakeOver(list.Release(), true);
  }
  //..............................................................................
  TTypeListExt(const ConstTypeListExt<T, DestructCast>& list) {
    TTypeListExt& l = list.Release();
    List.TakeOver(l.List);
    delete& l;
  }
  //..............................................................................
    /* copy constuctor - creates new copies of the objest, be careful as the copy
     constructor must exist for nonpointer objects */
  template <class alist> TTypeListExt(const alist& list)
    : List(list.Count())
  {
    for (size_t i = 0; i < list.Count(); i++) {
      List[i] = new T(list[i]);
    }
  }
  //..............................................................................
    /* copy constuctor - creates new copies of the objest, be careful as the copy
     constructor must exist for nonpointer objects */
  template <class list_t, class accessor_t> TTypeListExt(
    const list_t& list, const accessor_t& accessor)
    : List(list.Count())
  {
    for (size_t i = 0; i < list.Count(); i++) {
      List[i] = new T(accessor(list[i]));
    }
  }
  //..............................................................................
    /* copies values from an array of size elements  */
  TTypeListExt(size_t size, const T* array) : List(size) {
    for (size_t i = 0; i < size; i++) {
      List[i] = new T(array[i]);
    }
  }
  TTypeListExt(size_t size, T* array) : List(size) {
    for (size_t i = 0; i < size; i++) {
      List[i] = new T(array[i]);
    }
  }
  //..............................................................................
    //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TTypeListExt() { Clear(); }
  //..............................................................................
    //deletes the objects and clears the list
  TTypeListExt& Clear() {
    for (size_t i = 0; i < List.Count(); i++) {
      delete (DestructCast*)List[i];
    }
    List.Clear();
    return *this;
  }
  //..............................................................................
  TTypeListExt& TakeOver(TTypeListExt& l, bool do_delete = false) {
    Clear();
    List.TakeOver(l.List);
    if (do_delete) {
      delete &l;
    }
    return *this;
  }
  //..............................................................................
  const TPtrList<T>& ptr() const { return List; }
  //..............................................................................
  // creates new copies of objects, be careful as the copy constructor must exist
  template <class alist> void AddCopyAll(const alist& list) {
    List.SetCapacity(list.Count() + List.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      List.Add(new T(list[i]));
    }
  }
  //..............................................................................
  // objects will be deleted - make sure the owning container releases them!
  template <class alist> void AddAll(const alist& list) {
    List.SetCapacity(list.Count() + List.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      List.Add(list[i]);
    }
  }
  //..............................................................................
  //adds a new object into the list - will be deleted
  template <class obj_t>
  obj_t& Add(obj_t& Obj) { return *List.Add(&Obj); }
  //adds a new object into the list - will be deleted
  template <class obj_t>
  obj_t& Add(obj_t* Obj) { return *List.Add(Obj); }
  //..............................................................................
    //sets the list item to an object, which will be deleted
  template <class obj_t>
  obj_t& Set(size_t index, obj_t& Obj) {
    if (List[index] != 0) {
      delete (DestructCast*)List[index];
    }
    return *(T*)(List[index] = &Obj);
  }
  //sets the list item to an object, which will be deleted
  template <class obj_t>
  obj_t& Set(size_t index, obj_t* Obj) {
    if (List[index] != 0) {
      delete (DestructCast*)List[index];
    }
    return *(T*)(List[index] = Obj);
  }
  //..............................................................................
    /*replaces a list item with given value and returns a pointer to previous
    object (might be NULL)
    */
  T* Replace(size_t index, T& Obj) {
    T* rv = List[index];
    List[index] = &Obj;
    return rv;
  }
  /*replaces a list item with given value and returns a pointer to previous
  object (might be NULL)
  */
  T* Replace(size_t index, T* Obj) {
    T* rv = List[index];
    List[index] = Obj;
    return rv;
  }
  //..............................................................................
    // adds a copy of the object with copy constructor "copied copy"
  T& AddCopy(const T& Obj) { return AddNew<T>(Obj); }
  //..............................................................................
    //sets the listitem to an new object copied by the copy constructor
  T& SetCopy(size_t index, const T& Obj) {
    if (List[index] != 0) {
      delete (DestructCast*)List[index];
    }
    return *(List[index] = new T(Obj));
  }
  //..............................................................................
    // insert anobject into thelist; the object will be deleted
  T& Insert(size_t index, T& Obj) { return *List.Insert(index, &Obj); }
  // insert anobject into thelist; the object will be deleted
  T& Insert(size_t index, T* Obj) { return *List.Insert(index, Obj); }
  //..............................................................................
    // copy constructor created copy is inserted
  T& InsertCopy(size_t index, const T& Obj) {
    return InsertNew<T>(index, Obj);
  }
  //..............................................................................
    //inerts a new object at specified position
  T& InsertNew(size_t index) { return *List.Insert(index, new T()); }
  //..............................................................................
  template<class AC>
  T& InsertNew(size_t index, const AC& arg) {
    return *List.Insert(index, new T(arg));
  }
  //..............................................................................
  template<class FAC, class SAC>
  T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2) {
    return *List.Insert(index, new T(arg1, arg2));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC>
  T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2,
    const TAC& arg3)
  {
    return *List.Insert(index, new T(arg1, arg2, arg3));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
  T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2,
    const TAC& arg3, const FrAC& arg4)
  {
    return *List.Insert(index, new T(arg1, arg2, arg3, arg4));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
  T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2,
    const TAC& arg3, const FrAC& arg4, const FvAC& arg5)
  {
    return *List.Insert(index, new T(arg1, arg2, arg3, arg4, arg5));
  }
  //..............................................................................
  T& AddNew() { return *List.Add(new T()); }
  //..............................................................................
  template<class AC>
  T& AddNew(const AC& arg) { return *List.Add(new T(arg)); }
  //..............................................................................
  template<class FAC, class SAC>
  T& AddNew(const FAC& arg1, const SAC& arg2) {
    return *List.Add(new T(arg1, arg2));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC>
  T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3) {
    return *List.Add(new T(arg1, arg2, arg3));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
  T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4)
  {
    return *List.Add(new T(arg1, arg2, arg3, arg4));
  }
  //..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
  T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4, const FvAC& arg5)
  {
    return *List.Add(new T(arg1, arg2, arg3, arg4, arg5));
  }
  //..............................................................................
  T& operator [] (size_t index) const {
#ifdef _DEBUG
    T*& v = List[index];
    if (v == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "cannot dereference a NULL pointer");
    }
    return *v;
#else
    return *List[index];
#endif
  }
  //..............................................................................
  T& GetItem(size_t index) const {
#ifdef _DEBUG
    T*& v = List.GetItem(index);
    if (v == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "cannot dereference a NULL pointer");
    }
    return *v;
#else
    return *List.GetItem(index);
#endif
  }
  //..............................................................................
  T& GetLast() const {
#ifdef _DEBUG
    T*& v = List.GetLast();
    if (v == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "cannot dereference a NULL pointer");
    }
    return *v;
#else
    return *List.GetLast();
#endif
  }
  //..............................................................................
  bool IsNull(size_t index) const { return List[index] == 0; }
  //..............................................................................
    // the function is safe to be called repeatedly on the same index
  void NullItem(size_t index) const {
    if (List[index] != 0) {  // check if not deleted yet
      delete (DestructCast*)List[index];
      List[index] = 0;
    }
  }
  //..............................................................................
  template <class alist> TTypeListExt& Assign(const alist& list) {
    if ((void*)this == (void*)&list) {
      return *this;
    }
    for (size_t i = 0; i < List.Count(); i++) {
      delete (DestructCast*)List[i];
    }
    List.SetCount(list.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      List[i] = new T(list[i]);
    }
    return *this;
  }
  //..............................................................................
    /* copy - creates new copies of the objest, be careful as the copy constructor
     must exist  */
  TTypeListExt& operator = (const TTypeListExt& list) { return Assign(list); }
  //..............................................................................
  template <class wrapper_t>
  TTypeListExt& _Assign_Wrapper(const wrapper_t& list) {
    Clear();
    return TakeOver(list.Release(), true);
  }

  TTypeListExt& operator = (const SharedTypeList<T>& list) {
    return _Assign_Wrapper(list);
  }
  //..............................................................................
  TTypeListExt& operator = (const ConstTypeListExt<T, DestructCast>& list) {
    return _Assign_Wrapper(list);
  }
  //..............................................................................
  TTypeListExt& operator << (const T& item) { AddNew(item);  return *this; }
  //..............................................................................
  TTypeListExt& operator << (T* item) { Add(item);  return *this; }
  //..............................................................................
  TTypeListExt& operator << (const TTypeListExt& list) {
    AddAll(list);
    return *this;
  }
  //..............................................................................
    /* copy - creates new copies of the objest, be careful as the copy constructor
     must exist  */
  template <class alist> TTypeListExt& operator = (const alist& list) {
    return Assign(list);
  }
  //..............................................................................
  olx_capacity_t& GetCapacity() { return List.GetCapacity(); }
  const olx_capacity_t& GetCapacity() const { return List.GetCapacity(); }
  //..............................................................................
  TTypeListExt& SetCapacity(const olx_capacity_t &c) {
    List.SetCapacity(c);
    return *this;
  }
  //..............................................................................
  TTypeListExt& SetCapacity(size_t v) {
    List.SetCapacity(v);
    return *this;
  }
  //..............................................................................
  //..............................................................................
  void Delete(size_t index) {
    if (List[index] != 0) {
      delete (DestructCast*)List[index];
    }
    List.Delete(index);
  }
  //..............................................................................
  void DeleteRange(size_t from, size_t count) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0,
      List.Count());
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from + count, 0,
      List.Count() + 1);
#endif
    for (size_t i = 0; i < count; i++) {
      if (List[from + i] != 0) {
        delete (DestructCast*)List[from + i];
      }
    }
    List.DeleteRange(from, count);
  }
  //..............................................................................
  ConstTypeListExt<T, DestructCast> SubList(size_t from, size_t count) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0,
      List.Count() + 1);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from + count, 0,
      List.Count() + 1);
#endif
    TTypeListExt rv(count, false);
    for (size_t i = 0; i < count; i++) {
      rv.List[i] = new T(*List[i + from]);
    }
    return rv;
  }
  //..............................................................................
  ConstTypeListExt<T, DestructCast> SubListFrom(size_t from) const {
    return SubList(from, List.Count() - from);
  }
  //..............................................................................
  ConstTypeListExt<T, DestructCast> SubListTo(size_t to) const {
    return SubList(0, to);
  }
  //..............................................................................
  TTypeListExt& Shrink(size_t newSize) {
    if (newSize >= List.Count()) {
      return *this;
    }
    for (size_t i = newSize; i < List.Count(); i++)
      if (List[i] != 0) {
        delete (DestructCast*)List[i];
      }
    List.SetCount(newSize);
    return *this;
  }
  //..............................................................................
    // the memory has to be delalocated by calling process (using delete)
  T& Release(size_t index) {
    T* v = List[index];
#ifdef _DEBUG
    if (v == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "cannot dereference a NULL pointer");
    }
#endif
    List.Delete(index);
    return *v;
  }
  //..............................................................................
    // the memory has to be deallocated by calling process (using delete)
  void ReleaseAll() { List.Clear(); }
  //..............................................................................
  bool Remove(const T& Obj) {
    const size_t ind = IndexOf(Obj);
    if (ind == InvalidIndex) {
      return false;
    }
    Delete(ind);
    return true;
  }
  //..............................................................................
    /* rearranges the list according to provided indexes. Indexes must be unique
    unless objects are pointers
    */
  template <class size_t_list_t>
  TTypeListExt& Rearrange(const size_t_list_t& indexes) {
    List.Rearrange(indexes);
    return *this;
  }
  //..............................................................................
    // cyclic shift to the left
  TTypeListExt& ShiftL(size_t cnt) { List.ShiftL(cnt);  return *this; }
  //..............................................................................
    // cyclic shift to the right
  TTypeListExt& ShiftR(size_t cnt) { List.ShiftR(cnt);  return *this; }
  //..............................................................................
  void Swap(size_t i, size_t j) { List.Swap(i, j); }
  //..............................................................................
  void Move(size_t from, size_t to) { List.Move(from, to); }
  //..............................................................................
  TTypeListExt& Pack() { List.Pack();  return  *this; }
  //..............................................................................
  template <class PackAnalyser>
  TTypeListExt& Pack(const PackAnalyser& pa) {
    List.Pack(PackItemActor<PackAnalyser>(pa));
    return *this;
  }
  //..............................................................................
  template <class Functor> const TTypeListExt& ForEach(const Functor& f) const {
    for (size_t i = 0; i < List.Count(); i++) {
      f.OnItem(GetItem(i), i);
    }
    return *this;
  }
  //..............................................................................
    /* copy constructor must be implemented
    */
  template <class Functor>
  ConstTypeListExt<T, DestructCast> Filter(const Functor& f) const {
    TTypeListExt rv;
    rv.SetCapacity(List.Count());
    for (size_t i = 0; i < List.Count(); i++) {
      if (f.OnItem(GetItem(i), i)) {
        rv.Add(new T(GetItem(i)));
      }
    }
    return rv;
  }
  //..............................................................................
  template <class Analyser> size_t Count(const Analyser& a) const {
    size_t cnt = 0;
    for (size_t i = 0; i < List.Count(); i++) {
      if (a.OnItem(GetItem(i), i)) {
        cnt++;
      }
    }
    return cnt;
  }
  //..............................................................................
  size_t Count() const { return List.Count(); }
  //..............................................................................
  TTypeListExt& SetCount(size_t v, bool shrink = true) {
    if (v < List.Count()) {
      for (size_t i = v; i < List.Count(); i++) {
        if (List[i] != 0) {
          delete (DestructCast*)List[i];
        }
      }
      List.SetCount(v, shrink);
    }
    else {
      size_t cnt = List.Count();
      List.SetCapacity(v);
      for (size_t i = cnt; i < v; i++) {
        List.Add(new T());
      }
    }
    return *this;
  }
  //..............................................................................
  bool IsEmpty() const { return List.IsEmpty(); }
  //..............................................................................
  // the comparison operator is used
  size_t IndexOf(const T& val) const {
    for (size_t i = 0; i < List.Count(); i++) {
      if (*List[i] == val) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //..............................................................................
  size_t IndexOf(const T* val) const { return List.IndexOf(val); }
  //..............................................................................
  bool Contains(const T& v) const { return IndexOf(v) != InvalidIndex; }
  //..............................................................................
  bool Contains(const T* v) const { return IndexOf(v) != InvalidIndex; }
  //..............................................................................
  struct Accessor {
    static T* get(TTypeListExt<T, DestructCast>& l, size_t i) {
      return l.List[i];
    }
  };
  static TListTraverser<TTypeListExt<T, DestructCast> > Traverser;
public:
  struct InternalAccessor : public TPtrList<T>::InternalAccessor {
    InternalAccessor(TTypeListExt& l)
      : TPtrList<T>::InternalAccessor(l.List)
    {}
  };
  typedef T list_item_type;
  typedef ConstTypeListExt<T, DestructCast> const_list_type;
  olx_list_2_std;
};

template <class T>
  class TTypeList : public TTypeListExt<T,T>  {
  public:
    TTypeList() : TTypeListExt<T,T>()  {}
    TTypeList(size_t size, bool do_allocate=true)
      : TTypeListExt<T,T>(size, do_allocate)  {}
    TTypeList(int size, bool do_allocate=true)
      : TTypeListExt<T,T>(size, do_allocate)  {}
    TTypeList(const TTypeList& list) : TTypeListExt<T,T>(list)  {}
    TTypeList(const SharedTypeList<T>& list) : TTypeListExt<T,T>(list)  {}
    template <class alist> TTypeList(const alist& list)
      : TTypeListExt<T,T>(list)  {}
    template <class list_t, class accessor_t>
    TTypeList(const list_t& list, const accessor_t &acc)
      : TTypeListExt<T,T>(list, acc)  {}
    TTypeList(size_t size, const T* array) : TTypeListExt<T,T>(size, array)  {}
    TTypeList(size_t size, T* array) : TTypeListExt<T,T>(size, array)  {}
    TTypeList& operator = (const TTypeList& list)  {
      TTypeListExt<T,T>::operator = (list);
      return *this;
    }
    TTypeList& operator = (const SharedTypeList<T>& list)  {
      TTypeListExt<T,T>::operator = (list);
      return *this;
    }
    template <class alist> TTypeList& operator = (const alist& list)  {
      TTypeListExt<T,T>::operator = (list);
      return *this;
    }
    template <class Functor>
    ConstTypeList<T> Filter(const Functor& f) const {
      TTypeList rv;
      rv.SetCapacity(this->List.Count());
      for (size_t i = 0; i < this->List.Count(); i++) {
        if (f.OnItem(this->GetItem(i), i)) {
          rv.Add(new T(this->GetItem(i)));
        }
      }
      return rv;
    }
  public:
    typedef T list_item_type;
    typedef ConstTypeList<T> const_list_type;
  };
#ifndef __BORLANDC__
template <class T, typename DestructCast>
  TListTraverser<TTypeListExt<T,DestructCast> >
    TTypeListExt<T,DestructCast>::Traverser;
#endif


template <typename item_t>
class SharedTypeList : public shared_list<TTypeList<item_t>, item_t> {
  typedef TTypeList<item_t> lst_t;
  typedef shared_list<lst_t, item_t> parent_t;
public:
  SharedTypeList() {}
  SharedTypeList(const SharedTypeList &l) : parent_t(l) {}
  SharedTypeList(lst_t *lst) : parent_t(lst) {}
  SharedTypeList(lst_t &lst) : parent_t(lst) {}
  SharedTypeList &operator = (const SharedTypeList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t list_item_type;
};

template <typename item_t, typename d_t>
class ConstTypeListExt : public const_list<TTypeListExt<item_t, d_t> > {
  typedef TTypeListExt<item_t, d_t> lst_t;
  typedef const_list<lst_t> parent_t;
public:
  ConstTypeListExt(const ConstTypeListExt &l) : parent_t(l) {}
  ConstTypeListExt(lst_t *lst) : parent_t(lst) {}
  ConstTypeListExt(lst_t &lst) : parent_t(lst) {}
  ConstTypeListExt &operator = (const ConstTypeListExt &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t list_item_type;
};

template <typename item_t>
class ConstTypeList : public const_list<TTypeList<item_t> > {
  typedef TTypeList<item_t> lst_t;
  typedef const_list<lst_t> parent_t;
public:
  ConstTypeList(const ConstTypeList &l) : parent_t(l) {}
  ConstTypeList(lst_t *lst) : parent_t(lst) {}
  ConstTypeList(lst_t &lst) : parent_t(lst) {}
  ConstTypeList &operator = (const ConstTypeList &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef item_t list_item_type;
};

EndEsdlNamespace()
#endif
