/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
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

class TPrimitiveComparator {
public:
  TPrimitiveComparator() {}
  template <class ComparableA, class ComparableB>
  inline int Compare(const ComparableA& A, const ComparableB& B) const {
    if (olx_ref::get(A) < olx_ref::get(B))  return -1;
    if (olx_ref::get(A) > olx_ref::get(B))  return 1;
    return 0;
  }
};
//.............................................................................
class TPointerComparator {
public:
  TPointerComparator() {}
  template <class ComparableA, class ComparableB>
  inline int Compare(const ComparableA& A, const ComparableB& B) const {
    if (olx_ptr::get(A) < olx_ptr::get(B))  return -1;
    if (olx_ptr::get(A) > olx_ptr::get(B))  return 1;
    return 0;
  }
};
//.............................................................................
class TComparableComparator {
public:
  TComparableComparator() {}
  template <class ComparableA, class ComparableB>
  inline int Compare(const ComparableA& A, const ComparableB& B) const {
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
      :  Instance(instance), Func(func)
    {}
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &a, const item_b_t &b) const {
      return (Instance.*Func)(olx_ref::get(a), olx_ref::get(b));
    }
  };
  template <class Base, typename ItemClass> struct ComparatorCMF_ {
    const Base& Instance;
    int (Base::*Func)(const ItemClass &a, const ItemClass &b) const;
    ComparatorCMF_(const Base& instance,
      int (Base::*func)(const ItemClass &a, const ItemClass &b) const)
      :  Instance(instance), Func(func)
    {}
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &a, const item_b_t &b) const {
      return (Instance.*Func)(olx_ref::get(a), olx_ref::get(b));
    }
  };
  template <typename ItemClass> struct ComparatorSF_ {
    int (*Func)(const ItemClass &a, const ItemClass &b);
    ComparatorSF_(int (*func)(const ItemClass &a, const ItemClass &b))
      : Func(func)  {}
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &a, const item_b_t &b) const {
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
struct ComplexComparator {
  template <class Accessor, class Comparator>
  class TComplexComparator {
    Accessor acc;
    Comparator cmp;
  public:
    TComplexComparator(const Accessor& acc)
      : acc(acc)
    {}
    TComplexComparator(const Accessor &acc, const Comparator &cmp)
      : acc(acc), cmp(cmp)
    {}
    template <class ComparableA, class ComparableB>
    inline int Compare(const ComparableA& A, const ComparableB& B) const {
      return cmp.Compare(acc(A), acc(B));
    }
  };

  template <class Accessor, class Comparator>
  static TComplexComparator<Accessor, Comparator>
    Make(const Accessor &acc, const Comparator &cmp) {
      return TComplexComparator<Accessor, Comparator>(acc, cmp);
  }
};
//.............................................................................
struct ReverseComparator {
  template <class cmp_t> struct ReverseComparator_ {
    const cmp_t &cmp;
    ReverseComparator_(const cmp_t &cmp_) : cmp(cmp_) {}
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &i1, const item_b_t &i2) const {
      return cmp.Compare(i2, i1);
    }
  };
  template <typename item_t> struct ReverseSF {
    int (*func)(const item_t &, const item_t&);
    ReverseSF(int (*func_)(const item_t &, const item_t&)) : func(func_) {}
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &i1, const item_b_t &i2) const {
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
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &i1, const item_b_t &i2) const {
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
    template <typename item_a_t, typename item_b_t>
    int Compare(const item_a_t &i1, const item_b_t &i2) const {
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
// size_priority - longer list is larger disregarding elements
template <bool size_priority = false>
struct ListComparator {
  template <class Comparator>
  struct TListComparator {
    Comparator cmp;
    TListComparator() {}
    TListComparator(const Comparator& cmp)
      : cmp(cmp)
    {}

    template <class list_a_t, class list_b_t>
    int Compare(const list_a_t& a, const list_b_t& b) const {
      if (size_priority) {
        int r = olx_cmp(a.Count(), b.Count());
        if (r != 0) {
          return r;
        }
      }
      else {
        size_t sz = olx_min(a.Count(), b.Count());
        for (size_t i = 0; i < sz; i++) {
          int r = cmp.Compare(a[i], b[i]);
          if (r != 0) {
            return r;
          }
        }
        return olx_cmp(a.Count(), b.Count());
      }
    }
  };
  template <class Comparator>
  TListComparator<Comparator> make(const Comparator& cmp) {
    return TListComparator<Comparator>(cmp);
  }

  template <class Accessor, class Comparator>
  typename ComplexComparator::TComplexComparator<Accessor, Comparator>
    make(const Accessor &acc, const Comparator& cmp)
  {
    typedef ComplexComparator::TComplexComparator<Accessor, Comparator> cmp_t;
    return TListComparator<cmp_t>(cmp_t(acc, cmp));
  }
};
//.............................................................................
struct DummySortListener {
  static void OnSwap(size_t, size_t)  {}
  static void OnMove(size_t, size_t)  {}
};
//.............................................................................
struct SyncSortListener {
  template <class list_t, class acc_t>
  struct SyncSortListener_1 {
    list_t& list;
    acc_t acc;
    SyncSortListener_1(list_t& _list, const acc_t& a)
      : list(_list), acc(a)
    {}
    void OnSwap(size_t i, size_t j) const { acc(list).Swap(i, j); }
    void OnMove(size_t i, size_t j) const { acc(list).Move(i, j); }
  };
  template <class list_t, class acc_t>
  struct SyncSortListener_N {
    list_t& list;
    acc_t acc;
    SyncSortListener_N(list_t& _list, const acc_t &acc)
      : list(_list), acc(acc)
    {}
    void OnSwap(size_t i, size_t j) const {
      for (size_t li = 0; li < list.Count(); li++) {
        olx_ref::get(acc(list[li])).Swap(i, j);
      }
    }
    void OnMove(size_t i, size_t j) const {
      for (size_t li = 0; li < list.Count(); li++) {
        olx_ref::get(acc(list[li])).Move(i, j);
      }
    }
  };
  /* synchronises a single list */
  template <class list_t>
  static SyncSortListener_1<list_t, DummyAccessor> MakeSingle(
    list_t &l)
  {
    return SyncSortListener_1<list_t, DummyAccessor>(l, DummyAccessor());
  }
  template <class list_t, class acc_t>
  static SyncSortListener_1<list_t, acc_t> MakeSingle(
    list_t &l, const acc_t &acc)
  {
    return SyncSortListener_1<list_t, acc_t>(l, acc);
  }
  /* syncronises a list of lists */
  template <class list_t>
  static SyncSortListener_N<list_t, DummyAccessor> MakeMultiple(
    list_t &l)
  {
    return SyncSortListener_N<list_t, DummyAccessor>(l, DummyAccessor());
  }
  template <class list_t, class acc_t>
  static SyncSortListener_N<list_t, acc_t> MakeMultiple(
    list_t &l, const acc_t &acc)
  {
    return SyncSortListener_N<list_t, acc_t>(l, acc);
  }
};
//.............................................................................
template <class Sorter>
struct SortInterface {
  template <class list_t, class comparator_t, class listener_t>
  static void Sort(list_t &list, const comparator_t& cmp,
    const listener_t& l)
  {
    Sorter::Make(list, cmp, l).Sort();
  }
  template <class list_t, class comparator_t>
  static void Sort(list_t &list, const comparator_t &cmp)  {
    Sorter::Make(list, cmp, DummySortListener()).Sort();
  }
  template <class list_t>
  static void Sort(list_t &list) {
    Sorter::Make(list, TComparableComparator(), DummySortListener()).Sort();
  }

  template <class list_t, class item_t>
  static void SortSF(list_t &list, int (*f)(const item_t&, const item_t&))
  {
    Sorter::Make(list, FunctionComparator::Make(f), DummySortListener()).Sort();
  }
  template <class list_t, class base_t, class item_t>
  static void SortMF(list_t &list,
    const base_t &base,
    int (base_t::*f)(const item_t&, const item_t&) const)
  {
    Sorter::Make(list, FunctionComparator::MakeConst(base, f),
      DummySortListener()).Sort();
  }
};
struct QuickSorter : public SortInterface<QuickSorter> {
  template <class list_t, class comparator_t, class listener_t>
  struct QuickSorter_ {
    QuickSorter_(list_t& list_, const comparator_t& comparator_,
    const listener_t& listener_)
      : list(list_), listener(listener_), cmp(comparator_)
    {}
    void Sort() {
      size_t cnt = list.list.Count();
      if (cnt < 2)  return;
      DoSort(0, cnt - 1);
    }
  protected:
    typename list_t::InternalAccessor list;
    const listener_t& listener;
    const comparator_t& cmp;
    typedef typename list_t::InternalAccessor::list_item_type item_t;
    void DoSort(size_t lo0_, size_t hi0_) {
      typedef AnAssociation3<size_t, size_t, bool> d_t;
      TQueue<d_t> stack;
      stack.Push(d_t(lo0_, hi0_, true));
      while (!stack.IsEmpty()) {
        const d_t tv = stack.Pop();
        const size_t diff = tv.GetB() - tv.GetA();
        if (diff == 1) {
          if (cmp.Compare(list[tv.GetA()], list[tv.GetB()]) > 0) {
            listener.OnSwap(tv.GetA(), tv.GetB());
            list.list.Swap(tv.GetA(), tv.GetB());
          }
        }
        else if (diff > 0) {
          size_t pi = tv.GetA() + (diff >> 1);
          if (tv.GetC()) { // try to fix pivot
            size_t inc = (diff >> 2), li = pi - inc, ri = pi + inc;
            int c1 = cmp.Compare(list[pi], list[li]);
            int c2 = cmp.Compare(list[pi], list[ri]);
            if ((c1 > 0 && c2 > 0) || (c1 < 0 && c2 < 0)) {
              int c3 = cmp.Compare(list[li], list[ri]);
              if (c1 < 0) { // li/ri < mi
                pi = (c3 < 0 ? ri : li);
              }
              else { // mi < li/ri
                pi = (c3 < 0 ? li : ri);
              }
            }
          }
          item_t mid = list[pi];
          listener.OnSwap(pi, tv.GetB());
          list.list.Swap(pi, tv.GetB());
          pi = tv.GetA();
          for (size_t i = tv.GetA(); i < tv.GetB(); i++) {
            if (cmp.Compare(list[i], mid) <= 0) {
              if (i != pi) {
                listener.OnSwap(i, pi);
                list.list.Swap(i, pi);
              }
              pi++;
            }
          }
          listener.OnSwap(pi, tv.GetB());
          list.list.Swap(pi, tv.GetB());
          size_t l1 = pi - tv.GetA(), l2 = tv.GetB() - pi,
            ml = olx_max(l1, l2);
          bool sp = (l1 == 0 || l2 == 0) ? true
            : (ml > 32 && (double)ml / (double)olx_min(l1, l2) >= 1.75);
          if (l2 > 0) {
            stack.Push(d_t(pi + 1, tv.GetB(), sp ? l2 == ml : false));
          }
          if (l1 > 0) {
            stack.Push(d_t(tv.GetA(), pi - 1, sp ? l1 == ml : false));
          }
        }
      }
    }
  };
  template <class list_t, class comparator_t, class listener_t>
  static QuickSorter_ <list_t,comparator_t,listener_t>
  Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return QuickSorter_<list_t,comparator_t,listener_t>(
      list, cmp, listener);
  }
};
//.............................................................................
struct BubbleSorter : public SortInterface<BubbleSorter> {
  template <class list_t, class comparator_t, class listener_t>
  struct BubbleSorter_ {
    BubbleSorter_(list_t &list_, const comparator_t &cmp_,
      const listener_t &listener_)
      : list(list_), cmp(cmp_), listener(listener_)
    {}
    void Sort()  {
      size_t lc = list.list.Count();
      while (lc > 0) {
        size_t nlc = 0;
        for (size_t i=0; i < lc-1; i++) {
          if (cmp.Compare(list[i+1], list[i]) < 0) {
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
    const comparator_t &cmp;
    const listener_t &listener;
  };
  template <class list_t, class comparator_t, class listener_t>
  static BubbleSorter_<list_t,comparator_t,listener_t>
  Make(list_t &list, const comparator_t &cmp, const listener_t &listener) {
    return BubbleSorter_<list_t,comparator_t,listener_t>(
      list, cmp, listener);
  }
};
//.............................................................................
EndEsdlNamespace()
#endif
