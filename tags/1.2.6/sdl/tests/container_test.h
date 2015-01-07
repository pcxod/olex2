/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "../testsuit.h"
#include "../ellist.h"
#include "../equeue.h"
#include "../estack.h"
#include "../typelist.h"
#include "../talist.h"
#include "../bitarray.h"
#include "../eset.h"

namespace test {

void _PrepareList(TTypeList<int>& il)  {
  for( int i=0; i < 10; i++ )
    il.AddCopy(i);
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
  TUDTypeList<int>::Iterator itr = test.GetIterator();
  int cnt = 0;
  while( itr.HasNext() )  {
    if( itr.Next() != cnt++ )
      throw TFunctionFailedException(__OlxSourceInfo, "Iteration failed");
  }
  if( cnt != 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

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
void LinkedlListTest(OlxTests& t)  {
  t.description = __FUNC__;
  typedef TLinkedList<int> list_t;
  list_t test;
  for( int i=0; i < 10; i++ )
    test.Add(i);
  list_t::Iterator itr = test.GetIterator();
  int cnt = 0;
  while( itr.HasNext() )  {
    if( itr.Next() != cnt++ )
      throw TFunctionFailedException(__OlxSourceInfo, "Iteration failed");
  }
  if( cnt != 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
}
//.........................................................................
void StackTest(OlxTests& t)  {
  t.description = __FUNC__;
  TStack<int> stack;
  for( int i=0; i < 10; i++ )
    stack.Push(i);
  for( int i=0; i < 10; i++ )  {
    if( stack.Top() != 10-i-1 || stack.Pop() != 10-i-1 )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if( !stack.IsEmpty() || stack.Count() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
}
//.........................................................................
void QueueTest(OlxTests& t)  {
  t.description = __FUNC__;
  TQueue<int> queue;
  for( int i=0; i < 10; i++ )
    queue.Push(i);
  for( int i=0; i < 10; i++ )  {
    if( queue.First() != i || queue.PopFirst() != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if( !queue.IsEmpty() || queue.Count() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  // now as stack
  for( int i=0; i < 10; i++ )
    queue.PushFirst(i);
  for( int i=0; i < 10; i++ )  {
    if( queue.First() != 10-i-1 || queue.Pop() != 10-i-1 )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if( !queue.IsEmpty() || queue.Count() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
}
//.........................................................................
void SharedListTest(OlxTests& t)  {
  t.description = __FUNC__;
  SharedArrayList<int> aalist(new TArrayList<int>(10)), calist, balist(calist);
  SharedTypeList<int> atlist, btlist(atlist), ctlist(atlist);
  SharedPtrList<const int> aplist, bplist;
  for( size_t i=0; i < aalist.Count(); i++ )  {
    aalist[i] = int(i);
    atlist.Add(new int) = int(i);
    aplist.Add(&(const int&)aalist[i]);
  }
  calist = aalist;
  for( size_t i=0; i < aalist.Count(); i++ )  {
    if( aalist[i] != calist[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    if( atlist[i] != aalist[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    if( atlist[i] != *aplist[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }

  if( !btlist.IsEmpty() || !balist.IsEmpty() )  // original is empty
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  aalist[0] = 10;
  if( aalist[0] != 10 || calist[0] != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  balist = calist;
  balist[1] = 9;
  if( aalist[1] != 1 || balist[1] != 9 || calist[1] != 1 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  if( *aplist[0] != 0 || *aplist[1] != 1 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  // finally will modify the original
  calist[0] = 10;
  calist[1] = 9;

  if( *aplist[0] != 10 || *aplist[1] != 9 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  TArrayList<int> al(aalist), // release reference through copy constructor
    al1;
  al1 = balist; // release reference through assignmnet
  if( aalist.IsValid() || balist.IsValid() )
    throw TFunctionFailedException(__OlxSourceInfo, "Release failed");
  if( al.Count() != 10 || al1.Count() != 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  TTypeList<int> tl(atlist), tl1;
  tl1 = btlist;
  if( atlist.IsValid() || btlist.IsValid() )
    throw TFunctionFailedException(__OlxSourceInfo, "Release failed");
  if( tl.Count() != 10 || tl1.Count() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");

  TPtrList<const int> pl(aplist), pl1;
  pl1 = bplist;
  if( aplist.IsValid() || bplist.IsValid() )
    throw TFunctionFailedException(__OlxSourceInfo, "Release failed");
  if( pl.Count() != 10 || pl1.Count() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
}
//.........................................................................
struct ConstListTest {
  ConstPtrList<int> f() {
    TPtrList<int> rv(10);
    for( int i=0; i < 10; i++ )  rv[i] = new int(i);
    return rv;
  }
  ConstArrayList<int> f1() {
    TArrayList<int> rv(10);
    for( int i=0; i < 10; i++ )  rv[i] = i;
    return rv;
  }
  ConstTypeList<int> f2() {
    TTypeList<int> rv;
    for( int i=0; i < 10; i++ )  rv.AddNew(i);
    return rv;
  }
  void ContainerTests(OlxTests& t) {
    t.description = __OlxSrcInfo;
    TPtrList<int> pl = f();
    if( pl.Count() != 10 )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    for( int i=0; i < pl.Count(); i++ )  {
      if( *pl[i] != i )
        throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    }
    pl.DeleteItems(false);
    TArrayList<int> al = f1();
    if( al.Count() != 10 )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    for( int i=0; i < 10; i++ )  {
      if( al[i] != i )
        throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    }
    TTypeList<int> tl = f2();
    if( tl.Count() != 10 )
      throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    for( int i=0; tl.Count() < 10; i++ )  {
      if( tl[i] != i )
        throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
    }
  }
};
//.........................................................................
void SetTest(OlxTests& t) {
  t.description = __FUNC__;
  int sv[5] = { 1, 2, 5, 7, 1 }, sv1[4] = {2,7,8,9};
  olxset<int, TPrimitiveComparator> s(sv, 5), s1(sv1, 4);
  if (s.Count() != 4) {
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if ((s-s1).Count() != 2) {
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if ((s + s1).Count() != 6) {
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
  if ((s & s1).Count() != 2) {
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected result");
  }
}
//.........................................................................
void BitArrayTest(OlxTests& t)  {
  t.description = __FUNC__;
  TEBitArray br(16);
  for (size_t i=0; i < br.Count(); i++) {
    if ((i%2)==0)
      br.SetTrue(i);
  }
  const char*res[17] = {
    "101010101010101",
    "11111111",
    "0502050205",
    "05050505",
    "211021",
    "021021005",
    "085042001",
    "085085",
    "03410042",
    "03410021",
    "13650010",
    "0136500005",
    "0546100002",
    "0546100001",
    "021845",
    "021845",
    "021845"
  };
  for (int i=0; i < 17; i++) {
    if (br.FormatString(i+1) != res[i]) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr(br.FormatString(i+1)) << " != " << res[i]);
    }
  }
}
//.........................................................................
void ContainerTests(OlxTests& t)  {
  t.Add(&test::ListTests<TArrayList<int> >)
    .Add(&test::ListTests<TTypeList<int> >)
    .Add(&test::DirectionalListTest)
    .Add(&test::LinkedlListTest).
    Add(&test::QueueTest).
    Add(&test::StackTest).
    Add(&test::SharedListTest).
    Add(new ConstListTest, &ConstListTest::ContainerTests).
    Add(&test::BitArrayTest).
    Add(&test::SetTest);
}
//.........................................................................
};  // namespace test
