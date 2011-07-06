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
    if( stack.Current() != 10-i-1 || stack.Pop() != 10-i-1 )
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
void ContainerTests(OlxTests& t)  {
  t.Add(&test::ListTests<TArrayList<int> >)
    .Add(&test::ListTests<TTypeList<int> >)
    .Add(&test::DirectionalListTest)
    .Add(&test::LinkedlListTest).
    Add(&test::QueueTest).
    Add(&test::StackTest);
}
//.........................................................................
};  // namespace test
