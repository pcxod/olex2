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
#include "equeue.h"
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
struct DummySortListener {
  static void OnSwap(size_t, size_t)  {}
  static void OnMove(size_t, size_t)  {}
};

struct SyncSortListener {
  template <typename List> struct SyncSortListener_ {
    List& list;
    SyncSortListener_(List& _list) : list(_list)  {}
    void OnSwap(size_t i, size_t j) const {  list.Swap(i, j);  }
    void OnMove(size_t i, size_t j) const {  list.Move(i, j);  }
  };
  template <class list_t> static SyncSortListener_<list_t> Make(
    list_t &l)
  {
    return SyncSortListener_<list_t>(l);
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
      DummySortListener()).Sort();
  }
  template <class list_t>
  static void Sort(list_t &list)
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      TComparableComparator(),
      DummySortListener()).Sort();
  }

  template <class list_t, class item_t>
  static void SortSF(list_t &list, int (*f)(const item_t&, const item_t&))
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      FunctionComparator::Make(f),
      DummySortListener()).Sort();
  }
  template <class list_t, class base_t, class item_t>
  static void SortMF(list_t &list,
    const base_t &base,
    int (base_t::*f)(const item_t&, const item_t&) const)
  {
    Sorter::Make(list,
      TDirectAccessor<typename list_t::InternalAccessor::list_item_type>(),
      FunctionComparator::MakeConst(base, f),
      DummySortListener()).Sort();
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
    void Sort() {
      size_t cnt = list.list.Count();
      if (cnt < 2)  return;
      DoSort(0, cnt - 1);
    }
  protected:
    typename list_t::InternalAccessor list;
    const accessor_t& accessor;
    const listener_t& listener;
    const comparator_t& cmp;
    void DoSort(size_t lo0_, size_t hi0_) {
      typedef AnAssociation3<size_t, size_t, bool> d_t;
      TQueue<d_t> stack;
      stack.Push(d_t(lo0_, hi0_, true));
      while (!stack.IsEmpty()) {
        const d_t tv = stack.Pop();
        const size_t diff = tv.GetB() - tv.GetA();
        if (diff == 1) {
          if (cmp.Compare(
            olx_ref::get(accessor(list[tv.GetA()])),
            olx_ref::get(accessor(list[tv.GetB()]))) > 0)
          {
            listener.OnSwap(tv.GetA(), tv.GetB());
            list.list.Swap(tv.GetA(), tv.GetB());
          }
        }
        else if (diff > 0) {
          size_t pi = tv.GetA() + (diff >> 1);
          if (tv.GetC()) { // try to fix pivot
            size_t inc = (diff >> 2), li = pi - inc, ri = pi + inc;
            int c1 = cmp.Compare(
              olx_ref::get(accessor(list[pi])),
              olx_ref::get(accessor(list[li])));
            int c2 = cmp.Compare(
              olx_ref::get(accessor(list[pi])),
              olx_ref::get(accessor(list[ri])));
            if ((c1 > 0 && c2 > 0) || (c1 < 0 && c2 < 0)) {
              int c3 = cmp.Compare(
                olx_ref::get(accessor(list[li])),
                olx_ref::get(accessor(list[ri])));
              if (c1 < 0) { // li/ri < mi
                pi = (c3 < 0 ? ri : li);
              }
              else { // mi < li/ri
                pi = (c3 < 0 ? li : ri);
              }
            }
          }
          typename accessor_t::return_type mid = accessor(list[pi]);
          listener.OnSwap(pi, tv.GetB());
          list.list.Swap(pi, tv.GetB());
          pi = tv.GetA();
          for (size_t i = tv.GetA(); i < tv.GetB(); i++) {
            if (cmp.Compare(
              olx_ref::get(accessor(list[i])),
              olx_ref::get(mid)) <= 0)
            {
              listener.OnSwap(i, pi);
              list.list.Swap(i, pi);
              pi++;
            }
          }
          listener.OnSwap(pi, tv.GetB());
          list.list.Swap(pi, tv.GetB());
          size_t l1 = pi - tv.GetA(), l2 = tv.GetB() - pi,
            ml = olx_max(l1, l2);
          bool sp = (l1 == 0 || l2 == 0) ? true
            : (ml > 32 && (double)ml / (double)olx_min(l1, l2) >= 1.75);
          if (l2 > 0)
            stack.Push(d_t(pi + 1, tv.GetB(), sp ? l2 == ml : false));
          if (l1 > 0)
            stack.Push(d_t(tv.GetA(), pi - 1, sp ? l1 == ml : false));
        }
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
      size_t lc = list.list.Count();
      while (lc > 0) {
        size_t nlc = 0;
        for (size_t i=0; i < lc-1; i++) {
          if (cmp.Compare(
            olx_ref::get(accessor(list[i+1])),
            olx_ref::get(accessor(list[i]))) < 0)
          {
            list.list.Swap(i+1, i);
            listener.OnSwap(i+1, i);
            nlc = i+1;
          }
        }
        lc = nlc;
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
struct MoveSorter : public SortInterface<MoveSorter> {
  template <class list_t, class accessor_t,
  class comparator_t, class listener_t>
  struct MoveSorter_  {
    typedef typename accessor_t::return_type item_t;
    MoveSorter_(list_t &list_, const accessor_t &accessor_,
    const comparator_t &cmp_, const listener_t &listener_)
    : list(list_), accessor(accessor_), cmp(cmp_),
      listener(listener_)
    {}
    size_t findIndex(size_t sz) {
      if (cmp.Compare(olx_ref::get(list[sz]),
          olx_ref::get(accessor(list[0]))) <= 0)
      {
        return 0;
      }
      if (cmp.Compare(olx_ref::get(list[sz]),
          olx_ref::get(accessor(list[sz-1]))) >= 0)
      {
        return sz;
      }
      size_t from = 0, to = sz-1;
      while (true) {
        if ((to-from) == 1) return to;
        const size_t index = from + (to-from) / 2;
        const int cr = cmp.Compare(olx_ref::get(list[index]),
          olx_ref::get(accessor(list[sz])));
        if (cr < 0)
          from = index;
        else if (cr > 0)
          to = index;
        else
          return index;
      }
      return from;
    }
    void Sort() {
      size_t lc = list.list.Count();
      if (lc < 2) return;
      if (cmp.Compare(olx_ref::get(list[0]),
        olx_ref::get(accessor(list[1]))) > 0)
      {
        list.list.Move(1, 0);
        listener.OnMove(1, 0);
      }
      size_t sorted_sz = 2;
      while (sorted_sz != lc) {
        size_t np = findIndex(sorted_sz);
        if (np != sorted_sz) {
          list.list.Move(sorted_sz, np);
          listener.OnMove(sorted_sz, np);
        }
        sorted_sz++;
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
    MoveSorter_<list_t, accessor_t, comparator_t, listener_t> Make(
    list_t &list, const accessor_t &accessor,
    const comparator_t &cmp, const listener_t &listener)
  {
      return MoveSorter_<list_t, accessor_t, comparator_t, listener_t>(
        list, accessor, cmp, listener);
  }
};
//.............................................................................
EndEsdlNamespace()
#endif
