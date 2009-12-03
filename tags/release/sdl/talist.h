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
    Items = new T[FCapacity];
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
    delete [] Items;
  }
//..............................................................................
  //deletes the objects and clears the list
  inline void Clear()  {  SetCount(0);  }
//..............................................................................
  virtual IEObject* Replicate() const  {
    return new TArrayList(*this);
  }
//..............................................................................
  const TArrayList& Assign( const TArrayList& list )  {
    SetCount( list.Count() );
    for( size_t i=0; i < FCount; i++ )
      Items[i] = list.Items[i];
    return list;
  }
//..............................................................................
  void AddList( const TArrayList& list )  {
    SetCapacity( list.Count() + FCount );
    for( size_t i=0; i < list.FCount; i++ )
      Items[FCount+i] = list.Items[i];
    FCount += list.Count();
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
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T& Item(size_t index) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T& Last() const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, FCount-1, 0, FCount);
#endif
    return Items[FCount-1];
  }
//..............................................................................
  inline const T& GetItem(size_t index) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline const TArrayList& operator = ( const TArrayList& list )  {
    return Assign(list);
  }
//..............................................................................
  void SetCapacity(size_t v)  {
    if( v < FCapacity )    return;
    FCapacity = v;
    T* Bf = new T[v];
    for( size_t i=0; i < FCount; i++ )
      Bf[i] = Items[i];
    delete [] Items;
    Items = Bf;
  }
//..............................................................................
  inline void SetIncrement(size_t v)  {  FIncrement = v;  }
//..............................................................................
  void Delete(size_t index)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    for( size_t i=index+1; i < FCount; i++ )
      Items[i-1] = Items[i];
    FCount --;
  }
//..............................................................................
  void DeleteRange(size_t From, size_t To)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, From, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, To, 0, FCount);
#endif
    for( size_t i=To; i < FCount; i++ )
      Items[From+i-To] = Items[i];
    FCount -= (To-From);
  }
//..............................................................................
  void Remove(const T& pObj)  {
    index_t i = IndexOf(pObj);
    if( i != InvalidIndex )  Delete(i);
    else
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate specified object");
  }
//..............................................................................
  // cyclic shift to the left
  void ShiftL(size_t cnt)  {
    if( FCount == 0 )  return;
    size_t sv = cnt%FCount;
    if( sv <= 0 )  return;

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
    size_t sv = cnt%FCount;
    if( sv <= 0 )  return;

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
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, i, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, j, 0, FCount);
#endif
    T D = Items[i];
    Items[i] = Items[j];
    Items[j] = D;
  }
//..............................................................................
  void Move(size_t from, size_t to)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, to, 0, FCount);
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
  inline size_t Count()    const {  return FCount;  }
//..............................................................................
  inline bool IsEmpty()  const {  return FCount == 0;  }
//..............................................................................
  void SetCount(size_t v)  {
    if( v == FCount )  return;
#ifdef _OLX_DEBUG
   // TODO: check if v is valid
#endif
    if( v > FCount )  {
      if( v > FCapacity )
        SetCapacity(v + FIncrement);
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
      if( Items[i] == val )  return i;
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
