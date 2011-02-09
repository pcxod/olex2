namespace test {

void _PrepareList(TTypeList<int>& il)  {
  for( int i=0; i < 10; i++ )
    il.AddCCopy(i);
}
void _PrepareList(TArrayList<int>& il)  {
  for( int i=0; i < 10; i++ )
    il.Add(i);
}
template <class List>
void ListTests(OlxTests& t)  {
  int valid_move []   = {1,2,3,4,5,6,7,8,9,0};
  int valid_shift1 [] = {8,9,0,1,2,3,4,5,6,7};
  int valid_shift2 [] = {7,8,9,0,1,2,3,4,5,6};
  int valid_shift3 [] = {0,1,2,3,4,5,6,7,8,9};
  int valid_shift4 [] = {1,2,3,4,5,6,7,8,9,0};
  t.description = __FUNC__;
  t.description << ' ' << EsdlClassName(List);
  List il;
  _PrepareList(il);
  il.Move(0,9);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_move[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "Move failed");
  il.ShiftR(3);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift1[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftR failed");
  il.ShiftR(1);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift2[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftR failed");
  il.ShiftL(3);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift3[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftL failed");
  il.ShiftL(1);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift4[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftL failed");
  il.Move(9,0);
  for( int i=0; i < 10; i++ )
    if( il[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Consistency failed");
}
//.........................................................................
void DirectionalListTest(OlxTests& t)  {
  TUDTypeList<int> test;
  t.description = __FUNC__;
  for( int i=0; i < 10; i++ )
    test.Add(i);
  for( int i=0; i < 10; i++ )  {
    if( test[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Indexing is broken");
  }
  if( (test[5] = 7) != 7 )
    throw TFunctionFailedException(__OlxSourceInfo, "Assignment is broken");

  TUDTypeList<int*, NewCleanup<int> > test1;
  for( int i=0; i < 10; i++ )
    test1.Add(new int(i));
  for( int i=0; i < 10; i++ )  {
    if( *test1[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Indexing is broken");
  }
  if( (*test1[5] = 7) != 7 )
    throw TFunctionFailedException(__OlxSourceInfo, "Assignment is broken");
}
//.........................................................................
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