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
  void DoTest(OlxTests& t)  {
    t.description << __FUNC__;
    TArrayList<int> al1(100), al2(100);
    TPtrList<int> pl1(100), pl2(100);
    TTypeList<int> tl1(100, false), tl2(100, false);
    for( size_t i=0; i < 100; i++ )  {
      al1[i] = al2[i] = rand();
      pl1[i] = &al1[i];  pl2[i] = &al2[i];
      tl1.Set(i, &al1[i]);  tl2.Set(i, &al2[i]);
    }
    Cmp cmp;
    QuickSorter::Sort(al1, FunctionComparator::Make(icmp),
      SyncSwapListener::Make(al2));
    QuickSorter::Sort(pl1, Cmp(),
      SyncSwapListener::Make(pl2));
    QuickSorter::Sort(tl1, FunctionComparator::MakeConst(cmp, &Cmp::Compare),
      SyncSwapListener::Make(tl2));
    for( size_t i=0; i < 100; i++ )  {
      if( i > 0 && (al1[i-1] > al1[i] || tl1[i-1] > tl2[i] || *pl1[i-1] > *pl2[i]) )
        throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
      if( al1[i] != al2[i] || tl1[i] != tl2[i] || *pl1[i] != *pl2[i] )
        throw TFunctionFailedException(__OlxSourceInfo, "syncronised sorting failed");
    }
    QuickSorter::Sort(al1, ReverseComparator::Make(icmp));
    QuickSorter::Sort(al2, ReverseComparator::Make(Cmp()));
    for( size_t i=0; i < 100; i++ )  {
      if (i > 0 && (al1[i-1] < al1[i] || al2[i-1] < al2[i] || al1[i] != al2[i]))
        throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
    }
    QuickSorter::Sort(al2, ReverseComparator::MakeConst(cmp, &Cmp::Compare));
    for( size_t i=0; i < 100; i++ )  {
      if (al1[i] != al2[i])
        throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
    }
    tl1.ReleaseAll();
    tl2.ReleaseAll();
  }
};
//.........................................................................
};  // namespace test
