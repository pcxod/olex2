/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

namespace test {

class SortTest  {
  static int icmp(const int& a, const int& b)  {  return a-b;  }
  struct Cmp {
    int Compare(const int &a, const int &b) const {
      return a-b;
    }
  };
public:
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
    IEObject *cause=NULL;
    try {
      Cmp cmp;
      Sorter::Sort(al1, FunctionComparator::Make(icmp),
        SyncSortListener::Make(al2));
      Sorter::Sort(pl1, Cmp(),
        SyncSortListener::Make(pl2));
      Sorter::Sort(tl1, FunctionComparator::MakeConst(cmp, &Cmp::Compare),
        SyncSortListener::Make(tl2));
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
      Sorter::Sort(al2, ReverseComparator::MakeConst(cmp, &Cmp::Compare));
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
  void DoTest(OlxTests& t) {
    t.Add( &SortTest::DoTestT<BubbleSorter>)
      .Add(&SortTest::DoTestT<QuickSorter>)
      .Add(&SortTest::DoTestT<MoveSorter>);
  }
};
//.........................................................................
};  // namespace test
