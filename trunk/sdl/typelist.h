/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_typelist_H
#define __olx_sdl_typelist_H
#include "esort.h"
#include "etraverse.h"
#include "exception.h"
#include "tptrlist.h"
BeginEsdlNamespace()

template <typename> class SharedTypeList;

template <class T, class DestructCast> class TTypeListExt : public IEObject  {
private:
  // initialisation function
  inline T& Alloc(size_t i)  {  return *(T*)( List[i] = new T() );  }
protected:
  TPtrList<T> List;
  template <class Analyser> struct PackItemActor  {
    const Analyser& analyser;
    PackItemActor(const Analyser& _analyser) : analyser(_analyser)  {}
    inline bool OnItem(T& o, size_t i) const {
      if( analyser.OnItem(o, i) )  {
        delete &o;
        return true;
      }
      return false;
    }
  };
public:
  // creates a new empty objects
  TTypeListExt() {}
  // allocates size elements (can be accessed diretly)
  TTypeListExt(size_t size) : List(size)  {}
  TTypeListExt(int size) : List(size)  {}
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the copy
   constructor must exist for nonpointer objects */
  TTypeListExt(const TTypeListExt& list) : List(list.Count())  {
    for( size_t i=0; i < list.Count(); i++ )
      List[i] =  new T(list[i]);
  }
//..............................................................................
  TTypeListExt(const SharedTypeList<T>& list)  {
    TTypeListExt &l = list.Release();
    List.TakeOver(l.List);
    delete &l;
  }
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the copy
   constructor must exist for nonpointer objects */
  template <class alist> TTypeListExt(const alist& list ) : List(list.Count())  {
    for( size_t i=0; i < list.Count(); i++ )
      List[i] = new T(list[i]);
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TTypeListExt(size_t size, const T* array) : List(size)  {
    for( size_t i=0; i < size; i++ )
      List[i] = new T(array[i]);
  }
//..............................................................................
  //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TTypeListExt()  {  Clear();  }
//..............................................................................
  //deletes the objects and clears the list
  TTypeListExt& Clear()  {
    for( size_t i=0; i < List.Count(); i++ )
      delete (DestructCast*)List[i];
    List.Clear();
    return *this;
  }
//..............................................................................
/*  virtual IEObject* Replicate() const  {
    TTypeListExt<T, DestructCast>* list = new TTypeListExt<T, DestructCast>( Count() );
    for( size_t i=0; i < Count(); i++ )
      list->Alloc(i) = *(T*)FList->Item(i);
    return list;
  }
*/
//..............................................................................
  /* creates new copies of the objest, be careful as the assignement operator must exist  */
  template <class alist> void AddListA(const alist& list)  {
    List.SetCapacity(list.Count() + List.Count());
    for( size_t i=0; i < list.Count(); i++ )
      *List.Add(new T()) = list[i];
  }
//..............................................................................
  /* creates new copies of the objest, be careful as the copy constructor must exist  */
  template <class alist> void AddListC(const alist& list)  {
    List.SetCapacity(list.Count() + List.Count());
    for( size_t i=0; i < list.Count(); i++ )
      List.Add(new T(list[i]));
  }
//..............................................................................
//  operator const TEList& () const { return *FList;  }
//  TEList& c_list() { return *FList;  }
//..............................................................................
  //adds a new object ito the list - will be deleted
  inline T& Add(T& Obj)  {  return *List.Add(&Obj);  }
  //adds a new object ito the list - will be deleted
  inline T& Add(T* Obj)  {  return *List.Add(Obj);  }
//..............................................................................
  //sets the list item to an object, which will be deleted
  inline T& Set(size_t index, T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return *(T*)(List[index] = &Obj);
  }
  //sets the list item to an object, which will be deleted
  inline T& Set(size_t index, T* Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return *(T*)(List[index] = Obj);
  }
//..............................................................................
  //replaces a list item with given value and returns a pointer to previous object (might be NULL)
  inline T* Replace(size_t index, T& Obj)  {
    T* rv = List[index];
    List[index] = &Obj;
    return rv;
  }
  //replaces a list item with given value and returns a pointer to previous object (might be NULL)
  inline T* Replace(size_t index, T* Obj)  {
    T* rv = List[index];
    List[index] = Obj;
    return rv;
  }
//..............................................................................
  // adds a copy of the object with default constructor and assign operator "assigned copy"
  inline T& AddACopy(const T& Obj)  {  
    T& rv = AddNew();
    rv = Obj;
    return rv;
  }
//..............................................................................
  //sets the listitem to an new object copied by the assignement operator
  inline T& SetACopy(size_t index, const T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return (Alloc(index) = Obj);
  }
//..............................................................................
  // adds a copy of the object with copy constructor "copied copy"
  inline T& AddCCopy(const T& Obj)  {  return AddNew<T>(Obj);  }
//..............................................................................
  //sets the listitem to an new object copied by the copy constructor
  inline T& SetCCopy(size_t index, const T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return *(List[index] = new T(Obj));
  }
//..............................................................................
  // assigned copy inserted
  inline T& InsertACopy(size_t index, const T& Obj)  {  
    T& rv = InsertNew(index);
    rv = Obj;
    return rv;  
  }
//..............................................................................
  // insert anobject into thelist; the object will be deleted
  inline T& Insert(size_t index, T& Obj)  {  return *List.Insert(index, &Obj);  }
  // insert anobject into thelist; the object will be deleted
  inline T& Insert(size_t index, T* Obj)  {  return *List.Insert(index, Obj);  }
//..............................................................................
  // copy constructor created copy is inserted
  inline T& InsertCCopy(size_t index, const T& Obj)  {  return InsertNew<T>(index, Obj);  }
//..............................................................................
  //inerts a new object at specified position
  inline T& InsertNew(size_t index)  {  return *List.Insert(index, new T());  }
//..............................................................................
  template<class AC>
    inline T& InsertNew(size_t index, const AC& arg)  {
      return *List.Insert(index, new T(arg));
    }
//..............................................................................
  template<class FAC, class SAC>
    inline T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2)  {
      return *List.Insert(index, new T(arg1, arg2));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC>
    inline T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3)  {
      return *List.Insert(index, new T(arg1, arg2, arg3));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
    inline T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4)  {
      return *List.Insert(index, new T(arg1, arg2, arg3, arg4));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
    inline T& InsertNew(size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4, const FvAC& arg5)  {
      return *List.Insert(index, new T(arg1, arg2, arg3, arg4, arg5));
    }
//..............................................................................
  inline T& AddNew()  {  return *List.Add(new T());  }
//..............................................................................
  template<class AC>
    inline T& AddNew( const AC& arg )  {  return *List.Add(new T(arg));  }
//..............................................................................
  template<class FAC, class SAC>
    inline T& AddNew(const FAC& arg1, const SAC& arg2 )  {
      return *List.Add(new T(arg1, arg2));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC>
    inline T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3 )  {
      return *List.Add(new T(arg1, arg2, arg3));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
    inline T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4)  {
      return *List.Add(new T(arg1, arg2, arg3, arg4));
    }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
    inline T& AddNew(const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4, const FvAC& arg5)  {
      return *List.Add(new T(arg1, arg2, arg3, arg4, arg5));
    }
//..............................................................................
  inline T& operator [] (size_t index) const {
#ifdef _DEBUG
    T*& v = List[index];
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List[index];
#endif
  }
//..............................................................................
  inline T& GetItem(size_t index) const {
#ifdef _DEBUG
    T*& v = List.GetItem(index);
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List.GetItem(index);
#endif
  }
//..............................................................................
  inline T& GetLast() const {
#ifdef _DEBUG
    T*& v = List.GetLast();
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List.GetLast();
#endif
  }
//..............................................................................
  inline bool IsNull(size_t index) const {  return List[index] == NULL;  }
//..............................................................................
  // the function is safe to be called repeatedly on the same index
  inline void NullItem(size_t index) const {
    if( List[index] != NULL )  {  // check if not deleted yet
      delete (DestructCast*)List[index];
      List[index] = NULL;
    }
  }
//..............................................................................
  template <class alist> TTypeListExt& Assign(const alist& list)  {
    if( (void*)this == (void*)&list )  return *this;
    for( size_t i=0; i < List.Count(); i++ )
      delete (DestructCast*)List[i];
    List.SetCount( list.Count() );
    for( size_t i=0; i < list.Count(); i++ ) 
      List[i] =  new T(list[i]);
    return *this;
  }
//..............................................................................
  /* copy - creates new copies of the objest, be careful as the copy constructor
   must exist  */
  TTypeListExt& operator = (const TTypeListExt& list)  {  return Assign(list);  }
//..............................................................................
  TTypeListExt & operator = (const SharedTypeList<T>& list)  {
    Clear();
    TTypeListExt &l = list.Release();
    List.TakeOver(l.List);
    delete &l;
    return *this;
  }
//..............................................................................
  /* copy - creates new copies of the objest, be careful as the copy constructor
   must exist  */
  template <class alist> TTypeListExt& operator = (const alist& list)  {
    return Assign(list);
  }
//..............................................................................
  inline TTypeListExt& SetCapacity(size_t v)  {
    List.SetCapacity(v);
    return *this;
  }
//..............................................................................
  inline TTypeListExt& SetIncrement(size_t v)  {
    List.SetIncrement(v);
    return *this;
  }
//..............................................................................
  void Delete(size_t index)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    List.Delete(index);
  }
//..............................................................................
  void DeleteRange(size_t from, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, List.Count());
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+count, 0, List.Count()+1);
#endif
    for( size_t i=0; i < count; i++ )  {
      if( List[from+i] != NULL )
        delete (DestructCast*)List[from+i];
    }
    List.DeleteRange(from, count);
  }
//..............................................................................
  TTypeListExt SubList(size_t from, size_t count) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, List.Count()+1);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+count, 0, List.Count()+1);
#endif
    TTypeListExt rv(count);
    for( size_t i=0; i < count; i++ )
      rv.Set(i, new T(*List[i+from]));
    return rv;
  }
//..............................................................................
  TTypeListExt SubListFrom(size_t from) const {  return SubList(from, List.Count()-from);  }
//..............................................................................
  TTypeListExt SubListTo(size_t to) const {  return SubList(0, to);  }
//..............................................................................
  TTypeListExt& Shrink(size_t newSize)  {
    if( newSize >= List.Count() )  return *this;
    for( size_t i=newSize; i < List.Count(); i++ )
      if( List[i] != NULL )
        delete (DestructCast*)List[i];
    List.SetCount(newSize);
    return *this;
  }
//..............................................................................
  // the memory has to be delalocated by calling process (using delete)
  T& Release(size_t index)  {
    T*& v = List[index];
#ifdef _DEBUG
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
#endif
    List.Delete(index);
    return *v;
  }
//..............................................................................
  // the memory has to be delalocated by calling process (using delete)
  void ReleaseAll()  {  List.Clear();  }
//..............................................................................
  bool Remove(const T& Obj)  {
    const size_t ind = IndexOf(Obj);
    if( ind == InvalidIndex )
      return false;
    Delete(ind);
    return true;
  }
//..............................................................................
  /* rearranges the list according to provided indexes. Indexes must be unique unless
    objects are pointers
  */
  template <class size_t_list_t>
  TTypeListExt& Rearrange(const size_t_list_t& indexes)  {
    List.Rearrange(indexes);
    return *this;
  }
//..............................................................................
  // cyclic shift to the left
  inline TTypeListExt& ShiftL(size_t cnt)  {  List.ShiftL(cnt);  return *this;  }
//..............................................................................
  // cyclic shift to the right
  inline TTypeListExt& ShiftR(size_t cnt)  {  List.ShiftR(cnt);  return *this;  }
//..............................................................................
  inline void Swap(size_t i, size_t j)  {  List.Swap(i, j);  }
//..............................................................................
  inline void Move(size_t from, size_t to)  {  List.Move(from, to);  }
//..............................................................................
  inline TTypeListExt& Pack()  {  List.Pack();  return  *this;  }
//..............................................................................
  template <class PackAnalyser> inline TTypeListExt& Pack(const PackAnalyser& pa)  {
    List.Pack(PackItemActor<PackAnalyser>(pa));
    return *this;
  }
//..............................................................................
  template <class Functor> const TTypeListExt& ForEach(const Functor& f) const {
    for( size_t i=0; i < List.Count(); i++ )
      f.OnItem(GetItem(i), i);
    return *this;
  }
//..............................................................................
  inline size_t Count() const {  return List.Count();  }
//..............................................................................
  inline bool IsEmpty() const {  return List.IsEmpty();  }
//..............................................................................
  // the comparison operator is used
  size_t IndexOf(const T& val) const {
    for( size_t i=0; i < List.Count(); i++ )
      if( *List[i] == val )
        return i;
    return InvalidIndex;
  }
  struct Accessor  {
    static T* get(TTypeListExt<T,DestructCast>& l, size_t i)  {  return l.List[i];  }
  };
  static ListQuickSorter<TTypeListExt<T,DestructCast>,const T*, Accessor> QuickSorter;
  static ListBubbleSorter<TTypeListExt<T,DestructCast>,const T*, Accessor> BubleSorter;
  static TListTraverser<TTypeListExt<T,DestructCast> > Traverser;
};
template <class T>
  class TTypeList : public TTypeListExt<T,T>  {
  public:
    TTypeList() : TTypeListExt<T,T>()  {  }
    TTypeList(const size_t size) : TTypeListExt<T,T>(size)  {  }
    TTypeList(const TTypeList& list) : TTypeListExt<T,T>(list)  {  }
    TTypeList(const SharedTypeList<T>& list) : TTypeListExt<T,T>(list)  {  }
    template <class alist> TTypeList(const alist& list ) : TTypeListExt<T,T>(list)  {  }
    TTypeList(size_t size, const T* array) : TTypeListExt<T,T>(size, array)  {  }
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
  };
#ifndef __BORLANDC__
template <class T, typename DestructCast>
ListQuickSorter<TTypeListExt<T,DestructCast>,const T*,
  typename TTypeListExt<T,DestructCast>::Accessor> TTypeListExt<T,DestructCast>::QuickSorter;
template <class T, typename DestructCast>
ListBubbleSorter<TTypeListExt<T,DestructCast>,const T*,
  typename TTypeListExt<T,DestructCast>::Accessor> TTypeListExt<T,DestructCast>::BubleSorter;
template <class T, typename DestructCast>
  TListTraverser<TTypeListExt<T,DestructCast> > TTypeListExt<T,DestructCast>::Traverser;
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
};

EndEsdlNamespace()
#endif
