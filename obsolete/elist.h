//----------------------------------------------------------------------------//
// namespace TEObjects: lists
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef elistH
#define elistH
//---------------------------------------------------------------------------
#include "ebase.h"
//#include "smart/ostring.h"

BeginEsdlNamespace()

//defines a sort function
typedef int (*ListSort)(const void *I1, const void *I2);
//---------------------------------------------------------------------------
class TListSort  {
public:
  TListSort()  {  return; }
  virtual ~TListSort(){  ; }
  virtual int Compare(void* I, void* I1) = 0;
};

template<class T> class TTypeListSort  {
public:
  TTypeListSort()  {  return; }
  virtual ~TTypeListSort(){  return; }
  virtual int Compare(T& I, T& I1) = 0;
};
//---------------------------------------------------------------------------
class TEList: public IEObject  {
  int FCount, FCapacity;
  short FIncrement;
  void init(int count);
protected:
  void **Items;
  void QuickSort(int lo0, int hi0, ListSort v);
  void QuickSort(int lo0, int hi0, TListSort *v);
public:
  /* creates an empty list */
  TEList()  {  init(0) ; }
  /* the copy constructor */
  TEList(const TEList& l)  {
    init( l.Count() );
    Assign(l);
  }
  /* creates a list of specified size and initialises it with provided data. the couns
  must be equal to the bumber of passed arguments AND NEGATIVE  */
  TEList(int count, ...);
  virtual ~TEList();
  // clears current content and assignes it to the provided list
  void Assign(const TEList& E);
  // addscontent of the list to current content
  void Add(const TEList& E);
  // adds the content of a list to current list
#ifdef _OLX_DEBUG  // the bodies defined later then; index checking is performed
  void *& operator [] (int index) const;
  void *& Item(int index) const;
  void *& Last() const;
#else
  inline void *& operator [] (int index) const {  return Items[index];  }
  inline void *& Item(int index)         const {  return Items[index];  }
  inline void *& Last()                  const {  return Items[FCount-1];  }
#endif
  void Add(void *);
  // adds an object to the end of the list
  void Insert( int index, void *D);
  //inserts an object at specified position 0 <=index <=Count
  void Delete(int index);
  // deletes an object from the list by index
  void DeleteRange(int From, int To);
  // deletes a range of objects from the list
  void Remove(const void *D);
  // removes an object from the list
  void Clear();

  inline int  Count()   const {  return FCount;  }
  inline int  Size()    const {  return FCount;  }
  inline bool IsEmpty() const {  return (FCount == 0);  }

  int  IndexOf(const void *D) const;

  /* set the number of extra items allocated by the object when the array is needed
   to be expanded; use the function to optimise Add, insert operations  */
  inline void SetIncrement(short v) {  FIncrement = v;  }

  /* expands the Items array but does not change the count; use to optimise
   routing add operations, when the final size of the list is known; use
   SetIncrement if the final size of the list is not known*/
  void SetCapacity(int v);

  //removes NULL pointers from the list; can be used to optimize deletion operations
  void Pack();

  // if v > current size, then the array is expanded and extra items are initialised
  // with NULL; otherwise only changes the FCount
  void SetCount(int v);
  // sorts the list using a function
  void Sort(ListSort v);
  // sorts using an object derived from TListSort
  void Sort(TListSort *LS);
  // swaps the position of two items in the list
  void Swap(int i, int j);
  // moves an element from one position ot another
  void Move(int from, int to);
  // cyclic shift to the left
  void ShiftL(int cnt);
  // cyclic shift to the right
  void ShiftR(int cnt);

  inline bool Contains(void *A){  return (IndexOf(A) >=0); }

};
//---------------------------------------------------------------------------


EndEsdlNamespace()
#endif
