/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_esort_H
#define __olx_sdl_esort_H
#include "ebase.h"
BeginEsdlNamespace()

/* a comparator for primitive types, or object having < and > operators only
   the comparison might call both operators, so use the TComparableComparator
   for whose objects which implement Compare function to improve the speed

   Note that a pointer lists should be treated differently - as the Item(i)
   method returns a reference to the location and considering the implementation
   of QS that location will get changed as Swap operations used!
*/

class TPrimitivePtrComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA* A, const ComparableB* B )  {
    if( *A < *B )  return -1;
    if( *A > *B )  return 1;
    return 0;
  }
};

class TPrimitiveComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA& A, const ComparableB& B )  {
    if( A < B )  return -1;
    if( A > B )  return 1;
    return 0;
  }
};

class TPointerPtrComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA* A, const ComparableB* B )  {
    if( A < B )  return -1;
    if( A > B )  return 1;
    return 0;
  }
};

class TPointerComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA& A, const ComparableB& B )  {
    if( &A < &B )  return -1;
    if( &A > &B )  return 1;
    return 0;
  }
};

class TComparablePtrComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA* A, const ComparableB* B )  {
    return A->Compare(*B);
  }
};

class TComparableComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA& A, const ComparableB& B )  {
    return A.Compare(B);
  }
};

template <class Base, typename ItemClass> struct Sort_ConstMemberFunctionWrapper  {
  Base& Instance;
  int (Base::*Func)(ItemClass a, ItemClass b) const;
  Sort_ConstMemberFunctionWrapper(Base& instance, int (Base::*func)(ItemClass a, ItemClass b) const) :
  Instance(instance), Func(func) {  }
  inline int Compare(ItemClass a, ItemClass b) const {
    return (Instance.*Func)(a, b);
  }
};

template <class Base, typename ItemClass> struct Sort_MemberFunctionWrapper  {
  Base& Instance;
  int (Base::*Func)(ItemClass a, ItemClass b);
  Sort_MemberFunctionWrapper(Base& instance, int (Base::*func)(ItemClass a, ItemClass b)) :
  Instance(instance), Func(func) {  }
  inline int Compare(ItemClass a, ItemClass b) const {
    return (Instance.*Func)(a, b);
  }
};

template <class Comparator, class ItemClass> struct Sort_ComparatorWrapper  {
  inline int Compare(ItemClass a, ItemClass b) const {
    return Comparator::Compare(a, b);
  }
};

template <typename ItemClass> struct Sort_StaticFunctionWrapper  {
  int (*Func)(ItemClass a, ItemClass b);
  Sort_StaticFunctionWrapper(int (*func)(ItemClass a, ItemClass b)) : Func(func)  {}
  inline int Compare(ItemClass a, ItemClass b) const {
    return (*Func)(a, b);
  }
};

struct DummySwapListener  {
  static void OnSwap(size_t i, size_t j)  {}
};
template <typename List> struct SyncSwapListener  {
  List& list;
  SyncSwapListener(List& _list) : list(_list)  {}
  void OnSwap(size_t i, size_t j) const {  list.Swap(i, j);  }
};
//.........................................................................................................................
template <class List, class Item, class accessor, class Comparator, class Listener>
struct QuickSorter  {
  QuickSorter(List& _list, const Comparator& _cmp, const Listener& _listener) :
    list(_list), cmp(_cmp), listener(_listener)  {}
  void Sort()  {
    if( list.Count() < 2 )  return;
    DoSort(0, list.Count()-1);
  }
protected:
  List& list;
  const Listener& listener;
  const Comparator& cmp;
  void DoSort(size_t lo0, size_t hi0)  {
    const size_t diff = hi0-lo0;
    if( diff == 1 )  {
      if( cmp.Compare(accessor::get(list, lo0), accessor::get(list, hi0)) > 0 )  {
        listener.OnSwap(lo0, hi0);
        list.Swap(lo0, hi0);
      }
    }
    else if( diff > 0 ) {
      size_t lo = lo0;
      size_t hi = hi0;
      const size_t m_ind = (lo0 + hi0)/2;
      Item mid = accessor::get(list, m_ind);
      while( lo <= hi )  {
        while( cmp.Compare(accessor::get(list, lo), mid) < 0 )  {
          if( ++lo >= hi0 )  break;
        }
        while( cmp.Compare(accessor::get(list, hi), mid) > 0 )  {
          if( --hi <= lo0 )  break;
        }
        if( lo <= hi )  {
          if( lo != hi )  {
            listener.OnSwap(lo, hi);
            list.Swap(lo, hi);
          }
          lo++;
          if( --hi == InvalidIndex )
            break;
        }
      }
      if( lo0 < hi && hi != InvalidIndex )
        DoSort(lo0, hi);
      if( lo < hi0 )
        DoSort(lo, hi0);
    }
  }
};
//.........................................................................................................................
template <class List, class Item, class accessor, class Comparator, class Listener>
struct BubbleSorter  {
  BubbleSorter(List& _list, const Comparator& _cmp, const Listener& _listener) :
    list(_list), cmp(_cmp), listener(_listener)  {}
  void Sort()  {
    bool changes = true;
    const size_t lc = list.Count();
    while( changes )  {
      changes = false;
      for( size_t i=1; i < lc; i++ )  {
        if( cmp.Compare(accessor::get(list, i-1), accessor::get(list, i)) > 0 )  {
          list.Swap(i-1, i);
          changes = true;
        }
      }
    }
  }
protected:
  List& list;
  const Listener& listener;
  const Comparator& cmp;
};
//.........................................................................................................................
template <class ListClass, class ItemClass, class Accessor> class ListQuickSorter  {
public:
  template <class Comparator>
  static void Sort(ListClass& list)  {
    QuickSorter<ListClass, ItemClass, Accessor, Comparator, DummySwapListener>(list, Comparator(), DummySwapListener()).Sort();
  }
  template <class Comparator>
  static void Sort(ListClass& list, const Comparator& cmp)  {
    QuickSorter<ListClass, ItemClass, Accessor, Comparator, DummySwapListener>(list, cmp, DummySwapListener()).Sort();
  }
  template <class Comparator, class SwapListener>
  static void Sort(ListClass& list, const Comparator& cmp, const SwapListener& sl)  {
    QuickSorter<ListClass, ItemClass, Accessor, Comparator, SwapListener>(list, cmp, sl).Sort();
  }
  // convenience functions
  static void SortSF(ListClass& list, int (*f)(ItemClass a, ItemClass b))  {
    QuickSorter<ListClass, ItemClass, Accessor, Sort_StaticFunctionWrapper<ItemClass>, DummySwapListener>(
      list, Sort_StaticFunctionWrapper<ItemClass>(f), DummySwapListener()).Sort();
  }
  template <class BaseClass>
  static void SortMF(ListClass& list, BaseClass& base, int (BaseClass::*f)(ItemClass a, ItemClass b) const)
  {
    QuickSorter<ListClass, ItemClass, Accessor,
      Sort_ConstMemberFunctionWrapper<BaseClass,ItemClass>,
      DummySwapListener>(
        list, Sort_ConstMemberFunctionWrapper<BaseClass, ItemClass>(base, f),
        DummySwapListener()).Sort();
  }
  template <class BaseClass>
  static void SortMF(ListClass& list, BaseClass& base, int (BaseClass::*f)(ItemClass a, ItemClass b))
  {
    QuickSorter<ListClass, ItemClass, Accessor,
      Sort_MemberFunctionWrapper<BaseClass,ItemClass>,
      DummySwapListener>(
        list, Sort_MemberFunctionWrapper<BaseClass, ItemClass>(base, f),
        DummySwapListener()).Sort();
  }
};
//.........................................................................................................................
template <class ListClass, class ItemClass, class Accessor> class ListBubbleSorter  {
public:
  template <class Comparator>
  static void Sort(ListClass& list)  {
    BubbleSorter<ListClass, ItemClass, Accessor, Comparator, DummySwapListener>(list, Comparator(), DummySwapListener()).Sort();
  }
  template <class Comparator>
  static void Sort(ListClass& list, const Comparator& cmp)  {
    BubbleSorter<ListClass, ItemClass, Accessor, Comparator, DummySwapListener>(list, cmp, DummySwapListener()).Sort();
  }
  template <class Comparator, class SwapListener>
  static void Sort(ListClass& list, const Comparator& cmp, const SwapListener& sl)  {
    BubbleSorter<ListClass, ItemClass, Accessor, Comparator, SwapListener>(list, cmp, sl).Sort();
  }
  // convenience functions
  static void SortSF(ListClass& list, int (*f)(ItemClass a, ItemClass b))  {
    BubbleSorter<ListClass, ItemClass, Accessor, Sort_StaticFunctionWrapper<ItemClass>, DummySwapListener>(
      list, Sort_StaticFunctionWrapper<ItemClass>(f), DummySwapListener()).Sort();
  }
  template <class BaseClass>
  static void SortMF(ListClass& list, BaseClass& base, int (BaseClass::*f)(ItemClass a, ItemClass b) const)
  {
    BubbleSorter<ListClass, ItemClass, Accessor,
      Sort_ConstMemberFunctionWrapper<BaseClass,ItemClass>,
      DummySwapListener>(
        list, Sort_ConstMemberFunctionWrapper<BaseClass, ItemClass>(base, f),
        DummySwapListener()).Sort();
  }
  template <class BaseClass>
  static void SortMF(ListClass& list, BaseClass& base, int (BaseClass::*f)(ItemClass a, ItemClass b))
  {
    BubbleSorter<ListClass, ItemClass, Accessor,
      Sort_MemberFunctionWrapper<BaseClass,ItemClass>,
      DummySwapListener>(
        list, Sort_MemberFunctionWrapper<BaseClass, ItemClass>(base, f),
        DummySwapListener()).Sort();
  }
};

EndEsdlNamespace()
#endif
