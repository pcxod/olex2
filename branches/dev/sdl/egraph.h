#ifndef egraphH
#define egraphH
#include "typelist.h"
#include "etraverse.h"
#include "emath.h"
//---------------------------------------------------------------------------

template <class IC, class AssociatedOC> class TEGraphNode : ACollectionItem  {
  IC Data;
  typedef TEGraphNode<IC, AssociatedOC> NodeType;
  TTypeList<NodeType> Nodes;
  bool RingNode, Passed;
  AssociatedOC Object;
protected:
  inline bool IsPassed()  const  {  return Passed;  }
  inline void SetPassed(bool v)  {  Passed = v;  }
  static int _SortNodesByTag(const NodeType& n1, const NodeType& n2) {
    return n1.GetTag() - n2.GetTag();
  }
public:
  TEGraphNode( const IC& data, const AssociatedOC& object )  {
    Data = data;
    Passed = RingNode = false;
    Object = object;
  }

  inline bool IsRingNode()  const  {  return RingNode;  }
  inline void SetRIngNode()  {  RingNode = true;  }
  inline TEGraphNode& NewNode(const IC& Data, const AssociatedOC& obj )  {
    TEGraphNode& nd = Nodes.AddNew(Data, obj);
    nd.SetTag(Nodes.Count()-1);
    return nd;
  }
  void SortNodesByTag() {
    Nodes.BubleSorter.SortSF(Nodes, &TEGraphNode::_SortNodesByTag);
  }
  inline const IC& GetData()  const {  return Data;  }
  inline const AssociatedOC& GetObject()  const {  return Object;  }

  inline int Count()  const  {  return Nodes.Count();  }
  // this is for the traverser
  inline TEGraphNode& Item(int i)  const   {  return  Nodes[i];  }
  inline TEGraphNode& operator [](int i)  const {  return  Nodes[i];  }
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

  template <class Analyser> bool FullMatch(TEGraphNode& node, Analyser& analyser) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    for( int i=0; i < Count(); i++ )  {
      int mc=0;
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        // we cannot do node swapping here, since it will invalidate the matching indexes
        if( Nodes[i].FullMatch( node[j], analyser ) )  {
           analyser.OnMatch(*this, node, i, j);
           mc++;
        }
      }
      if( mc == 0 )  return false;
    }
    return true;
  }
  template <class Analyser> bool FullMatchEx(TEGraphNode& node, Analyser& analyser, bool analyse = true) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    if( Count() == 0 )  return true;
    if( !analyse )
      return DoMatch(node);
    typedef AnAssociation2<TIntList,TIntList> ConnInfo;
    TTypeList< ConnInfo > conn;
    TIntList permutation, permuted;
    for( int i=0; i < Count(); i++ )  {
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( Nodes[i].FullMatchEx( node[j], analyser, false ) )  {
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
    for( int i=0; i < conn.Count(); i++ )  {
      ConnInfo& ci = conn[i];
      if( ci.GetA().Count() != ci.GetB().Count() )
        throw 1;
      if( ci.GetA().Count() == 1 )  // one to one match
        node.SwapItems( ci.GetA()[0], ci.GetB()[0] );
      else  {
        const int perm_cnt = (int)Factorial(ci.GetA().Count());
        int best_perm = -1;
        double minRms = 0;
        for( int j=0; j < perm_cnt; j++ )  {
          permutation = ci.GetB();
          GeneratePermutation(permutation, j);
          permuted.Clear();
          for( int k=0; k < permutation.Count(); k++ )  {
            if( permuted.IndexOf(ci.GetA()[k]) == -1 )  {
              node.SwapItems(ci.GetA()[k], permutation[k]);
              permuted.Add(ci.GetA()[k]);
              if( permuted.IndexOf(permutation[k]) == -1 )
                permuted.Add(permutation[k]);
            }
          }
    
          for( int k=0; k < permutation.Count(); k++ )
            Nodes[ci.GetA()[k]].FullMatchEx(node[ci.GetA()[k]], analyser, true);
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
          // rewind...
          for( int k=0; k < permutation.Count(); k++ )  {
            if( permuted.IndexOf(ci.GetA()[k]) == -1 )
              node.SwapItems(ci.GetA()[k], permutation[k]);
          }
        }
        permutation = ci.GetB();
        GeneratePermutation(permutation, best_perm);
        for( int k=0; k < permutation.Count(); k++ )
          node.SwapItems(ci.GetA()[k], permutation[k]);
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
