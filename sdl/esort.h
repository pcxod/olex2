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

//.............................................................................
class TPrimitiveComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA& A, const ComparableB& B)  {
    if( olx_ref::get(A) < olx_ref::get(B) )  return -1;
    if( olx_ref::get(A) > olx_ref::get(B) )  return 1;
    return 0;
  }
};
//.............................................................................
class TPointerComparator  {
public:
  template <class ComparableA, class ComparableB>
  static inline int Compare(const ComparableA& A, const ComparableB& B)  {
    if( olx_ptr::get(A) < olx_ptr::get(B) )  return -1;
    if( olx_ptr::get(A) > olx_ptr::get(B) )  return 1;
    return 0;
  }
};
//.............................................................................
class TComparableComparator  {
public:
  template <class ComparableA, class ComparableB>
  static int Compare(const ComparableA& A, const ComparableB& B)  {
    return olx_ref::get(A).Compare(olx_ref::get(B));
  }
};
//.............................................................................
struct FunctionComparator {
  template <class Base, typename ItemClass> struct ComparatorMF_ {
    Base& Instance;
    int (Base::*Func)(const ItemClass &a, const ItemClass &b);
    ComparatorMF_(Base& instance,
      int (Base::*func)(const ItemClass &a, const ItemClass &b))
      :  Instance(instance), Func(func) {}
    template <typename item_t>
    int Compare(const item_t &a, const item_t &b) const {
      return (Instance.*Func)(olx_ref::get(a), olx_ref::get(b));
    }
  };
  template <class Base, typename ItemClass> struct ComparatorCMF_ {
    const Base& Instance;
    int (Base::*Func)(const ItemClass &a, const ItemClass &b) const;
    ComparatorCMF_(const Base& instance,
      int (Base::*func)(const ItemClass &a, const ItemClass &b) const)
      :  Instance(instance), Func(func) {}
    template <typename item_t>
    int Compare(const item_t &a, const item_t &b) const {
      return (Instance.*Func)(olx_ref::get(a), olx_ref::get(b));
    }
  };
  template <typename ItemClass> struct ComparatorSF_ {
    int (*Func)(const ItemClass &a, const ItemClass &b);
    ComparatorSF_(
      int (*func)(const ItemClass &a, const ItemClass &b))
      : Func(func)  {}
    template <typename item_t>
    int Compare(const item_t &a, const item_t &b) const {
      return (*Func)(olx_ref::get(a), olx_ref::get(b));
    }
  };

  template <typename item_t, typename base_t> static
  ComparatorMF_<base_t,item_t> Make(
    base_t &base,
    int (base_t::*func)(const item_t &, const item_t&) const)
  {
    return ComparatorMF_<base_t,item_t>(base, func);
  }
  template <typename item_t, typename base_t> static
  ComparatorCMF_<base_t,item_t> MakeConst(
    const base_t &base,
    int (base_t::*func)(const item_t &, const item_t&) const)
  {
    return ComparatorCMF_<base_t,item_t>(base, func);
  }

  template <typename item_t> static
  ComparatorSF_<item_t> Make(
    int (*func)(const item_t &, const item_t&))
  {
    return ComparatorSF_<item_t>(func);
  }
};
//.............................................................................

struct ReverseComparator {
  template <class cmp_t> struct ReverseComparator_ {
    const cmp_t &cmp;
    ReverseComparator_(const cmp_t &cmp_) : cmp(cmp_) {}
    template <typename item_t> int Compare(
      const item_t &i1, const item_t &i2) const
    {
      return cmp.Compare(i2, i1);
    }
  };
  template <typename item_t> struct ReverseSF {
    int (*func)(const item_t &, const item_t&);
    ReverseSF(int (*func_)(const item_t &, const item_t&)) : func(func_) {}
    template <class i_t>
    int Compare(const i_t &i1, const i_t &i2) const {
      return (*func)(olx_ref::get(i2), olx_ref::get(i1));
    }
  };
  template <class base_t, typename item_t> struct ReverseMF {
    base_t &instance;
    int (base_t::*func)(const item_t &, const item_t&);
    ReverseMF(base_t &instance_,
      int (base_t::*func_)(const item_t &, const item_t&))
    : instance(instance_), func(func_)
    {}
    template <class i_t>
    int Compare(const i_t &i1, const i_t &i2) const {
      return (instance.*func)(olx_ref::get(i2), olx_ref::get(i1));
    }
  };
  template <class base_t, typename item_t> struct ReverseCMF {
    const base_t &instance;
    int (base_t::*func)(const item_t &, const item_t&) const;
    ReverseCMF(const base_t &instance_,
      int (base_t::*func_)(const item_t &, const item_t&) const)
    : instance(instance_), func(func_)
    {}
    template <class i_t>
    int Compare(const i_t &i1, const i_t &i2) const {
      return (instance.*func)(olx_ref::get(i2), olx_ref::get(i1));
    }
  };

  template <typename item_t>
   static ReverseSF<item_t> Make(int (*func)(const item_t &, const item_t&)) {
     return ReverseSF<item_t>(func);
  }
  template <class base_t, typename item_t>
   static ReverseMF<base_t, item_t> Make(base_t &instance,
     int (base_t::*func)(const item_t &, const item_t&))
  {
    return ReverseMF<base_t, item_t>(instance, func);
  }
  template <class base_t, typename item_t>
   static ReverseCMF<base_t, item_t> MakeConst(const base_t &instance,
     int (base_t::*func)(const item_t &, const item_t&) const)
  {
    return ReverseCMF<base_t, item_t>(instance, func);
  }
  template <class cmp_t>
   static ReverseComparator_<cmp_t> Make(const cmp_t& cmp)
  {
    return ReverseComparator_<cmp_t>(cmp);
  }
};

//.............................................................................
struct DummySwapListener  {
  static void OnSwap(size_t, size_t)  {}
};

struct SyncSwapListener {
  template <typename List> struct SyncSwapListener_ {
    List& list;
    SyncSwapListener_(List& _list) : list(_list)  {}
    void OnSwap(size_t i, size_t j) const {  list.Swap(i, j);  }
  };
  template <class list_t> static SyncSwapListener_<list_t> Make(
    list_t &l)
  {
    return SyncSwapListener_<list_t>(l);
  }
};
//.............................................................................
template <class Sorter>
struct SortInterface {
  template <class list_t, class accessor_t, class comparator_t,
            class listener_t>
  static void Sort(list_t &list, const accessor_t &acc,
    const comparator_t &cmp, const listener_t &listener)
  {
    Sorter::Make(list, acc, cmp, listener).Sort();
  }
  template <class list_t, class comparator_t, class listener_t>
  static void Sort(list_t &list, const comparator_t& cmp,
    const listener_t& l)
  {
    
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      cmp, l).Sort();
  }
  template <class list_t, class comparator_t>
  static void Sort(list_t &list, const comparator_t &cmp)  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      cmp,
      DummySwapListener()).Sort();
  }
  template <class list_t>
  static void Sort(list_t &list)
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      TComparableComparator(),
      DummySwapListener()).Sort();
  }

  template <class list_t, class item_t>
  static void SortSF(list_t &list, int (*f)(const item_t&, const item_t&))
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      FunctionComparator::Make(f),
      DummySwapListener()).Sort();
  }
  template <class list_t, class base_t, class item_t>
  static void SortMF(list_t &list,
    const base_t &base,
    int (base_t::*f)(const item_t&, const item_t&) const)
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      FunctionComparator::MakeConst(base, f),
      DummySwapListener()).Sort();
  }
};
struct QuickSorter : public SortInterface<QuickSorter> {
  template <class list_t, class accessor_t, class comparator_t,
            class listener_t>
  struct QuickSorter_ {
    QuickSorter_(list_t& list_, const accessor_t &accessor_,
      const comparator_t& comparator_, const listener_t& listener_)
      : list(list_), accessor(accessor_), listener(listener_),
        cmp(comparator_)
    {}
    void Sort()  {
      if( list.list.Count() < 2 )  return;
      DoSort(0, list.list.Count()-1);
    }
  protected:
    typename list_t::InternalAccessor list;
    const accessor_t& accessor;
    const listener_t& listener;
    const comparator_t& cmp;
    void DoSort(size_t lo0, size_t hi0)  {
      const size_t diff = hi0-lo0;
      if( diff == 1 )  {
        if( cmp.Compare(
          olx_ref::get(accessor(list[lo0])),
          olx_ref::get(accessor(list[hi0]))) > 0 )
        {
          listener.OnSwap(lo0, hi0);
          list.list.Swap(lo0, hi0);
        }
      }
      else if( diff > 0 ) {
        size_t lo = lo0;
        size_t hi = hi0;
        const size_t m_ind = (lo0 + hi0)/2;
        typename accessor_t::return_type mid = accessor(list[m_ind]);
        while( lo <= hi )  {
          while( cmp.Compare(
            olx_ref::get(accessor(list[lo])),
            olx_ref::get(mid)) < 0 )
          {
            if( ++lo >= hi0 )  break;
          }
          while( cmp.Compare(
            olx_ref::get(accessor(list[hi])),
            olx_ref::get(mid)) > 0 )
          {
            if( --hi <= lo0 )  break;
          }
          if( lo <= hi )  {
            if( lo != hi )  {
              listener.OnSwap(lo, hi);
              list.list.Swap(lo, hi);
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
  template <class list_t, class accessor_t, class comparator_t,
            class listener_t> static
  QuickSorter_ <list_t,accessor_t,comparator_t,listener_t> Make(
    list_t& list, const accessor_t &acc, const comparator_t& cmp,
    const listener_t& listener)
  {
    return QuickSorter_<list_t,accessor_t,comparator_t,listener_t>(
      list, acc, cmp, listener);
  }
};
//.............................................................................
struct BubbleSorter : public SortInterface<BubbleSorter> {
  template <class list_t, class accessor_t,
            class comparator_t, class listener_t>
  struct BubbleSorter_  {
    BubbleSorter_(list_t &list_, const accessor_t &accessor_,
      const comparator_t &cmp_, const listener_t &listener_)
      : list(list_), accessor(accessor_), cmp(cmp_),
        listener(listener_)  {}
    void Sort()  {
      bool changes = true;
      const size_t lc = list.list.Count();
      while( changes )  {
        changes = false;
        for( size_t i=1; i < lc; i++ )  {
          if( cmp.Compare(
            olx_ref::get(accessor(list[i-1])),
            olx_ref::get(accessor(list[i]))) > 0 )
          {
            list.list.Swap(i-1, i);
            listener.OnSwap(i-1, i);
            changes = true;
          }
        }
      }
    }
  protected:
    typename list_t::InternalAccessor list;
    const accessor_t &accessor;
    const comparator_t &cmp;
    const listener_t &listener;
  };
  template <class list_t, class accessor_t,
            class comparator_t, class listener_t> static
  BubbleSorter_<list_t,accessor_t,comparator_t,listener_t> Make(
    list_t &list, const accessor_t &accessor,
    const comparator_t &cmp, const listener_t &listener)
  {
    return BubbleSorter_<list_t,accessor_t,comparator_t,listener_t>(
      list, accessor, cmp, listener);
  }
};
//.............................................................................
EndEsdlNamespace()
#endif
