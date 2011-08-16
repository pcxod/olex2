/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "egraph.h"

class Test_TTraverserR  {
public:
  bool OnItem(const TEGraphNode<int, void*>& v)  {
    return true;
  }
};

template class TEGraph<int, void*>;

template<> 
void TEGraph<int, void*>::CompileTest()  {
  TEGraph<int, void*> g(10, NULL);
  TEGraph<int, void*> g1(10, NULL);
  g.GetRoot().DoMatch( g1.GetRoot());
  Test_TTraverserR tr;
  g.GetRoot().Traverser.Traverse(g.GetRoot(), tr);
}
