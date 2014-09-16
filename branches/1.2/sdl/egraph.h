/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_egraph_H
#define __olx_sdl_egraph_H
#include "typelist.h"
#include "talist.h"
#include "etraverse.h"
#include "emath.h"
#include "edict.h"

template <class IC, class AssociatedOC> class TEGraphNode : ACollectionItem  {
  IC Data;
  typedef TEGraphNode<IC, AssociatedOC> NodeType;
  typedef olx_pair_t<TSizeList,TSizeList> ConnInfo;
  TPtrList<NodeType> Nodes;
  bool RingNode, Root;
  size_t GroupIndex;
  mutable bool Passed, Mutable;
  AssociatedOC Object;
  mutable olxdict<NodeType*, TTypeList<ConnInfo>, TPointerComparator> Connectivity;
  TTypeList<TSizeList> *Permutations;
protected:
  inline bool IsPassed() const {  return Passed;  }
  inline void SetPassed(bool v)  {  Passed = v;  }
  void SetPassedR(bool v)  {
    Passed = v;
    for( size_t i=0; i < Nodes.Count(); i++ )
      Nodes[i]->SetPassedR(v);
  }
  size_t CountR() const {
    size_t rv = Nodes.Count();
    for( size_t i=0; i < Nodes.Count(); i++ )
      rv += Nodes[i]->CountR();
    return rv;
  }

  static int _SortNodesByTag(const NodeType* n1, const NodeType* n2) {
    return n1->GetTag() - n2->GetTag();
  }

  TTypeList<ConnInfo>& GetConnInfo(NodeType& node) const {
    TTypeList<ConnInfo>& conn = Connectivity.Add(&node);
    if (!conn.IsEmpty()) return conn;
    const size_t node_cnt = Count();
    for (size_t i=0; i < node_cnt; i++) {
      for (size_t j=0; j < node_cnt; j++) {
        if (Nodes[i]->GroupIndex == node[j].GroupIndex &&
          Nodes[i]->GroupIndex != InvalidIndex)
        {
          if (!Nodes[i]->ShallowEquals(node[j])) { // sanity check
            throw TFunctionFailedException(__OlxSourceInfo,
              "due to graph connectivity");
          }
          bool found = false;
          for (size_t k=0; k < conn.Count(); k++) {
            if (conn[k].GetB().Contains(i)) {
              if (!conn[k].GetA().Contains(j))
                conn[k].a.Add(j);
              found = true;
              break;
            }
            else if (conn[k].GetA().Contains(j)) {
              if (!conn[k].GetB().Contains(i))
                conn[k].b.Add(i);
              found = true;
              break;
            }
          }
          if (!found) {
            ConnInfo& ci = conn.AddNew();
            ci.a.Add(j);
            ci.b.Add(i);
          }
        }
      }
    }
    return conn;
  }
public:
  static void GeneratePermutations(const TTypeList<ConnInfo>& conn,
    TTypeList<TSizeList>& res)
  {
    TSizeList permutation;
    size_t total_perm = 1,
        total_perm_size = 0,
        perm_size = 0,
        group_size = 1;
    for( size_t i=0; i < conn.Count(); i++ )  {
      total_perm *= olx_factorial_t<size_t, size_t>(conn[i].GetA().Count());
      total_perm_size += conn[i].GetA().Count();
    }
    // precreate the lists
    for( size_t i=0; i < total_perm; i++ )
      res.AddNew(total_perm_size);
    for( size_t i=0; i < conn.Count(); i++ )  {
      const ConnInfo& ci = conn[i];
      const size_t perm_cnt = olx_factorial_t<size_t, size_t>(ci.GetA().Count());
      for( size_t j=1; j < perm_cnt; j++ )
        for( size_t k=0; k < group_size; k++ )
          for( size_t l=0; l < perm_size; l++ )
            res[k+j*group_size][l] = res[k][l];
      for( size_t j=0; j < perm_cnt; j++ )  {
        permutation = ci.GetA();
        GeneratePermutation(permutation, j);
        for( size_t k=0; k < group_size; k++ )
          for( size_t l=0; l < permutation.Count(); l++ )
            res[j*group_size+k][perm_size+l] = permutation[l];
      }
      group_size *= perm_cnt;
      perm_size += ci.GetA().Count();
    }
  }
public:
  TEGraphNode(const IC& data, const AssociatedOC& object)
    : GroupIndex(InvalidIndex)
  {
    Data = data;
    Mutable = Root = Passed = RingNode = false;
    Object = object;
    Permutations = NULL;
  }
  ~TEGraphNode()  {
    Nodes.DeleteItems(false);
    if (Permutations != NULL)
      delete Permutations;
  }
  const IC & GetData() const { return Data; }
  void SetData(const IC &d) { Data = d; }
  bool IsRingNode() const {  return RingNode;  }
  bool IsMutable() const {  return Mutable;  }
  size_t GetGroupIndex() const {  return GroupIndex;  }
  void SetRingNode()  {  RingNode = true;  }

  TEGraphNode& NewNode(const IC& Data, const AssociatedOC& obj)  {
    return *Nodes.Add(new TEGraphNode(Data, obj));
  }
  void SortNodesByTag() {
    Nodes.BubleSorter.SortSF(Nodes, &TEGraphNode::_SortNodesByTag);
  }
  TPtrList<NodeType>& GetNodes() {  return Nodes;  }
  inline const AssociatedOC& GetObject() const {  return Object;  }

  inline size_t Count() const {  return Nodes.Count();  }
  // this is for the traverser
  inline TEGraphNode& Item(size_t i) const {  return  *Nodes[i];  }
  inline TEGraphNode& operator [](size_t i) const {  return  *Nodes[i];  }
  void SwapItems(size_t i, size_t j)  {
    if( i != j )
      Nodes.Swap(i,j);
  }
  bool ShallowEquals(const TEGraphNode& node) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    return true;
  }
  /* Checks if a graph node matches the other, on the way it rearranges nodes
  so that nodes match by index like this[i] == that[i]
  */
  bool DoMatch(TEGraphNode& node) const {
    if (!ShallowEquals(node)) return false;
    for( size_t i=0; i < node.Count(); i++ )
      node[i].SetPassed(false);
    TSizeList indices(Count());
    for( size_t i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( size_t j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( node[j].IsPassed() )  continue;
        if( Nodes[i]->DoMatch(node[j]) )  {
          node[j].SetPassed(true);
          indices[i] = j;  // sorting the nodes to match
          Matched = true;
          break;
        }
      }
      if( !Matched )  return false;
    }
    node.Nodes.Rearrange(indices);
    return true;
  }

  /* Compares graphs to establish their equality */
  bool DryMatch(TEGraphNode& node) const {
    if (!ShallowEquals(node)) return false;
    for( size_t i=0; i < node.Count(); i++ )
      node[i].SetPassed(false);
    for( size_t i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( size_t j=0; j < Count(); j++ )  {  // Count equals for both nodes
        if( node[j].IsPassed() )  continue;
        if( Nodes[i]->DoMatch(node[j]) )  {
          node[j].SetPassed(true);
          Matched = true;
          break;
        }
      }
      if( !Matched )  return false;
    }
    return true;
  }

  template <class Analyser> bool FullMatch(TEGraphNode& node,
    Analyser& analyser) const
  {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() != Count() )  return false;
    for( size_t i=0; i < Count(); i++ )  {
      size_t mc=0;
      for( size_t j=0; j < Count(); j++ )  {
        /* we cannot do node swapping here, since it will invalidate the
        matching indexes
        */
        if( Nodes[i]->FullMatch(node[j], analyser) )  {
           analyser.OnMatch(*this, node, i, j);
           mc++;
        }
      }
      if( mc == 0 )  return false;
    }
    return true;
  }

  bool AnalyseMutability(TEGraphNode& node, double& permutations)  {
    if (node.GetData() != GetData()) return false;
    if (node.Count() != Count()) return false;
    Mutable = false;
    for (size_t i=0; i < Count(); i++) {
      size_t mc = 0;
      for (size_t j=0; j < Count(); j++) {  // Count equals for both nodes
        if( j > i )  // do consistent node ordering for all non-unique
          Nodes[i]->DoMatch(*Nodes[j]);
        if (Nodes[i]->DoMatch(node[j]) &&
            Nodes[i]->AnalyseMutability(node[j], permutations))
        {
          if (node[j].GroupIndex == InvalidIndex) {
            mc++;
            if (Nodes[i]->GroupIndex == InvalidIndex)
              Nodes[i]->GroupIndex = node[j].GroupIndex = i;
            else
              node[j].GroupIndex = Nodes[i]->GroupIndex;
          }
          else {
            if (Nodes[i]->GroupIndex != InvalidIndex &&
                Nodes[i]->GroupIndex != node[j].GroupIndex)
            {
              throw TFunctionFailedException(__OlxSourceInfo, "assert");
            }
            Nodes[i]->GroupIndex = node[j].GroupIndex;
          }
        }
      }
      if (mc > 1) {
        Mutable = true;
        permutations *= olx_factorial_t<double,size_t>(mc);
      }
    }
    return true;
  }

  template <class Analyser> bool FullMatchEx(TEGraphNode& node,
    Analyser& analyser, bool analyse=false)
  {
    if( IsRoot() )  {
      double permutations = 1;
      this->AnalyseMutability(node, permutations);
      analyser.OnStart(permutations);
    }
    if (!ShallowEquals(node)) return false;
    const size_t node_cnt = Count();
    if( node_cnt == 0  )  return true;
    TTypeList<ConnInfo>& conn = GetConnInfo(node);
    size_t dest_ind = 0;
    TSizeList dest(node_cnt);
    for (size_t i=0; i < conn.Count(); i++) {
      const ConnInfo& ci = conn[i];
      for (size_t j=0; j < ci.GetB().Count(); j++)
        dest[dest_ind++] = ci.GetB()[j];
    }
    if( dest_ind != node_cnt )
      return false;
    TPtrList<TEGraphNode> ond(node.Nodes);
    if (Permutations == NULL) {
      Permutations = new TTypeList<TSizeList>();
      GeneratePermutations(conn, *Permutations);
    }
    TTypeList<TSizeList> &permutations = *Permutations;
    const size_t perm_cnt = permutations.Count();
    size_t best_perm = 0;
    double minRms = -1;

    for( size_t i=0; i < permutations.Count(); i++ )  {
      const TSizeList& permutation = permutations[i];
      for( size_t j=0; j < node_cnt; j++ )
        node.Nodes[dest[j]] = ond[permutation[j]];
      for( size_t j=0; j < node_cnt; j++ )
        Nodes[j]->FullMatchEx(node[j], analyser, true);
      const double rms = analyser.CalcRMS(*this, node);
      if( rms < 0 )
        continue;
      else if( rms < 1e-5 )  {// must be there
        best_perm = InvalidIndex; // specify that we stopped at the best one
        break;
      }
      if( minRms < 0 || rms < minRms )  {
        minRms = rms;
        best_perm = i;
      }
    }
    if( best_perm != InvalidIndex && best_perm != perm_cnt-1 )  {
      const TSizeList& permutation = permutations[best_perm];
      for( size_t j=0; j < node_cnt; j++ )
        node.Nodes[dest[j]] = ond[permutation[j]];
      for( size_t j=0; j < node_cnt; j++ )
        Nodes[j]->FullMatchEx(node[j], analyser, true);
    }
    if( IsRoot() )
      analyser.OnFinish();
    return true;
  }

  bool IsSubgraphOf(TEGraphNode& node) const {
    if( node.GetData() != GetData() )  return false;
    if( node.Count() < Count() )  return false;
    for( size_t i=0; i < node.Count(); i++ )
      node[i].SetPassed(false);
    for( size_t i=0; i < Count(); i++ )  {
      bool Matched = false;
      for( size_t j=0; j < node.Count(); j++ )  {
        if( Nodes[i]->IsSubgraphOf(node[j]) )  {
          node[j].SetPassed(true);
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
  DefPropBIsSet(Root)
  static TGraphTraverser< TEGraphNode<IC, AssociatedOC> > Traverser;
};

#ifndef __BORLANDC__
template<typename IC, typename AssociatedOC>
TGraphTraverser< TEGraphNode<IC,AssociatedOC> >
  TEGraphNode<IC,AssociatedOC>::Traverser;
#endif

template <class IC, class AssociatedOC>
class TEGraph {
  TEGraphNode<IC, AssociatedOC> Root;
public:
  TEGraph( const IC& Data, const AssociatedOC& obj) : Root(Data, obj) {
    Root.SetRoot(true);
  }
  TEGraphNode<IC, AssociatedOC>& GetRoot()  {  return Root;  }

  size_t Count() const {  return 1;  }

  const TEGraphNode<IC, AssociatedOC>& Item(size_t i) const {
    return Root;
  }
};
#endif
