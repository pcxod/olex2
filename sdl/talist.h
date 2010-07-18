#ifndef __olx_sdl_array_list_H
#define __olx_sdl_array_list_H
#include "ebase.h"
#include "esort.h"
#include "etraverse.h"
#include "exception.h"

BeginEsdlNamespace()

template <class T> class TArrayList : public AReferencible  {
private:
  size_t FCount, FCapacity;
  size_t FIncrement;
  T *Items;
  void init(size_t size, bool exact)  {
    FCount = size;
    FIncrement = 5;
    if( !exact )
      FCapacity = FCount + FIncrement;
    else
      FCapacity = FCount;
    if( FCapacity != 0 )
      Items = new T[FCapacity];
    else
      Items = NULL;
  }
public:
  // creates a new empty objects
  TArrayList()  {  init(0, true);  }
  // allocates size elements (can be accessed diretly)
  TArrayList(size_t size, bool exact=true)  {  init(size, exact);  }
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the assignement
   operator must exist for nonpointer objects */
  TArrayList(const TArrayList& list)  {
    init(list.Count(), true);
    for( size_t i=0; i < FCount; i++ )
      Items[i] = list.Items[i];
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TArrayList(size_t size, const T* array)  {
   init(size, true);
   for( size_t i=0; i < FCount; i++ )
     Items[i] = array[i];
  }
//..............................................................................
  //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TArrayList()  {
    if( Items != NULL )
      delete [] Items;
  }
//..............................................................................
  //deletes the objects and clears the list
  inline void Clear()  {  SetCount(0);  }
//..............................................................................
  virtual IEObject* Replicate() const {  return new TArrayList(*this);  }
//..............................................................................
  template <class List> TArrayList& Assign(const List& list)  {
    SetCount(list.Count());
    for( size_t i=0; i < FCount; i++ )
      Items[i] = list[i];
    return *this;
  }
//..............................................................................
  template <class List> inline TArrayList& operator = (const List& list)  {
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
  template <class List> inline TArrayList& operator += (const TArrayList& list)  {
    return AddList(list);
  }
//..............................................................................
  const T& Add(const T& Obj)  {
    if( FCapacity == FCount )  SetCapacity((size_t)(1.5*FCount + FIncrement));
    Items[FCount] = Obj;
    FCount ++;
    return Obj;
  }
//..............................................................................
  const T& Insert(size_t index, const T& Obj)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount+1);
#endif
    if( FCapacity == FCount )  SetCapacity((size_t)(1.5*FCount + FIncrement));
    const size_t diff = FCount - index;
    for( size_t i=0; i < diff; i++ )  {
      const size_t ind = FCount -i;
      Items[ind] = Items[ind-1];
    }
    Items[index] = Obj;
    FCount++;
    return Obj;
  }
//..............................................................................
  inline T& operator [] (size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T& Item(size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T& Last() const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, FCount-1, 0, FCount);
#endif
    return Items[FCount-1];
  }
//..............................................................................
  inline const T& GetItem(size_t index) const {
#ifdef _DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  template <class Functor> void ForEach(const Functor& f) const {
    for( size_t i=0; i < FCount; i++ )
      f.OnItem(Items[i]);
  }
//..............................................................................
  template <class Functor> void ForEachEx(const Functor& f) const {
    for( size_t i=0; i < FCount; i++ )
      f.OnItem(Items[i], i);
  }
//..............................................................................
  void SetCapacity(size_t v)  {
    if( v < FCapacity )    return;
    FCapacity = v;
    T* Bf = new T[v];
    for( size_t i=0; i < FCount; i++ )
      Bf[i] = Items[i];
    if( Items != NULL )
      delete [] Items;
    Items = Bf;
  }
//..............................................................................
  inline void SetIncrement(size_t v)  {  FIncrement = v;  }
//..............................................................................
  void Delete(size_t index)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, FCount);
#endif
    for( size_t i=index+1; i < FCount; i++ )
      Items[i-1] = Items[i];
    FCount--;
  }
//..............................................................................
  void DeleteRange(size_t from, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+count, 0, FCount+1);
#endif
    const size_t copy_cnt = FCount-from-count;
    for( size_t i=0; i < copy_cnt; i++ )
      Items[from+i] = Items[from+count+i];
    FCount -= count;
  }
//..............................................................................
  void Remove(const T& pObj)  {
    size_t i = IndexOf(pObj);
    if( i != InvalidIndex )
      Delete(i);
    else
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate specified object");
  }
//..............................................................................
  // cyclic shift to the left
  void ShiftL(size_t cnt)  {
    if( FCount == 0 )  return;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return;
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
  }
//..............................................................................
  // cyclic shift to the right
  void ShiftR(size_t cnt)  {
    if( FCount == 0 )  return;
    const size_t sv = cnt%FCount;
    if( sv == 0 )  return;
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
  }
//..............................................................................
  inline void Swap(size_t i, size_t j)  {
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
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, to, 0, FCount);
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
  inline size_t Count() const {  return FCount;  }
//..............................................................................
  inline bool IsEmpty() const {  return FCount == 0;  }
//..............................................................................
  void SetCount(size_t v)  {
    if( v == FCount )  return;
    if( v > FCount )  {
      if( v > FCapacity )
        SetCapacity(v + FIncrement);
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
      delete [] Items;
      Items = Bf;
      FCapacity = v + FIncrement;
    }
    FCount = v;
  }
//..............................................................................
  size_t IndexOf(const T& val) const  {
    for( size_t i=0; i < FCount; i++ )
      if( Items[i] == val )
        return i;
    return InvalidIndex;
  }

  static TQuickObjectSorter<TArrayList<T>,T> QuickSorter;
  static TBubbleSorter<TArrayList<T>,T> BubleSorter;
  static TListTraverser<TArrayList<T> > Traverser;
};

  typedef TArrayList<int> TIntList;
  typedef TArrayList<unsigned int> TUIntList;
  typedef TArrayList<size_t> TSizeList;
  typedef TArrayList<index_t> TIndexList;
  typedef TArrayList<double> TDoubleList;

EndEsdlNamespace()
#endif
