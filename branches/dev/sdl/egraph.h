#ifndef egraphH
#define egraphH
#include "typelist.h"
#include "etraverse.h"
#include "emath.h"
//---------------------------------------------------------------------------

template <class IC, class AssociatedOC> class TEGraphNode : ACollectionItem  {
  IC Data;
  typedef TEGraphNode<IC, AssociatedOC> NodeType;
  typedef AnAssociation2<TIntList,TIntList> ConnInfo;
  TPtrList<NodeType> Nodes;
  bool RingNode, Root;
  mutable bool Passed, Mutable;
  AssociatedOC Object;
protected:
  inline bool IsPassed()  const  {  return Passed;  }
  inline void SetPassed(bool v)  {  Passed = v;  }
  static int _SortNodesByTag(const NodeType* n1, const NodeType* n2) {
    return n1->GetTag() - n2->GetTag();
  }
public:
  TEGraphNode( const IC& data, const AssociatedOC& object )  {
    Data = data;
    Mutable = Root = Passed = RingNode = false;
    Object = object;
  }
  ~TEGraphNode()  {
    for( int i=0; i < Nodes.Count(); i++ )
      delete Nodes[i];
  }

  inline bool IsRingNode()  const  {  return RingNode;  }
  inline void SetRIngNode()  {  RingNode = true;  }
  inline TEGraphNode& NewNode(const IC& Data, const AssociatedOC& obj )  {
    return *Nodes.Add( new TEGraphNode(Data, obj) );
  }
  void SortNodesByTag() {
    Nodes.BubleSorter.SortSF(Nodes, &TEGraphNode::_SortNodesByTag);
  }
  inline const IC& GetData()  const {  return Data;  }
  inline const AssociatedOC& GetObject()  const {  return Object;  }

  inline int Count()  const  {  return Nodes.Count();  }
  // this is for the traverser
  inline TEGraphNode& Item(int i)  const   {  return  *Nodes[i];  }
  inline TEGraphNode& operator [](int i)  const {  return  *Nodes[i];  }
  void SwapItems(int i, int j )  {  
    if( i != j )
      Nodes.Swap(i,j);  
  }

  bool DoMatch( TEGraphNode& node )  const {
    if( node.GetData() != GetData() )  return false;
    //if( IsRingNode() )  return true;
    if( node.Count() != Count() )  return false;
    for( int i=0; i < node.Count(); i++ )
      node[i].SetPassed( false );
    for( int i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( node[j].IsPassed() )  continue;
        if( Nodes[i]->DoMatch(node[j]) )  {
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

  template <class Analyser> bool FullMatch(TEGraphNode& node, Analyser& analyser) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    for( int i=0; i < Count(); i++ )  {
      int mc=0;
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        // we cannot do node swapping here, since it will invalidate the matching indexes
        if( Nodes[i]->FullMatch( node[j], analyser ) )  {
           analyser.OnMatch(*this, node, i, j);
           mc++;
        }
      }
      if( mc == 0 )  return false;
    }
    return true;
  }
  bool AnalyseMutability(TEGraphNode& node) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    int maxMatches = 0;
    Mutable = false;
    for( int i=0; i < Count(); i++ )  {
      int mc=0;
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( Nodes[i]->AnalyseMutability( node[j] ) )
           mc++;
      }
      if( mc == 0 )  return false;
      if( mc > maxMatches )
        maxMatches = mc;
    }
    if( maxMatches > 1 )
      Mutable = true;
    return true;
  }
  template <class Analyser> bool FullMatchEx(TEGraphNode& node, Analyser& analyser, bool analyse = false) const {
    if( IsRoot() )
      this->AnalyseMutability(node);
    const int node_cnt = Count();
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != node_cnt )  return false;
    if( node_cnt == 0 )  return true;
    if( (!analyse && !IsRoot()) || !Mutable )  {
      for( int i=0; i < node_cnt; i++ )
        node[i].SetPassed( false );
      for( int i=0; i < node_cnt; i++ )  {
        bool Matched = false;
        for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
          if( node[j].IsPassed() )  continue;
          if( Nodes[i]->FullMatchEx(node[j], analyser, analyse) )  {
            node[j].SetPassed( true );
            if( i != j )  
              node.SwapItems(i, j);  // sorting the nodes to match
            Matched = true;
            break;
          }
        }
        if( !Matched )  return false;
      }
      return true;
    }
    TTypeList< ConnInfo > conn;
    TIntList permutation;
    for( int i=0; i < node_cnt; i++ )  {
      for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
        if( Nodes[i]->FullMatchEx( node[j], analyser, false ) )  {
          bool found = false;
          for( int k=0; k < conn.Count(); k++ )  {
            if( conn[k].GetA().IndexOf(i) != -1 )  {
              if( conn[k].GetB().IndexOf(j) == -1 )
                conn[k].B().Add(j);
              found = true;
              break;
            }
            else if( conn[k].GetB().IndexOf(j) != -1 )  {
              if( conn[k].GetA().IndexOf(i) == -1 )
                conn[k].A().Add(i);
              found = true;
              break;
            }
          }
          if( !found )  {
            ConnInfo& ci = conn.AddNew();
            ci.A().Add(i);
            ci.B().Add(j);
          }
        }
      }
    }
    if( conn.IsEmpty() )
      return false;
    for( int i=0; i < conn.Count(); i++ )  {  // run one to one matches first
      const ConnInfo& ci = conn[i];
      for( int j=0; j < ci.GetA().Count(); j++ )
        node[ci.GetB()[j]].SetTag(ci.GetA()[j]);
      if( ci.GetA().Count() == 1 )
        conn.NullItem(i);
    }
    node.SortNodesByTag();
    conn.Pack();
    if( conn.IsEmpty() )  {
      for( int i=0; i < node_cnt; i++ )
        Nodes[i]->FullMatchEx(node[i], analyser, analyse);
      return true;
    }
    TPtrList<TEGraphNode> ond(node.Nodes);
    for( int i=0; i < conn.Count(); i++ )  {
      const ConnInfo& ci = conn[i];
      const int perm_cnt = (int)Factorial(ci.GetA().Count());
      int best_perm = -1;
      double minRms = 0;
      for( int j=0; j < perm_cnt; j++ )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, j);
        for( int k=0; k < permutation.Count(); k++ )
          node.Nodes[ci.GetA()[k]] = ond[permutation[k]];

        for( int k=0; k < node_cnt; k++ )
          Nodes[k]->FullMatchEx(node[k], analyser, true);
        if( j == 0 )  {
          minRms = analyser.CalcRMS();
          best_perm = j;
        }
        else  {
          const double rms = analyser.CalcRMS();
          if( rms < minRms )  {
            minRms = rms;
            best_perm = j;
          }
        }
      }
      if( best_perm != perm_cnt-1 )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, best_perm);
        for( int k=0; k < permutation.Count(); k++ )
          node.Nodes[ci.GetA()[k]] = ond[permutation[k]];
        for( int k=0; k < node_cnt; k++ )
          Nodes[k]->FullMatchEx(node[k], analyser, analyse);
      }
    }
    return true;
  }
  //template <class Analyser> bool FullMatchEx(TEGraphNode& node, Analyser& analyser) const {
  //  if( node.GetData() != GetData() )  return false;
  //  if( node.Count() != Count() )  return false;
  //  for( int i=0; i < Count(); i++ )
  //    node[i].SetPassed(false);
  //  for( int i=0; i < Count(); i++ )  {
  //    int mc=0, bestIndex = -1;
  //    double minRMS = 0;
  //    for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
  //      if( node[j].IsPassed() )  continue;
  //      if( Nodes[i].FullMatchEx( node[j], analyser ) )  {
  //        if( mc == 0 )
  //          bestIndex = j;
  //        else  {
  //          if( mc == 1 )  {  // calculate the RMS only if more than 1 matches
  //            node.SwapItems(i, bestIndex);
  //            minRMS = analyser.CalcRMS();
  //            node.SwapItems(i, bestIndex);
  //          }
  //          node.SwapItems(i, j);
  //          const double RMS = analyser.CalcRMS();
  //          if( RMS < minRMS )  {
  //            bestIndex = j;
  //            minRMS = RMS;
  //          }
  //          node.SwapItems(i, j);  // restore the node order
  //        }
  //        mc++;
  //      }
  //    }
  //    if( mc == 0 )  return false;
  //    node[bestIndex].SetPassed(true);
  //    node.SwapItems(i, bestIndex);
  //  }
  //  return true;
  //}

  bool IsSubgraphOf( TEGraphNode& node ) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() < Count() )  return false;
    for( int i=0; i < node.Count(); i++ )
      node[i].SetPassed( false );
    for( int i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( int j=0; j < node.Count(); j++ )  {  // Count may not equal for nodes
        if( Nodes[i]->IsSubgraphOf( node[j] ) )  {
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
  DefPropB(Root)
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
    Root.SetRoot(true);
  }
  TEGraphNode<IC, AssociatedOC>& GetRoot()  {  return Root;  }

  inline int Count()  const  {  return 1;  }
  inline const TEGraphNode<IC, AssociatedOC>& Item(int i)  const {  return Root;  }

  static void CompileTest();
};
#endif
