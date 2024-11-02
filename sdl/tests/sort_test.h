/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "std_sort.h"
#include "stable_sort.h"

namespace test {

class SortTest  {
  static int icmp(const int& a, const int& b)  {  return a-b;  }
  struct Cmp {
    template <class it>
    int Compare(const it &a, const it &b) const {
      return olx_ref::get(a)-olx_ref::get(b);
    }
    int compare(const int &a, const int &b) const {
      return a-b;
    }
  };
public:
  template <class list_t>
  static void shuffle(list_t& list) {
    size_t lc = list.Count();
    for (size_t i = 0; i < lc*2; i++) {
      size_t p1 = (olx_abs(rand()) * lc / RAND_MAX) % lc;
      size_t p2 = (olx_abs(rand()) * lc / RAND_MAX) % lc;
      list.Swap(p1, p2);
    }
  }
  template <class Sorter>
  static void DoTestT(OlxTests& t)  {
    t.description << __FUNC__ << '<' << EsdlClassName(Sorter) << '>';
    TArrayList<int> al1(1001), al2(1001);
    TPtrList<int> pl1(1001), pl2(1001);
    TTypeList<int> tl1(1001, false), tl2(1001, false);
    for( size_t i=0; i < al1.Count(); i++ )  {
      al1[i] = al2[i] = rand();
      pl1[i] = &al1[i];  pl2[i] = &al2[i];
      tl1.Set(i, &al1[i]);  tl2.Set(i, &al2[i]);
    }
    IOlxObject *cause=NULL;
    try {
      Cmp cmp;
      Sorter::Sort(al1, FunctionComparator::Make(icmp),
        SyncSortListener::MakeSingle(al2));
      Sorter::Sort(pl1, Cmp(),
        SyncSortListener::MakeSingle(pl2));
      Sorter::Sort(tl1, FunctionComparator::MakeConst(cmp, &Cmp::compare),
        SyncSortListener::MakeSingle(tl2));
      for( size_t i=0; i < al1.Count(); i++ )  {
        if( i > 0 && (al1[i-1] > al1[i] || tl1[i-1] > tl2[i] || *pl1[i-1] > *pl2[i]) )
          throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
        if( al1[i] != al2[i] || tl1[i] != tl2[i] || *pl1[i] != *pl2[i] )
          throw TFunctionFailedException(__OlxSourceInfo, "syncronised sorting failed");
      }
      Sorter::Sort(al1, ReverseComparator::Make(icmp));
      Sorter::Sort(al2, ReverseComparator::Make(Cmp()));
      for( size_t i=0; i < al1.Count(); i++ )  {
        if (i > 0 && (al1[i-1] < al1[i] || al2[i-1] < al2[i] || al1[i] != al2[i]))
          throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
      }
      Sorter::Sort(al2, ReverseComparator::MakeConst(cmp, &Cmp::compare));
      for( size_t i=0; i < al1.Count(); i++ )  {
        if (al1[i] != al2[i])
          throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
      }
    }
    catch(TExceptionBase &e) {
      cause = e.Replicate();
    }
    tl1.ReleaseAll();
    tl2.ReleaseAll();
    if (cause != 0) {
      throw TFunctionFailedException(__OlxSourceInfo, cause);
    }
  }
  struct StableSortVal {
    int val;
    size_t idx;
    StableSortVal() {}
    StableSortVal(int val, size_t idx)
      : val(val), idx(idx)
    {}
    int Compare(const StableSortVal& o) const {
      return olx_cmp(val, o.val);
    }
  };

  template <class Sorter>
  static void DoStableTestT(OlxTests& t) {
    t.description << __FUNC__ << '<' << EsdlClassName(Sorter) << '>';
    TArrayList<StableSortVal> al1(1021), al2(1021);
    // create N elements with values 0-5
    for (size_t i = 0; i < al1.Count(); i++) {
      al1[i] = al2[i] = StableSortVal(rand()%6, i);
    }
    Sorter::Sort(al1, TComparableComparator(),
      SyncSortListener::MakeSingle(al2));
    for (size_t i = 0; i < al1.Count(); i++) {
      if (i > 0 && (al1[i - 1].val > al1[i].val)) {
        throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
      }
      if (al1[i].val != al2[i].val) {
        throw TFunctionFailedException(__OlxSourceInfo, "syncronised sorting failed");
      }
      if (i > 0 && (al1[i - 1].val == al1[i].val && al1[i - 1].idx > al1[i].idx)) {
        throw TFunctionFailedException(__OlxSourceInfo, "stable sorting failed");
      }
    }
  }



  void DoTest(OlxTests& t) {
    t.Add( &SortTest::DoTestT<BubbleSorter>)
      .Add(&SortTest::DoTestT<QuickSorter>)
      .Add(&SortTest::DoTestT<StdSorter>)
      .Add(&SortTest::DoTestT<StdStableSorter>)
      ;
    t.Add(&SortTest::DoStableTestT<BubbleSorter>)
      .Add(&SortTest::DoStableTestT<StdStableSorter>)
      .Add(&SortTest::DoStableTestT<StableQuickSorter>)
      ;
  }
};
//.........................................................................
};  // namespace test
