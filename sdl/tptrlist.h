//---------------------------------------------------------------------------
#ifndef typeptrlistH
#define typeptrlistH
#include <string.h>
#include <stdlib.h>
#include "talist.h"

BeginEsdlNamespace()

template <class T> class TPtrList : public IEObject  {
private:
  size_t FCount, FCapacity;
  size_t FIncrement;
  T **Items;
  inline void Allocate()  {
    Items = (T**)realloc( (void*)Items, FCapacity*sizeof(T*));
  }

  void init(size_t size)  {
    FCount = size;
    FIncrement = 5;
    FCapacity = FCount + FIncrement;
    Items = NULL;
    Allocate();
    memset(Items, 0, FCapacity*sizeof(T*));
  }

public:
  // creates a new empty objects
  TPtrList()  {  init(0);  }
  // allocates size elements (can be accessed diretly)
  TPtrList(size_t size)  {  init(size);  }
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the assignement
   operator must exist for nonpointer objects */
  TPtrList(const TPtrList& list)  {
   init(list.Count());
   memcpy(Items, list.Items, list.Count()*sizeof(T*));
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TPtrList(size_t size, const T** array )  {
   init( size );
   memcpy( Items, array, size*sizeof(T*));
  }
//..............................................................................
  //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TPtrList()  {  free(Items);  }
//..............................................................................
  //deletes the objects and clears the list
  inline void Clear()  {  SetCount(0);  }
//..............................................................................
  virtual IEObject* Replicate() const  {  return new TPtrList(*this);  }
//..............................................................................
  inline TPtrList& Assign(const TPtrList& list)  {
    SetCount(list.Count());
    memcpy(Items, list.Items, list.Count()*sizeof(T*));
    FCount = list.Count();
    return *this;
  }
  inline TPtrList& operator = (const TPtrList& l)  {  return Assign(l);  }
//..............................................................................
  inline TPtrList& AddList(const TPtrList& list)  {
    SetCapacity(list.Count() + FCount);
    memcpy(&Items[FCount], list.Items, list.Count()*sizeof(T*));
    FCount += list.Count();
    return *this;
  }
  inline TPtrList& operator += (const TPtrList& l)  {  return AddList(l);  }
//..............................................................................
  inline T*& Add(T* pObj)  {
    if( FCapacity == FCount )  
      SetCapacity((long)(1.5*FCount + FIncrement));
    Items[FCount] = pObj;
    return Items[FCount++];
  }
//..............................................................................
  inline T*& Add(T& Obj)  {
    if( FCapacity == FCount )  
      SetCapacity((long)(1.5*FCount + FIncrement));
    Items[FCount] = &Obj;
    return Items[FCount++];
  }
//..............................................................................
  inline T*& AddUnique(T* pObj)  {
    size_t ind = IndexOf(pObj);
    if( ind != InvalidIndex )  
      return Items[ind];
    if( FCapacity == FCount )  SetCapacity((long)(1.5*FCount + FIncrement));
    Items[FCount] = pObj;
    return Items[FCount++];
  }
//..............................................................................
  inline T*& AddUnique(T& Obj)  {
    index_t ind = IndexOf(Obj);
    if( ind >=0 )  
      return *Items[ind];
    if( FCapacity == FCount )  
      SetCapacity((long)(1.5*FCount + FIncrement));
    Items[FCount] = &Obj;
    return Items[FCount++];
  }
//..............................................................................
  inline T*& Insert(size_t index, T* pObj)  {
    if( FCapacity == FCount )  
      SetCapacity((long)(1.5*FCount + FIncrement));
    memmove(&Items[index+1], &Items[index], (FCount-index)*sizeof(T*));
    Items[index] = pObj;
    FCount++;
    return Items[index];
  }
//..............................................................................
  inline T*& Insert(size_t index, T& Obj)  {
    if( FCapacity == FCount )  
      SetCapacity((long)(1.5*FCount + FIncrement));
    memmove(&Items[index+1], &Items[index], (FCount-index)*sizeof(T*));
    Items[index] = &Obj;
    FCount++;
    return Items[index];
  }
//..............................................................................
  TPtrList<T>& Insert(size_t index, const TPtrList<T>& list)  {
    SetCapacity((long)(FCount + FIncrement + list.Count()));
    size_t lc = list.Count();
    memmove(&Items[index+lc], &Items[index], (FCount-index)*sizeof(T*));
    memcpy(&Items[index], list.Items, lc*sizeof(T*));
    FCount += list.Count();
    return *this;
  }
//..............................................................................
  inline void Insert(size_t index, size_t cnt)  {
    SetCapacity((long)(FCount + FIncrement + cnt));
    memmove(&Items[index+cnt], &Items[index], (FCount-index)*sizeof(T*));
    FCount += cnt;
  }
//..............................................................................
  inline T*& operator [] (size_t index) const {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T*& Item(size_t index) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline T*& Last() const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, FCount-1, 0, FCount);
#endif
    return Items[FCount-1];
  }
//..............................................................................
  inline const T* GetItem(size_t index) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
    return Items[index];
  }
//..............................................................................
  inline void SetCapacity(size_t v)  {
    if( v < FCapacity )    return;
    FCapacity = v;
    Allocate();
    memset( &Items[FCount], 0, (FCapacity-FCount)*sizeof(T*) );  // initialise the rest of items to NULL
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
  void DeleteRange(size_t from, size_t to)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, From, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, To, 0, FCount);
#endif
    for( size_t i=to; i < FCount; i++ )
      Items[from+i-to] = Items[i];
    FCount -= (to-from);
  }
//..............................................................................
  TPtrList SubList(size_t from, size_t count) const {
    TPtrList rv(count);
    memcpy(rv.Items, Items[from], count*sizeof(T*));
    return rv;
  }
//..............................................................................
  inline TPtrList SubListFrom(size_t start) const {  return SubList(from, FCount-start);  }
//..............................................................................
  inline TPtrList SubListTo(size_t to) const {  return SubList(0, to);  }
//..............................................................................
  inline void Remove(T* pObj)  {
    size_t i = IndexOf(pObj);
    if( i == InvalidIndex )  
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate specified object");
    Delete(i);
  }
//..............................................................................
  inline void Remove(const T& Obj)  {
    size_t i = IndexOf(Obj);
    if( i == InvalidIndex )  
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate specified object");
    Delete(i);
  }
//..............................................................................
  // cyclic shift to the left
  void ShiftL(size_t cnt)  {
    if( FCount == 0 )  return;
    size_t sv = cnt%FCount;
    if( sv == 0 )  return;
    if( sv == 1 )  {  // special case
      T *D = Items[0];
      for( size_t i=1; i <= FCount-1; i++ )
        Items[i-1] = Items[i];
      Items[FCount-1] = D;
    }
    else  {
      T** D = new T*[sv];
      memcpy( D, Items, sv*sizeof(T*) );
      for( size_t i=sv; i <= FCount-1; i++ )
        Items[i-sv] = Items[i];
      memcpy(&Items[FCount-sv], D, sv*sizeof(T*));
      delete [] D;
    }
  }
//..............................................................................
  // cyclic shift to the right
  void ShiftR(size_t cnt)  {
    if( FCount == 0 )  return;
    size_t sv = cnt%FCount;
    if( sv == 0 )  return;
    if( sv == 1 )  {  // special case
      T* D = Items[FCount-1];
      for( size_t i=1; i < FCount; i++ )
        Items[FCount-i] = Items[FCount-i-1];
      Items[0] = D;
    }
    else  {
      T** D = new T*[sv];
      memcpy( &D[0], &Items[FCount-sv], sv*sizeof(T*) );
      const size_t diff = FCount-sv;
      for( size_t i=1; i <= diff; i++ )
        Items[FCount-i] = Items[diff-i];
      memcpy(&Items[0], &D[0], sv*sizeof(T*));
      delete [] D;
    }
  }
//..............................................................................
  inline void Swap(size_t i, size_t j)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, i, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, j, 0, FCount);
#endif
    T *D = Items[i];
    Items[i] = Items[j];
    Items[j] = D;
  }
//..............................................................................
  void Move(size_t from, size_t to)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, from, 0, FCount);
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, to, 0, FCount);
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
  void Pack()  {
    size_t nc = 0;  // count null pointers
    for( size_t i=0; i < FCount; i++, nc++ )  {
      if( Items[i] == NULL )  {
        nc--;
        continue;
      }
      if( i != nc )
        Items[nc] = Items[i];
    }
    FCount = nc;
  }
//..............................................................................
  inline void Shrink() {
    FCapacity = olx_max(1,FCount); // 0 will cause realloc to delete Items, causing troubles
    Allocate();
  }
//..............................................................................
  inline size_t Count() const {  return FCount;  }
//..............................................................................
  inline bool IsEmpty() const {  return (FCount == 0);  }
//..............................................................................
  inline void SetCount(size_t v)  {
    if( v == FCount )  return;
#ifdef _OLX_DEBUG
   // TODO: check if v is valid
#endif
    if( v > FCount )  {
      if( v > FCapacity )
        SetCapacity(v + FIncrement);
      FCount = v;
    }
    else  {  // shrinking to exact size
      FCount = v;
      Shrink();
    }
  }
//..............................................................................
  size_t IndexOf(const T* val) const {
    for( size_t i=0; i < FCount; i++ )
      if( Items[i] == val )  
        return i;
    return InvalidIndex;
  }
//..............................................................................
  size_t IndexOf(const T& val) const {
    const T* pv = &val;
    for( size_t i=0; i < FCount; i++ )
      if( Items[i] == pv )  
        return i;
    return InvalidIndex;
  }

  void Rearrange(const TSizeList& indexes)  {
    if( FCount < 2 )  return;
#ifdef _OLX_DEBUG
    if( Count() != indexes.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "size mismatch");
#endif
      // allocate the list of NULLs
      T** ni = (T**)malloc(FCount*sizeof(T*));
      for( size_t i=0; i < FCount; i++ )  {
        ni[i] = Items[indexes[i]];
      }
      free(Items);
      Items = ni;
  }

  static TQuickPtrSorter<TPtrList<T>,T> QuickSorter;
  static TBubblePtrSorter<TPtrList<T>,T> BubleSorter;
  static TListTraverser<TPtrList<T> > Traverser;
};

#ifndef __BORLANDC__
template <class T>
  TQuickPtrSorter<TPtrList<T>,T> TPtrList<T>::QuickSorter;
template <class T>
  TBubblePtrSorter<TPtrList<T>,T> TPtrList<T>::BubleSorter;
template <class T>
  TListTraverser<TPtrList<T> > TPtrList<T>::Traverser;
#endif

EndEsdlNamespace()
#endif



