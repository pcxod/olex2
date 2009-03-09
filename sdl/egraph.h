#ifndef egraphH
#define egraphH
#include "typelist.h"
#include "etraverse.h"
//---------------------------------------------------------------------------

template <class IC, class AssociatedOC> class TEGraphNode  {
  IC Data;
  TTypeList<TEGraphNode<IC, AssociatedOC> > Nodes;
  bool RingNode, Passed;
  AssociatedOC Object;
protected:
  inline bool IsPassed()  const  {  return Passed;  }
  inline void SetPassed(bool v)  {  Passed = v;  }
  void SwapItems(int i, int j )  {  Nodes.Swap(i,j);  }
public:
  TEGraphNode( const IC& data, const AssociatedOC& object )  {
    Data = data;
    Passed = RingNode = false;
    Object = object;
  }

  inline bool IsRingNode()  const  {  return RingNode;  }
  inline void SetRIngNode()  {  RingNode = true;  }
  inline TEGraphNode& NewNode(const IC& Data, const AssociatedOC& obj )  {
    return Nodes.AddNew(Data, obj);
  }

  inline const IC& GetData()  const {  return Data;  }
  inline const AssociatedOC& GetObject()  const {  return Object;  }

  inline int Count()  const  {  return Nodes.Count();  }
  // this is for the traverser
  inline TEGraphNode& Item(int i)  const   {  return  Nodes[i];  }
  inline TEGraphNode& operator [](int i)  const {  return  Nodes[i];  }

  bool DoMatch( TEGraphNode& node )  const {
    if( node.GetData() != GetData() )  return false;
    //if( IsRingNode() )  return true;
    if( node.Count() != Count() )  return false;
    for( int i=0; i < node.Count(); i++ )
      node[i].SetPassed( false );
    for( int i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( int j=0; j < node.Count(); j++ )  {  // Count equals for both nodes
        if( node[j].IsPassed() )  continue;
        if( node[j].DoMatch( Nodes[i] ) )  {
          node[j].SetPassed( true );
          if( i != j )  node.SwapItems(i, j);  // sorting the nodes to match
          Matched = true;
          break;
        }
      }
      if( !Matched )  return false;
    }
    return true;
  }

  bool IsSubgraphOf( TEGraphNode& node ) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() < Count() )  return false;
    for( int i=0; i < node.Count(); i++ )
      node[i].SetPassed( false );
    for( int i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( int j=0; j < node.Count(); j++ )  {  // Count may not equal for nodes
        if( Nodes[i].IsSubgraphOf( node[j] ) )  {
          node[j].SetPassed( true );
          if( i != j )  node.SwapItems(i, j);  // sorting the nodes to match
          Matched = true;
          break;
        }
      }
      if( !Matched )
        return false;
    }
    return true;
  }
  static TGraphTraverser< TEGraphNode<IC, AssociatedOC> > Traverser;
};

#ifndef __BORLANDC__
template<typename IC, typename AssociatedOC>
TGraphTraverser< TEGraphNode<IC,AssociatedOC> > TEGraphNode<IC,AssociatedOC>::Traverser;
#endif

template <class IC, class AssociatedOC>  class TEGraph  {
  TEGraphNode<IC, AssociatedOC> Root;
public:
  TEGraph( const IC& Data, const AssociatedOC& obj) : Root(Data, obj)  {
  }
  TEGraphNode<IC, AssociatedOC>& GetRoot()  {  return Root;  }

  inline int Count()  const  {  return 1;  }
  inline const TEGraphNode<IC, AssociatedOC>& Item(int i)  const {  return Root;  }

  static void CompileTest();
};
#endif
