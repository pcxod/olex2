/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_network_H
#define __olx_xl_network_H
#include "math/align.h"
#include "evalue.h"
#include "satom.h"
#include "sbond.h"
#include "tptrlist.h"
#include "conninfo.h"
#include "atomregistry.h"
#include "linked_list.h"

BeginXlibNamespace()

const int DefNoPart = -100;

typedef TTypeListExt<class TNetwork, IOlxObject> TNetList;
typedef TPtrList<class TNetwork> TNetPList;

class TNetwork: public TBasicNode<TNetwork, TSAtom, TSBond>  {
protected:
  class TLattice  *Lattice;
  // sorts atoms by distance from 0, must be called before search for Hbonds
  void SortNodes();
public:
  TNetwork(TLattice* P, TNetwork *N);
  virtual ~TNetwork()  {}

  TLattice& GetLattice() const {  return *Lattice;  }
  // empties the content of the network
  void Clear();
  // adds a node to the network and assigns its NetId
  void AddNode(TSAtom& N)  { Nodes.Add(N)->SetFragmentId(NodeCount());  }
  // adds a bond to the network and assigns its NetId
  void AddBond(TSBond& B)  {  Bonds.Add(B)->SetFragmentId(BondCount());  }

  // copies the content of S to the net
  void Assign(TNetwork& S);

  /* disassembles the list of Atoms into the fragments; does not affect current
  net
  */
  void Disassemble(ASObjectProvider& objects, TNetPList& Frags);
  /* creates bonds and fragments for atoms initialised by Disassemble
  */
  void CreateBondsAndFragments(ASObjectProvider& objects, TNetPList& Frags);
  // returns true if the two atoms share a matrix
  static bool HaveSharedMatrix(const TSAtom& sa, const TSAtom& sb);
  static bool IsBondAllowed(const TSAtom& sa, const TSAtom& sb)  {
    if( sa.CAtom().GetPart() == 0 || sb.CAtom().GetPart() == 0 ||
       (sa.CAtom().GetPart() == sb.CAtom().GetPart()) )
    {
      if ((sa.CAtom().GetPart() < 0 || sb.CAtom().GetPart() < 0))
        return HaveSharedMatrix(sa, sb);
      return true;
    }
    return false;
  }
  static bool IsBondAllowed(const TSAtom& sa, const TCAtom& cb,
    const smatd& sm)
  {
    if( sa.CAtom().GetPart() == 0 || cb.GetPart() == 0 ||
       (sa.CAtom().GetPart() == cb.GetPart()) )
    {
      if ((sa.CAtom().GetPart() < 0 || cb.GetPart() < 0))
        return sa.IsGenerator(sm);
      return true;
    }
    return false;
  }

  static bool IsBondAllowed(const TCAtom& ca, const TCAtom& cb,
    const smatd& sm);

  static bool IsBondAllowed(const TCAtom& ca, const TCAtom& cb)  {
    return (ca.GetPart() == 0 || cb.GetPart() == 0 ||
            ca.GetPart() == cb.GetPart());
  }

  bool CBondExists(const TSAtom& A1, const TSAtom& CA2, const double& D) const;
  // considers quadratic distance
  bool CBondExistsQ(const TSAtom& A1, const TSAtom& CA2,
    const double& qD) const;

  bool CBondExists(const TCAtom& CA1, const TCAtom& CA2,
    const smatd& sm, const double& D) const;
  // compares the quadratic distances
  bool CBondExistsQ(const TCAtom& CA1, const TCAtom& CA2,
    const smatd& sm, const double& qD) const;

  bool HBondExists(const TCAtom& CA1, const TCAtom& CA2,
    const smatd& sm, const double& D) const;
  // compares the quadratic distances
  bool HBondExistsQ(const TCAtom& CA1, const TCAtom& CA2,
    const smatd& sm, const double& qD) const;

  static bool BondExists(const TSAtom& a1, const TSAtom& a2, double D,
    double delta)
  {
    if (D < (a1.CAtom().GetConnInfo().r + a2.CAtom().GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2);
    return false;
  }
  static bool BondExistsQ(const TSAtom& a1, const TSAtom& a2, double qD,
    double delta)
  {
    if (qD < olx_sqr(
      a1.CAtom().GetConnInfo().r + a2.CAtom().GetConnInfo().r + delta))
    {
      return IsBondAllowed(a1, a2);
    }
    return false;
  }
  static bool BondExists(const TSAtom& a1, const TCAtom& a2, const smatd& m,
    double D, double delta)
  {
    if (D < (a1.CAtom().GetConnInfo().r + a2.GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExistsQ(const TSAtom& a1, const TCAtom& a2, const smatd& m,
    double qD, double delta)
  {
    if (qD < olx_sqr(
      a1.CAtom().GetConnInfo().r + a2.GetConnInfo().r + delta))
    {
      return IsBondAllowed(a1, a2, m);
    }
    return false;
  }
  static bool BondExists(const TCAtom& a1, const TCAtom& a2, const smatd& m,
    double D, double delta)
  {
    if (D < (a1.GetConnInfo().r + a2.GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExistsQ(const TCAtom& a1, const TCAtom& a2, const smatd& m,
    double qD, double delta)
  {
    if (qD < olx_sqr(a1.GetConnInfo().r + a2.GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExists(const TCAtom& a1, const TCAtom& a2, double D,
    double delta)
  {
    if (D < (a1.GetConnInfo().r + a2.GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2);
    return false;
  }
  static bool BondExistsQ(const TCAtom& a1, const TCAtom& a2, double qD,
    double delta)
  {
    if (qD < olx_sqr(a1.GetConnInfo().r + a2.GetConnInfo().r + delta))
      return IsBondAllowed(a1, a2);
    return false;
  }

  // only pointers are compared!!
  bool operator == (const TNetwork& n) const {  return this == &n;  }
  bool operator == (const TNetwork* n) const {  return this == n;  }
  bool operator != (const TNetwork& n) const {  return this != &n;  }
  bool operator != (const TNetwork* n) const {  return this != n;  }

  // returns true if the ring is regular (distances from centroid and angles)
  static bool IsRingRegular(const TSAtomPList& ring);
  static bool IsRingPrimitive(const TSAtomPList& ring);
  // invertion must be specified for the permutational graph match
  bool DoMatch(TNetwork& net, TTypeList< olx_pair_t<size_t, size_t> >& res,
    bool Invert,
    double (*weight_calculator)(const TSAtom&));
  bool IsSubgraphOf(TNetwork& net,
    TTypeList< olx_pair_t<size_t, size_t> >& res,
    const TSizeList& rootsToSkip);

protected:
  static int TNetwork_SortRingAtoms(const TSAtom &a, const TSAtom &b)  {
    return (int)(a.GetTag()-b.GetTag());
  }
  static index_t ShortestDistance(TSAtom &a, TSAtom &b);
  static bool TryRing(TSAtom& sa, TSAtomPList& ring,
    const ElementPList& ringContent, size_t level=1);
  static bool TryRing(TSAtom& sa, TSAtomPList& ring, size_t level=1);
// tries to find the ring in given direction
  static bool TryRing(TSAtom& sa, size_t node, TSAtomPList& ring,
    const ElementPList& ringContent);
  static bool TryRing(TSAtom& sa, size_t node, TSAtomPList& ring);
  void UnifyRings(TTypeList<TSAtomPList>& rings);

public:
  // finds only primitive rings
  void FindRings(const ElementPList& ringContent, TTypeList<TSAtomPList>& res);
  // returns a content of this fragment
  ContentList GetContentList() const;
  olxstr GetFormula() const;
  void FindAtomRings(TSAtom& ringAtom, const ElementPList& ringContent,
        TTypeList<TSAtomPList>& res);
  // finds all rings
  void FindAtomRings(TSAtom& ringAtom, TTypeList<TSAtomPList>& res);
  struct RingInfo  {
    const cm_Element* HeaviestSubsType;
    size_t MaxSubsANode, HeaviestSubsIndex;
    TSizeList Ternary, // three bond inside the ring
      Substituted,    // more than two connections, two belong to the ring
      Alpha;          // susbtituted next to a ternary atom
    TTypeList<TSAtomPList> Substituents;
    bool HasAfix;
    RingInfo()
      : HeaviestSubsType(NULL), MaxSubsANode(0),
        HeaviestSubsIndex(InvalidIndex), HasAfix(false)  {}
    RingInfo& Clear()  {
      MaxSubsANode = 0;
      HeaviestSubsIndex = InvalidIndex;
      HasAfix = false;
      Ternary.Clear();
      Substituted.Clear();
      Alpha.Clear();
      Substituents.Clear();
      return *this;
    }
    // returns true if all substituents are single CHn groups
    bool IsSingleCSubstituted() const;
  };
  static RingInfo& AnalyseRing(const TSAtomPList& ring, RingInfo& ri);
  /* returns true of the fragment has enough nodes to be matched/aligned to
  others
  */
  bool IsSuitableForMatching() const {
    size_t cnt=0;
    for (size_t i=0; i < NodeCount(); i++) {
      if (Node(i).CAtom().IsAvailable() && ++cnt > 3)
        return true;
    }
    return false;
  }
  struct AlignInfo  {
    align::out align_out;
    TEValueD rmsd;  // unweighted rmsd
    bool inverted;
  };
  static AlignInfo GetAlignmentRMSD(
    const TTypeList< olx_pair_t<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&),
    bool reset_crd=true
    );
  // prepares a list of atoms, coordinates and weights for VcoV calculations
  static void PrepareESDCalc(
    const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
    bool TryInversion,
    TSAtomPList& atoms_out,
    vec3d_alist& crd_out,
    TDoubleList& wght_out,
    double (*weight_calculator)(const TSAtom&));

  static void DoAlignAtoms(const TSAtomPList& atomsToTransform,
    const AlignInfo& align_info);
  // new = (old-center)*m + shift
  static void DoAlignAtoms(const TSAtomPList& atomsToTransform,
    const vec3d &center,
    const mat3d &m, const vec3d &shift);

  static TArrayList<align::pair>& AtomsToPairs(
    const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&),
    TArrayList<align::pair>& pairs,
    bool reset_crd=true);

  static align::out GetAlignmentInfo(
    const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&),
    bool reset_crd=true);
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
};

EndXlibNamespace()
#endif
