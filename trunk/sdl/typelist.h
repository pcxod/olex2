//---------------------------------------------------------------------------
#ifndef typelistH
#define typelistH
#include "esort.h"
#include "etraverse.h"
#include "exception.h"
#include "talist.h"
#include "tptrlist.h"

BeginEsdlNamespace()

template <class T, class DestructCast> class TTypeListExt : public IEObject  {
private:
  // initialisation function
  inline T& Alloc(size_t i)  {  return *(T*)( List[i] = new T() );  }
protected:
  TPtrList<T> List;
public:
  // creates a new empty objects
  TTypeListExt() {  }
  // allocates size elements (can be accessed diretly)
  TTypeListExt(int size) : List(size)  {  }
//..............................................................................
  /* copy constuctor - creates new copies of the objest, be careful as the copy
   constructor must exist for nonpointer objects */
  TTypeListExt( const TTypeListExt& list ) : List(list.Count())  {
    for( int i=0; i < list.Count(); i++ )
      List[i] =  new T(list[i]);
  }
//..............................................................................
  /* copies values from an array of size elements  */
  TTypeListExt( size_t size, const T* array ) : List(size)  {
    for( int i=0; i < size; i++ )
      Alloc(i) =  array[i];
  }
//..............................................................................
  //destructor - beware t40: error: expecthe objects are deleted!
  virtual ~TTypeListExt()  {
    Clear();
  }
//..............................................................................
  //deletes the objects and clears the list
  void Clear()  {
    for( int i=0; i < List.Count(); i++ )
      delete (DestructCast*)List[i];
    List.Clear();
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
  /* copy constuctor - creates new copies of the objest, be careful as the assignement
   operator must exist  */
  void AddList( const TTypeListExt& list )  {
    List.SetCapacity( list.Count() + List.Count() );
    for( int i=0; i < list.Count(); i++ )  {
      T* o = new T();
      *o = list[i];
      List.Add(o);
    }
  }
//..............................................................................
//  operator const TEList& () const { return *FList;  }
//  TEList& c_list() { return *FList;  }
//..............................................................................
  //adds a new object ito the list - will be deleted
  inline T& Add(T& Obj)  {  return *List.Add(&Obj);  }
//..............................................................................
  //adds a new object ito the list - will be deleted
  inline T& Add(T* Obj)  {  return *List.Add(Obj);  }
//..............................................................................
  //sets the list item to an object, which will be deleted
  inline T& Set(size_t index, T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return *(T*)(List[index] = &Obj);
  }
//..............................................................................
  // adds a copy of the object with default constructor  "assigned copy"
  inline const T& AddACopy(const T& Obj)  {  return (AddNew() = Obj);  }
//..............................................................................
  //sets the listitem to an new object copied by the assignement operator
  inline const T& SetACopy(size_t index, const T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return (Alloc(index) = Obj);
  }
//..............................................................................
  // adds a copy of the object with copy constructor "copied copy"
  inline const T& AddCCopy(const T& Obj)  {  return AddNew<T>(Obj);  }
//..............................................................................
  //sets the listitem to an new object copied by the copy constructor
  inline const T& SetCCopy(size_t index, const T& Obj)  {
    if( List[index] != NULL )
      delete (DestructCast*)List[index];
    return *(T*)( List[index] = new T(Obj) );
  }
//..............................................................................
//..............................................................................
  // assigned copy inserted
  inline const T& InsertACopy(size_t index, const T& Obj)  {  return (InsertNew(index) = Obj);  }
//..............................................................................
  // insert anobject into thelist; the object will be deleted
  inline const T& Insert(size_t index, T& Obj)  {  return *List.Insert(index, &Obj);  }
//..............................................................................
  // copy constructor created copy is inserted
  inline const T& InsertCCopy(size_t index, const T& Obj)  {  return InsertNew<T>(index, Obj);  }
//..............................................................................
  //inerts a new object at specified position
  inline T& InsertNew(size_t index)  {  return *List.Insert(index, new T());  }
//..............................................................................
  template<class AC>
    inline T& InsertNew( size_t index, const AC& arg )  {
      return *List.Insert(index, new T(arg));  }
//..............................................................................
  template<class FAC, class SAC>
    inline T& InsertNew( size_t index, const FAC& arg1, const SAC& arg2 )  {
      return *List.Insert(index, new T(arg1, arg2));  }
//..............................................................................
  template<class FAC, class SAC, class TAC>
    inline T& InsertNew( size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3)  {
      return *List.Insert(index, new T(arg1, arg2, arg2));  }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
    inline T& InsertNew( size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4)  {
      return *List.Insert(index, new T(arg1, arg2, arg2, arg4));  }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
    inline T& InsertNew( size_t index, const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4, const FvAC& arg5)  {
      return *List.Insert(index, new T(arg1, arg2, arg2, arg4, arg5));  }
//..............................................................................
  inline T& AddNew()  {  return *List.Add(new T());  }
//..............................................................................
  template<class AC>
    inline T& AddNew( const AC& arg )  {
      return *List.Add(new T(arg));  }
//..............................................................................
  template<class FAC, class SAC>
    inline T& AddNew( const FAC& arg1, const SAC& arg2 )  {
      return *List.Add(new T(arg1, arg2));  }
//..............................................................................
  template<class FAC, class SAC, class TAC>
    inline T& AddNew( const FAC& arg1, const SAC& arg2, const TAC& arg3 )  {
      return *List.Add(new T(arg1, arg2, arg3));  }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC>
    inline T& AddNew( const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4)  {
      return *List.Add(new T(arg1, arg2, arg3, arg4));  }
//..............................................................................
  template<class FAC, class SAC, class TAC, class FrAC, class FvAC>
    inline T& AddNew( const FAC& arg1, const SAC& arg2, const TAC& arg3, const FrAC& arg4, const FvAC& arg5)  {
      return *List.Add(new T(arg1, arg2, arg3, arg4, arg5));  }
//..............................................................................
  inline T& operator [] (size_t index) const {
#ifdef _OLX_DEBUG
    T*& v = List[index];
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List[index];
#endif
  }
//..............................................................................
  inline T& Item(size_t index) const  {
#ifdef _OLX_DEBUG
    T*& v = List[index];
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List[index];
#endif
  }
//..............................................................................
  inline T& Last() const  {
#ifdef _OLX_DEBUG
    T*& v = List.Last();
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
    return *v;
#else
    return *List.Last();
#endif
  }
//..............................................................................
  inline bool IsNull(size_t index) const  {  return List[index] == NULL;  }
//..............................................................................
  // beware that poiter assignement will not work with syntaxes above, use this function instead!
  inline void NullItem(size_t index) const  {
    T* v = List[index];
    if( v != NULL )  {  // check if not deleted yet
      delete (DestructCast*)v;
      List[index] = NULL;
    }
  }
//..............................................................................
  /* copy - creates new copies of the objest, be careful as the copy constructor
   must exist  */
  const TTypeListExt& operator = ( const TTypeListExt& list )  {
    for( int i=0; i < List.Count(); i++ )
      delete (DestructCast*)List[i];
    List.SetCount( list.Count() );
    for( int i=0; i < list.Count(); i++ ) 
      List[i] =  new T(list[i]);
    return list;
  }
//..............................................................................
  inline void SetCapacity(size_t v)   {  List.SetCapacity(v);  }
//..............................................................................
  inline void SetIncrement(size_t v)  {  List.SetIncrement(v);  }
//..............................................................................
  void Delete(size_t index)  {
    // check if already deleted
    if( List[index] == NULL )  return;
    delete (DestructCast*)List[index];
    List.Delete(index);
  }
//..............................................................................
  void DeleteRange(size_t from, size_t to)  {
    for( int i=from; i < to; i++ )  {
      if( List[i] != NULL )  {  // check if not deleted yet
        delete (DestructCast*)List[i];
        List[i] = NULL;
      }
    }
    List.Pack();
  }
//..............................................................................
  void Shrink(size_t newSize)  {
    if( !(newSize >=0 && newSize < (size_t)List.Count()) )  return;
    for( int i=newSize; i < List.Count(); i++ )
      if( List[i] != NULL )
        delete (DestructCast*)List[i];
    List.SetCount( newSize );
  }
//..............................................................................
  // the memory has to be delalocated by calling process
  T& Release(size_t index)  {
    T*& v = List[index];
#ifdef _OLX_DEBUG
    if( v == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot dereference a NULL pointer");
#endif
    List.Delete(index);
    return *v;
  }
//..............................................................................
  void Remove(const T& Obj)  {
    int ind = IndexOf(Obj);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "object is not in the list");
    Delete( ind );
  }
//..............................................................................
  /* rearranges the list according to provided indexes. Indexes must be unique unless
    objects are pointers
  */
  void Rearrange(const TIntList& indexes)  {
#ifdef _OLX_DEBUG
    if( Count() != indexes.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "size mismatch");
#endif
    List.Rearrange(indexes);
  }
//..............................................................................
  // cyclic shift to the left
  inline void ShiftL(int cnt)  {  List.ShiftL(cnt);  }
//..............................................................................
  // cyclic shift to the right
  inline void ShiftR(int cnt)  {  List.ShiftR(cnt);  }
//..............................................................................
  inline void Swap(size_t i, size_t j)  {  List.Swap(i, j);  }
//..............................................................................
  inline void Move(size_t from, size_t to)  {  List.Move(from, to);  }
//..............................................................................
  inline void Pack()  {  List.Pack();  }
//..............................................................................
  inline int Count()    const {  return List.Count();  }
//..............................................................................
  inline bool IsEmpty() const {  return List.IsEmpty();  }
//..............................................................................
  // the comparison operator is used
  int IndexOf(const T& val) const  {
    for( int i=0; i < List.Count(); i++ )
      if( *List[i] == val )  return i;
    return -1;
  }

  static TQuickSorter<TTypeListExt<T,DestructCast>,T> QuickSorter;
  static TBubbleSorter<TTypeListExt<T,DestructCast>,T> BubleSorter;
  static TListTraverser<TTypeListExt<T,DestructCast> > Traverser;
};
#ifndef __BORLANDC__
template <class T, class DC>
  TQuickSorter<TTypeListExt<T,DC>,T> TTypeListExt<T,DC>::QuickSorter;
template <class T, class DC>
  TBubbleSorter<TTypeListExt<T,DC>,T> TTypeListExt<T,DC>::BubleSorter;
template <class T, class DC>
  TListTraverser<TTypeListExt<T,DC> > TTypeListExt<T,DC>::Traverser;
#endif
template <class T>
  class TTypeList : public TTypeListExt<T,T>  {
  public:
   TTypeList() : TTypeListExt<T,T>()  {  ;  }
   TTypeList(const size_t size) : TTypeListExt<T,T>(size)  {  ;  }
   TTypeList(const TTypeList& list) : TTypeListExt<T,T>(list)  {  ;  }
   TTypeList(size_t size, const T* array) : TTypeListExt<T,T>(size, array)  {  ;  }
};

template <>
  class TTypeList<int> : public TTypeListExt<int,int>  {
  public:
   TTypeList() : TTypeListExt<int,int>()  {  ;  }
   TTypeList(const int size) : TTypeListExt<int,int>(size)  {  ;  }
   TTypeList(const TTypeList& list) : TTypeListExt<int,int>(list)  {  ;  }
   TTypeList(int size, const int* array) : TTypeListExt<int,int>(size, array)  {  ;  }
};

EndEsdlNamespace()
#endif



