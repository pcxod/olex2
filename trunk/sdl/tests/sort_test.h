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
  static int iptrcmp(const int* a, const int* b)  {  return *a-*b;  }
public:
  void DoTest(OlxTests& t)  {
    t.description << __FUNC__;
    TArrayList<int> al1(100), al2(100);
    TPtrList<int> pl1(100), pl2(100);
    TTypeList<int> tl1((size_t)100), tl2((size_t)100);
    for( size_t i=0; i < 100; i++ )  {
      al1[i] = al2[i] = rand();
      pl1[i] = &al1[i];  pl2[i] = &al2[i];
      tl1.Set(i, &al1[i]);  tl2.Set(i, &al2[i]);
    }
    al1.QuickSorter.Sort(al1, Sort_StaticFunctionWrapper<const int&>(icmp), SyncSwapListener<TArrayList<int> >(al2));
    pl1.QuickSorter.Sort(pl1, Sort_StaticFunctionWrapper<const int*>(iptrcmp), SyncSwapListener<TPtrList<int> >(pl2));
    tl1.QuickSorter.Sort(tl1, Sort_StaticFunctionWrapper<const int*>(iptrcmp), SyncSwapListener<TTypeList<int> >(tl2));
    for( size_t i=0; i < 100; i++ )  {
      if( i > 0 && (al1[i-1] > al1[i] || tl1[i-1] > tl2[i] || *pl1[i-1] > *pl2[i]) )
        throw TFunctionFailedException(__OlxSourceInfo, "sorting failed");
      if( al1[i] != al2[i] || tl1[i] != tl2[i] || *pl1[i] != *pl2[i] )
        throw TFunctionFailedException(__OlxSourceInfo, "syncronised sorting failed");
    }
    tl1.ReleaseAll();
    tl2.ReleaseAll();
  }
};
//.........................................................................
};  // namespace test
