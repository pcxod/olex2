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
  static void GeneratePermutations(const TTypeList<ConnInfo>& conn, TTypeList<TIntList>& res)  {
    TTypeList<TIntList> op;
    TIntList permutation;
    int total_perm = 1, 
        total_perm_size = 0,
        perm_size = 0, 
        group_size = 1;
    for( int i=0; i < conn.Count(); i++ )  {
      total_perm *= (int)Factorial(conn[i].GetA().Count());
      total_perm_size += conn[i].GetA().Count();
    }
    // precreate the lists
    for( int i=0; i < total_perm; i++ )
      res.AddNew(total_perm_size);
    for( int i=0; i < conn.Count(); i++ )  {
      const ConnInfo& ci = conn[i];
      const int perm_cnt = (int)Factorial(ci.GetA().Count());
      const int repeat_n = total_perm/(perm_cnt*group_size);
      for( int j=1; j < perm_cnt; j++ )
        for( int k=0; k < group_size; k++ )
          for( int l=0; l < perm_size; l++ )
            res[k+j*group_size][l] = res[k][l];
      for( int j=0; j < perm_cnt; j++ )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, j);
        for( int k=0; k < group_size; k++ )
          for( int l=0; l < permutation.Count(); l++ )
            res[j*group_size+k][perm_size+l] = permutation[l];
      }
      group_size *= perm_cnt;
      perm_size += ci.GetA().Count();
    }
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
    if( node_cnt == 0  )  return true;
    if( (!analyse && !IsRoot()) || !Mutable )  {
      for( int i=0; i < node_cnt; i++ )
        node[i].SetPassed( false );
      TIntList matches(node_cnt);
      for( int i=0; i < node_cnt; i++ )  {
        bool Matched = false;
        for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
          if( node[j].IsPassed() )  continue;
          if( Nodes[i]->FullMatchEx(node[j], analyser, analyse) )  {
            node[j].SetPassed( true );
            matches[j] = i;
            Matched = true;
            break;
          }
        }
        if( !Matched )  return false;
      }
      node.Nodes.Rearrange(matches);
      return true;
    }
    TTypeList< ConnInfo > conn;
    for( int i=0; i < node_cnt; i++ )  {
      for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
        if( Nodes[i]->FullMatchEx( node[j], analyser, analyse ) )  {
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
        Nodes[i]->FullMatchEx(node[i], analyser, true);
      return true;
    }
    TPtrList<TEGraphNode> ond(node.Nodes);
    TIntList permutation;
    if( conn.Count() == 1 )  {
      const ConnInfo& ci = conn[0];
      const int perm_cnt = (int)Factorial(ci.GetA().Count());
      int best_perm = 0;
      double minRms;
      for( int i=0; i < perm_cnt; i++ )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, i);
        for( int j=0; j < permutation.Count(); j++ )
          node.Nodes[ci.GetA()[j]] = ond[permutation[j]];
        for( int j=0; j < node_cnt; j++ )
          Nodes[j]->FullMatchEx(node[j], analyser, true);
        //const double rms = analyser.CalcRMS(*this, node);
        const double rms = analyser.CalcRMS();
        if( i == 0 )
          minRms = rms;
        else if( rms < minRms )  {
          minRms = rms;
          best_perm = i;
        }
      }
      if( best_perm != perm_cnt-1 )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, best_perm);
        for( int j=0; j < permutation.Count(); j++ )
          node.Nodes[ci.GetA()[j]] = ond[permutation[j]];
        for( int j=0; j < node_cnt; j++ )
          Nodes[j]->FullMatchEx(node[j], analyser, true);
      }
    }
    else  {
      TTypeList<TIntList> permutations;
      GeneratePermutations(conn, permutations);
      const int perm_cnt = permutations.Count();
      const TIntList& original_perm = permutations[0];
      const int perm_size = permutations[0].Count();
      int best_perm = 0;
      double minRms;
      for( int i=0; i < permutations.Count(); i++ )  {
        const TIntList& permutation = permutations[i];
        for( int j=0; j < perm_size; j++ )
          node.Nodes[original_perm[j]] = ond[permutation[j]];
        for( int j=0; j < node_cnt; j++ )
          Nodes[j]->FullMatchEx(node[j], analyser, true);
        const double rms = analyser.CalcRMS();
        if( i == 0 )
          minRms = rms;
        else if( rms < minRms )  {
          minRms = rms;
          best_perm = i;
        }
      }
      if( best_perm != perm_cnt-1 )  {
        const TIntList& permutation = permutations[best_perm];
        for( int j=0; j < permutation.Count(); j++ )
          node.Nodes[original_perm[j]] = ond[permutation[j]];
        for( int j=0; j < node_cnt; j++ )
          Nodes[j]->FullMatchEx(node[j], analyser, true);
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
