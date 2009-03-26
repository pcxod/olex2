#ifndef egraphH
#define egraphH
#include "typelist.h"
#include "etraverse.h"
#include "emath.h"
#include "edict.h"
//---------------------------------------------------------------------------

template <class IC, class AssociatedOC> class TEGraphNode : ACollectionItem  {
  IC Data;
  typedef TEGraphNode<IC, AssociatedOC> NodeType;
  typedef AnAssociation2<TIntList,TIntList> ConnInfo;
  TPtrList<NodeType> Nodes;
  bool RingNode, Root;
  mutable bool Passed, Mutable;
  mutable TEGraphNode* PassedFor;
  mutable TPtrList<NodeType> PassedForNodes;
  AssociatedOC Object;
  mutable olxdict<NodeType*, TTypeList<ConnInfo>, TPointerComparator> Connectivity;
protected:
  inline bool IsPassed()  const  {  return Passed;  }
  inline void SetPassed(bool v)  {  Passed = v;  }
  static int _SortNodesByTag(const NodeType* n1, const NodeType* n2) {
    return n1->GetTag() - n2->GetTag();
  }
  template <class Analyser>
  TTypeList<ConnInfo>& GetConnInfo(NodeType& node, Analyser& analyser) const {
    TTypeList<ConnInfo>& conn = Connectivity.Add(&node);
    if( !conn.IsEmpty() )  return conn;
    const int node_cnt = Count();
    for( int i=0; i < node_cnt; i++ )  {
      for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
        if( Nodes[i]->FullMatchEx( node[j], analyser, false ) )  {
          bool found = false;
          for( int k=0; k < conn.Count(); k++ )  {
            if( conn[k].GetB().IndexOf(i) != -1 )  {
              if( conn[k].GetA().IndexOf(j) == -1 )
                conn[k].A().Add(j);
              found = true;
              break;
            }
            else if( conn[k].GetA().IndexOf(j) != -1 )  {
              if( conn[k].GetB().IndexOf(i) == -1 )
                conn[k].B().Add(i);
              found = true;
              break;
            }
          }
          if( !found )  {
            ConnInfo& ci = conn.AddNew();
            ci.A().Add(j);
            ci.B().Add(i);
          }
        }
      }
    }
    return conn;
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
    PassedFor = NULL;
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
  TPtrList<NodeType>& GetNodes() {  return Nodes;  }
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
  bool IsShallowEqual( const TEGraphNode& node ) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    return true;
  }
  bool DoMatch( TEGraphNode& node )  const {
    if( node.GetData() != GetData() )  return false;
    //if( IsRingNode() )  return true;
    if( node.Count() != Count() )  return false;
    if( Count() == 0 )  return true;
    for( int i=0; i < node.Count(); i++ )
      node[i].SetPassed( false );
    TIntList indeces(Count());
    for( int i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( int j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( node[j].IsPassed() )  continue;
        if( Nodes[i]->DoMatch(node[j]) )  {
          node[j].SetPassed( true );
          indeces[i] = j;  // sorting the nodes to match
          Matched = true;
          break;
        }
      }
      if( !Matched )  return false;
    }
    node.Nodes.Rearrange(indeces);
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
    if( (!analyse || !Mutable) && !IsRoot() )  {
      if( PassedFor == &node )  {
        node.Nodes = PassedForNodes;
        for( int i=0; i < node_cnt; i++ )
          if( !Nodes[i]->FullMatchEx(node[i], analyser, analyse) )  // this should never happen...
            throw TFunctionFailedException(__OlxSourceInfo, "the matching has failed");
        return true;
      }
      for( int i=0; i < node_cnt; i++ )
        node[i].SetPassed( false );
      TIntList matches(node_cnt);
      for( int i=0; i < node_cnt; i++ )  {
        bool Matched = false;
        for( int j=0; j < node_cnt; j++ )  {  // Count equals for both nodes
          if( node[j].IsPassed() )  continue;
          if( Nodes[i]->DoMatch(node[j]) )  {
            node[j].SetPassed( true );
            matches[i] = j;
            Matched = true;
            break;
          }
        }
        if( !Matched )  return false;
      }
      node.Nodes.Rearrange(matches);
      for( int i=0; i < node_cnt; i++ )
        if( !Nodes[i]->FullMatchEx(node[i], analyser, analyse) )   // this should never happen...
          throw TFunctionFailedException(__OlxSourceInfo, "the matching has failed");
      PassedFor = &node;
      PassedForNodes = node.Nodes;
      return true;
    }
    TTypeList<ConnInfo>& conn = GetConnInfo(node, analyser);
    int dest_ind = 0;
    TIntList dest(node_cnt);
    for( int i=0; i < conn.Count(); i++ )  {
      const ConnInfo& ci = conn[i];
      for( int j=0; j < ci.GetB().Count(); j++ )
        dest[dest_ind++] = ci.GetB()[j];
    }
    if( dest_ind != node_cnt )
      return false;
    TPtrList<TEGraphNode> ond(node.Nodes);
    TTypeList<TIntList> permutations;
    GeneratePermutations(conn, permutations);
    const int perm_cnt = permutations.Count();
    int best_perm = 0;
    double minRms = -1;


    for( int i=0; i < permutations.Count(); i++ )  {
      const TIntList& permutation = permutations[i];
      for( int j=0; j < node_cnt; j++ )
        node.Nodes[dest[j]] = ond[permutation[j]];
      for( int j=0; j < node_cnt; j++ )
        Nodes[j]->FullMatchEx(node[j], analyser, true);
      const double rms = analyser.CalcRMS(*this, node);
      if( rms < 0 )
        continue;
      else if( rms < 1e-5 )  {// must be there
        best_perm = -1; // specify that we stopped at the best one
        break;
      }
      if( minRms < 0 || rms < minRms )  {
        minRms = rms;
        best_perm = i;
      }
    }
    if( best_perm >= 0 && best_perm != perm_cnt-1 )  {
      const TIntList& permutation = permutations[best_perm];
      for( int j=0; j < node_cnt; j++ )
        node.Nodes[dest[j]] = ond[permutation[j]];
      for( int j=0; j < node_cnt; j++ )
        Nodes[j]->FullMatchEx(node[j], analyser, true);
    }
    if( IsRoot() )
      analyser.OnFinish();
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
