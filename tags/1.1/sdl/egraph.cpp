#ifdef __BORLANDC__
  #pragma hdrstop
#endif

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
  g.GetRoot().Traverser.Traverse(g.GetRoot(), tr );
}
