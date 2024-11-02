/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "network.h"
#include "satom.h"
#include "sbond.h"
#include "actions.h"
#include "lattice.h"
#include "unitcell.h"
#include "asymmunit.h"
#include "xapp.h"
#include "log.h"
#include "egraph.h"
#include "olxmps.h"
#include "estopwatch.h"
#include "edict.h"
#include "emath.h"
#include "refmodel.h"
#include "index_range.h"
#include "equeue.h"
#include "afixgroup.h"

#undef GetObject

TNetwork::TNetwork(TLattice* P, TNetwork *N)
  : TBasicNode<TNetwork, TSAtom, TSBond>(N)
{
  Lattice = P;
  SetTag(-1);
}
//..............................................................................
void TNetwork::SortNodes()  {
  QuickSorter::Sort(Nodes, TSAtom::SortByDistance());
}
//..............................................................................
void TNetwork::CreateBondsAndFragments(ASObjectProvider& objects,
  TNetPList& Frags)
{
  objects.atoms.ForEach(ACollectionItem::TagSetter(1));
  const size_t ac = objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& A1 = objects.atoms[i];
    if (A1.IsDeleted()) {
      continue;
    }
    A1.SetStandalone(A1.NodeCount() == 0);
    if (A1.GetTag() != 0) {
      TNetwork* Net = Frags.Add(new TNetwork(&GetLattice(), this));
      Net->AddNode(A1);
      A1.SetNetwork(*Net);
      A1.SetTag(0);
      for (size_t j = 0; j < Net->NodeCount(); j++) {
        TSAtom& A2 = Net->Node(j);
        const size_t a2_cnt = A2.NodeCount();
        for (size_t k = 0; k < a2_cnt; k++) {
          TSAtom& A3 = A2.Node(k);
          if (A3.GetTag() != 0) {
            Net->AddNode(A3);
            A3.SetNetwork(*Net);
            TSBond& B = objects.bonds.New(Net);
            B.SetType(sotBond);
            B.SetA(A2);
            B.SetB(A3);
            A2.AddBond(B);
            A3.AddBond(B);
            Net->AddBond(B);
            A3.SetTag(0);
          }
          else if (A3.GetIdInNetwork() > j) {  // the atom is in the list, but has not been processes
            TSBond& B = objects.bonds.New(Net);  // in this case we need to create a bond
            B.SetType(sotBond);
            B.SetA(A2);
            B.SetB(A3);
            A2.AddBond(B);
            A3.AddBond(B);
            Net->AddBond(B);
          }
        }
      }
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(ASObjectProvider& objects, TNetPList& Frags) {
  //define the cosine of minimum allowed angle
  const double cos_th = cos(TXApp::GetMinHBondAngle()*M_PI / 180);
  const TUnitCell& uc = Lattice->GetUnitCell();
  const size_t ac = objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = objects.atoms[i];
    sa.ClearBonds();
    sa.ClearNodes();
    if (!sa.IsAvailable()) {
      continue;
    }
    for (size_t j = 0; j < sa.CAtom().AttachedSiteCount(); j++) {
      TCAtom::Site& site = sa.CAtom().GetAttachedSite(j);
      const smatd m = uc.MulMatrix(site.matrix, sa.GetMatrix());
      TSAtom* a = objects.atomRegistry.Find(
        TSAtom::GetRef(*site.atom, m));
      if (a != 0 && a->IsAvailable()) {
        sa.AddNode(*a);
      }
    }
  }
  // in second pass - Nodes have to get initialised
  SortedObjectList<int, TPrimitiveComparator> &from = TXApp::GetInteractionsFrom();
  SortedObjectList<int, TPrimitiveComparator> &to = TXApp::GetInteractionsTo();
  objects.atoms.ForEach(ACollectionItem::TagSetter(0));
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.IsAvailable() || sa.GetTag() != 0) {
      continue;
    }
    const cm_Element& thisT = sa.GetType();
    if (!from.Contains(thisT.z)) {
      continue;
    }
    for (size_t j = 0; j < sa.CAtom().AttachedSiteICount(); j++) {
      TCAtom::Site& site = sa.CAtom().GetAttachedSiteI(j);
      const cm_Element& thatT = site.atom->GetType();
      if (!to.Contains(thatT.z)) {
        continue;
      }
      const smatd m = sa.GetMatrix().IsFirst() ? site.matrix :
        uc.MulMatrix(site.matrix, sa.GetMatrix());
      TSAtom* a = objects.atomRegistry.Find(
        TSAtom::Ref(*site.atom, m.GetId()));
      if (a == 0 || !a->IsAvailable()) {
        for (size_t k = 0; k < site.atom->EquivCount(); k++) {
          uint32_t id = uc.MulMatrixId(site.atom->GetEquiv(k), m);
          a = objects.atomRegistry.Find(
            TSAtom::Ref(*site.atom, id));
          if (a != 0 && a->IsAvailable()) {
            break;
          }
        }
      }
      if (a != 0) {
        a->SetTag(1);
        bool process = true;
        double min_cs = cos_th;
        for (size_t k = 0; k < sa.NodeCount(); k++) {
          if (sa.Node(k).GetType() != iQPeakZ) {
            if (a->IsConnectedTo(sa.Node(k))) {
              process = false;
              break;
            }
            else {
              try {
                double cs = (sa.Node(k).crd() - sa.crd()).CAngle(a->crd() - sa.crd());
                if (cs < min_cs) {
                  min_cs = cs;
                }
              }
              catch (const TDivException &e) {
                TBasicApp::NewLogEntry(logError) << "Check the following atoms: "
                  << sa.GetLabel() << ", " << a->GetLabel() << " and "
                  << sa.Node(k).GetLabel();
                process = false;
                break;
              }
            }
          }
        }
        if (!process || min_cs >= cos_th) {
          continue;
        }
        TSBond& B = objects.bonds.New(&Lattice->GetNetwork());
        B.SetType(sotHBond);
        B.SetA(sa);
        B.SetB(*a);
        a->AddBond(B);
        sa.AddBond(B);
      }
    }
  }
  CreateBondsAndFragments(objects, Frags);
}
//..............................................................................
bool TNetwork::CBondExists(const TCAtom& CA1, const TCAtom& CA2,
  const smatd& sm, const double& D) const
{
  if (D <
    (CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDelta()))
  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExistsQ(const TCAtom& CA1, const TCAtom& CA2,
  const smatd& sm, const double& qD) const
{
  if (qD <
    olx_sqr(CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDelta()))
  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExists(const TCAtom& CA1, const TCAtom& CA2,
  const smatd& sm, const double& D) const
{
  if (D <
    (CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDeltaI()))
  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExistsQ(const TCAtom& CA1, const TCAtom& CA2,
  const smatd& sm, const double& qD) const
{
  if (qD < olx_sqr(
    CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDeltaI()))
  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExists(const TSAtom& A1, const TSAtom& A2,
  const double& D) const
{
  if (D < (A1.CAtom().GetConnInfo().r +
    A2.CAtom().GetConnInfo().r + GetLattice().GetDelta()))
  {
    return IsBondAllowed(A1, A2);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExistsQ(const TSAtom& A1, const TSAtom& A2,
  const double& qD) const
{
  if (qD < olx_sqr(A1.CAtom().GetConnInfo().r +
    A2.CAtom().GetConnInfo().r + GetLattice().GetDelta()))
  {
    return IsBondAllowed(A1, A2);
  }
  return false;
}
//..............................................................................
bool TNetwork::IsBondAllowed(const TCAtom& ca, const TCAtom& cb) {
  if (ca.GetPart() == 0 || cb.GetPart() == 0 ||
    (ca.GetPart() == cb.GetPart()) ||
    ca.GetParent()->GetRefMod()->Conn.ArePartsGroupped(ca.GetPart(), cb.GetPart()))
  {
    if (ca.GetParentAfixGroup() == cb.GetParentAfixGroup() &&
      ca.GetParentAfixGroup() != 0)
    {
      if (ca.GetParentAfixGroup()->GetM() == 12) {
        return false;
      }
    }
    return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::IsBondAllowed(const TCAtom& ca, const TCAtom& cb,
  const smatd& sm)
{
  if (IsBondAllowed(ca, cb)) {
    if (ca.GetPart() < 0 || cb.GetPart() < 0) {
      if (ca.GetPart() == 0 || cb.GetPart() == 0) {
        return true;
      }
      if (sm.IsFirst()) {
        return true;
      }
      for (size_t i = 0; i < ca.EquivCount(); i++) {
        if (sm.GetId() == ca.GetEquiv(i).GetId()) {
          return true;
        }
      }
      const TUnitCell &uc = ca.GetParent()->GetLattice().GetUnitCell();
      for (size_t i = 0; i < cb.EquivCount(); i++) {
        if (smatd::IsFirst(uc.MulMatrixId(cb.GetEquiv(i), sm))) {
          return true;
        }
      }
      return false;
    }
    return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::IsBondAllowed(const TSAtom& sa, const TSAtom& sb) {
  if (IsBondAllowed(sa.CAtom(), sb.CAtom())) {
    if ((sa.CAtom().GetPart() < 0 || sb.CAtom().GetPart() < 0)) {
      if (sa.CAtom().GetPart() == 0 || sb.CAtom().GetPart() == 0) {
        return true;
      }
      return HaveSharedMatrix(sa, sb);
    }
    return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::IsBondAllowed(const TSAtom& sa, const TCAtom& cb,
  const smatd& sm)
{
  if (IsBondAllowed(sa.CAtom(), cb)) {
    if ((sa.CAtom().GetPart() < 0 || cb.GetPart() < 0)) {
      if (sa.CAtom().GetPart() == 0 || cb.GetPart() == 0) {
        return true;
      }
      return sa.IsGenerator(sm);
    }
    return true;
  }
  return false;
}
//..............................................................................
// HELPER function
class TNetTraverser {
  olxstr Data;
public:
  TNetTraverser() { Data.SetIncrement(1024); }
  bool OnItem(const TEGraphNode<uint64_t, TSAtom*>& v) {
    olxstr tmp(v.GetData(), v.Count() * 6);
    tmp << '{';
    for (size_t i = 0; i < v.Count(); i++) {
      tmp << v.Item(i).GetData();
      if ((i + 1) < v.Count()) {
        tmp << ',';
      }
    }
    tmp << '}';
    Data << tmp;
    return true;
  }
  const olxstr& GetData() const { return Data; }
  void ClearData() { Data.SetLength(0); }
};

void ResultCollector(
  TEGraphNode<uint64_t, TSAtom*>& subRoot,
  TEGraphNode<uint64_t, TSAtom*>& Root,
  TTypeList<olx_pair_t<size_t, size_t> >& res)
{
  res.AddNew(subRoot.GetObject()->GetIdInNetwork(),
    Root.GetObject()->GetIdInNetwork());
  subRoot.GetObject()->SetTag(0);
  Root.GetObject()->SetTag(0);
  for (size_t i = 0; i < olx_min(subRoot.Count(), Root.Count()); i++) {
    if (subRoot[i].GetObject()->GetTag() != 0 &&
      Root[i].GetObject()->GetTag() != 0)
    {
      ResultCollector(subRoot[i], Root[i], res);
    }
  }
}
void ResultCollector(
  TEGraphNode<uint64_t,TSAtom*>& subRoot,
  TEGraphNode<uint64_t,TSAtom*>& Root,
  TTypeList< olx_pair_t<TSAtom*, TSAtom*> >& res)
{
  if (!subRoot.ShallowEquals(Root)) {
    return;
  }
  res.AddNew(subRoot.GetObject(), Root.GetObject());
  subRoot.GetObject()->SetTag(0);
  Root.GetObject()->SetTag(0);
  for( size_t i=0; i < subRoot.Count(); i++ ) {
    if( subRoot[i].GetObject()->GetTag() != 0 &&
      Root[i].GetObject()->GetTag() != 0 )
    {
      ResultCollector(subRoot[i], Root[i], res);
    }
  }
}

/* calculates a node idenifier based on the atom type and the breadth first
graph size and complexity
*/
uint64_t CalculateNodeHash(TEGraphNode<uint64_t, TSAtom*>& graphNode) {
  typedef TEGraphNode<uint64_t, TSAtom*> node_t;
  TNetwork& net = graphNode.GetObject()->GetNetwork();
  net.GetNodes().ForEach(ACollectionItem::TagSetter(-1));
  TQueue<TSAtom*> aqueue;
  aqueue.Push(graphNode.GetObject());
  aqueue.Push(0);
  index_t v = 0;
  TArrayList<size_t> counts(16, olx_list_init::zero());
  while (!aqueue.IsEmpty()) {
    TSAtom* n = aqueue.Pop();
    if (n == 0) {
      if (aqueue.IsEmpty()) {
        break;
      }
      v++;
      aqueue.Push(0);
      continue;
    }
    if (n->GetTag() != -1) {
      continue;
    }
    n->SetTag(v);
    if (counts.Count() <= (size_t)v) {
      counts.SetCount(v + 1, olx_list_init::zero());
    }
    counts[v] += (size_t)n->GetType().z;
    for (size_t i = 0; i < n->NodeCount(); i++) {
      if (n->Node(i).GetTag() == -1) {
        aqueue.Push(&n->Node(i));
      }
    }
  }
  uint64_t rv = ((uint64_t)graphNode.GetObject()->GetType().z) << 56;
  rv |= ((uint64_t)v) << 48;
  int64_t h = 0;
  for (size_t i = counts.Count(); i != 0; i--) {
    h = (h << 4) | counts[i - 1];
  }
  rv |= (h & 0x0000ffffffffffffULL);
  return rv;
}

uint64_t CalculateNodeHash2(TEGraphNode<uint64_t,TSAtom*>& graphNode,
  const olxdict<TSAtom*, TEGraphNode<uint64_t,TSAtom*>*, TPointerComparator>& map)
{
  TSAtom &a = *graphNode.GetObject();
  TArrayList<uint64_t> ids(a.NodeCount());
  for (size_t i=0; i < a.NodeCount(); i++) {
    ids[i] = map.Get(&a.Node(i))->GetData();
  }
  BubbleSorter::Sort(ids, TPrimitiveComparator());
  uint64_t id=0;
  for (size_t i=0; i < ids.Count(); i++) {
    if ((id & 1) == 1) {
      id |= (ids[i] << 16);
    }
    else {
      id ^= (ids[i] >> 16);
    }
  }
  uint64_t rv=(graphNode.GetData()&0xffff000000000000ULL);
  rv |= (id&0x0000ffffffffffffULL);
  return rv;
}

void AssignHashes(TEGraphNode<uint64_t,TSAtom*>& graphNode) {
  graphNode.SetData(CalculateNodeHash(graphNode));
  for (size_t i=0; i < graphNode.Count(); i++) {
    AssignHashes(graphNode.Item(i));
  }
  //graphNode.GetObject()->CAtom().SetLabel(graphNode.GetData(), false);
}
void CalculateHashes2(TEGraphNode<uint64_t,TSAtom*>& graphNode,
  const olxdict<TSAtom*, TEGraphNode<uint64_t,TSAtom*>*, TPointerComparator>& map,
  olxdict<TEGraphNode<uint64_t,TSAtom*>*, uint64_t, TPointerComparator>& vs)
{
  vs(&graphNode, CalculateNodeHash2(graphNode, map));
  for (size_t i=0; i < graphNode.Count(); i++) {
    CalculateHashes2(graphNode.Item(i), map, vs);
  }
  //graphNode.GetObject()->CAtom().SetLabel(graphNode.GetData(), false);
}
void AssignHashes2(TEGraphNode<uint64_t,TSAtom*>& graphNode,
  const olxdict<TEGraphNode<uint64_t,TSAtom*>*, uint64_t, TPointerComparator>& vs)
{
  graphNode.SetData(vs.Get(&graphNode));
  for (size_t i=0; i < graphNode.Count(); i++) {
    AssignHashes2(graphNode.Item(i), vs);
  }
  //graphNode.GetObject()->CAtom().SetLabel(graphNode.GetData(), false);
}

void BuildGraph(TEGraphNode<uint64_t,TSAtom*>& graphNode)  {
  typedef TEGraphNode<uint64_t,TSAtom*> node_t;
  olxdict<TSAtom*, node_t*, TPointerComparator> map;
  TNetwork& net = graphNode.GetObject()->GetNetwork();
  net.GetNodes().ForEach(ACollectionItem::TagSetter(-1));
  TQueue<TSAtom*> aqueue;
  aqueue.Push(graphNode.GetObject());
  aqueue.Push(0);
  index_t v = 0;
  while (!aqueue.IsEmpty()) {
    TSAtom* n = aqueue.Pop();
    if (n == 0) {
      if (aqueue.IsEmpty()) {
        break;
      }
      v++;
      aqueue.Push(0);
      continue;
    }
    if (n->GetTag() != -1) {
      continue;
    }
    n->SetTag(v);
    for (size_t i = 0; i < n->NodeCount(); i++) {
      if (n->Node(i).GetTag() == -1) {
        aqueue.Push(&n->Node(i));
      }
    }
  }
  TQueue<node_t*> queue;
  queue.Push(&graphNode);
  map.Add(graphNode.GetObject(), &graphNode);
  while (!queue.IsEmpty()) {
    node_t *n = queue.Pop();
    for (size_t i=0; i < n->GetObject()->NodeCount(); i++) {
      TSAtom &sa = n->GetObject()->Node(i);
      if (sa.GetTag() > n->GetObject()->GetTag()) {
        node_t *nd =
          queue.Push(&n->NewNode((sa.NodeCount()<<16)|sa.GetType().z, &sa));
        map.Add(&sa, nd);
      }
    }
  }
  AssignHashes(graphNode);
  olxdict<node_t *, uint64_t, TPointerComparator> h2s;
  CalculateHashes2(graphNode, map, h2s);
  AssignHashes2(graphNode, h2s);
}

struct GraphAnalyser {
  TEGraphNode<uint64_t, TSAtom*>& RootA, & RootB;
  int CallsCount;
  bool Invert, CalcRMSForH;
  vec3d bCent;
  smatdd bestMatrix;
  double minRms, permutations;
  double (*weight_calculator)(const TSAtom&);
  size_t atomsToMatch;
  time_t start_time;
  struct TagSetter {
    size_t calls;
    TagSetter() : calls(0) {}
    bool OnItem(const TEGraphNode<uint64_t, TSAtom*>& v) {
      calls++;
      v.GetObject()->SetTag(1);
      return true;
    }
  };
  GraphAnalyser(TEGraphNode<uint64_t, TSAtom*>& rootA,
    TEGraphNode<uint64_t, TSAtom*>& rootB,
    double (*_weight_calculator)(const TSAtom&))
    : RootA(rootA), RootB(rootB), CallsCount(0), Invert(false),
    weight_calculator(_weight_calculator)
  {
    minRms = -1;
    CalcRMSForH = true;
  }

  void OnStart(double perms) {
    permutations = perms;
    start_time = TETime::msNow();
  }

  double CalcRMS() {
    TTypeList< olx_pair_t<TSAtom*, TSAtom*> > matchedAtoms;
    TagSetter tag_s;
    RootA.Traverser.Traverse(RootA, tag_s);
    matchedAtoms.SetCapacity(tag_s.calls);
    RootB.Traverser.Traverse(RootB, tag_s);
    ResultCollector(RootA, RootB, matchedAtoms);
    if (!CalcRMSForH) {
      for (size_t i = 0; i < matchedAtoms.Count(); i++)
        if (matchedAtoms[i].a->GetType() == iHydrogenZ)
          matchedAtoms.NullItem(i);
    }
    matchedAtoms.Pack();
    CallsCount++;
    align::out ao = TNetwork::GetAlignmentInfo(matchedAtoms, Invert, weight_calculator);
    if (minRms < 0 || ao.rmsd[0] < minRms) {
      QuaternionToMatrix(ao.quaternions[0], bestMatrix.r);
      bestMatrix.r.Transpose();
      bestMatrix.t = ao.center_a;
      minRms = ao.rmsd[0];
      bCent = ao.center_b;
    }
    return ao.rmsd[0];
  }
  double CalcRMSFull() {
    TTypeList< olx_pair_t<TSAtom*, TSAtom*> > matchedAtoms;
    TagSetter tag_s;
    RootA.Traverser.Traverse(RootA, tag_s);
    matchedAtoms.SetCapacity(tag_s.calls);
    RootB.Traverser.Traverse(RootB, tag_s);
    ResultCollector(RootA, RootB, matchedAtoms);
    align::out ao = TNetwork::GetAlignmentInfo(matchedAtoms, Invert, weight_calculator);
    QuaternionToMatrix(ao.quaternions[0], bestMatrix.r);
    bestMatrix.r.Transpose();
    bestMatrix.t = ao.center_a;
    return ao.rmsd[0];
  }
  double CalcDsq(const TEGraphNode<uint64_t, TSAtom*>& src,
    const TEGraphNode<uint64_t, TSAtom*>& dest)
  {
    const TAsymmUnit& au_a = *src.GetObject()->CAtom().GetParent();
    const TAsymmUnit& au_b = *dest.GetObject()->CAtom().GetParent();
    vec3d v = dest.GetObject()->ccrd();
    if (Invert) {
      v *= -1;
    }
    v = bestMatrix * (au_b.CellToCartesian(v) - bCent);
    return v.QDistanceTo(au_a.Orthogonalise(src.GetObject()->ccrd()));
  }
  double CalcRMS(const TEGraphNode<uint64_t, TSAtom*>& src,
    const TEGraphNode<uint64_t, TSAtom*>& dest)
  {
    if ((TETime::msNow() - start_time) >= 60000) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("the procedure was terminated. Number of permutations: ") <<
        permutations << ", number of calls: " << CallsCount
      );
    }
    if (CalcRMSForH) {
      return CalcRMS();
    }
    size_t h_cnt = 0;
    for (size_t i = 0; i < src.Count(); i++)
      if (src[i].GetObject()->GetType() == iHydrogenZ) {
        h_cnt++;
      }
    if (h_cnt < 2) {
      return CalcRMS();
    }
    if (bestMatrix.r.Trace() == 0) {
      CalcRMS();
    }
    //if (++CallsCount > 1e5)
    //  throw TFunctionFailedException(__OlxSourceInfo, "too many iterations");
    const TAsymmUnit& au_a = *src[0].GetObject()->CAtom().GetParent();
    const TAsymmUnit& au_b = *dest[0].GetObject()->CAtom().GetParent();
    double rsum = 0;
    for (size_t i = 0; i < src.Count(); i++) {
      vec3d v = dest[i].GetObject()->ccrd();
      if (Invert) {
        v *= -1;
      }
      v = bestMatrix * (au_b.CellToCartesian(v) - bCent);
      rsum += v.QDistanceTo(au_a.Orthogonalise(src[i].GetObject()->ccrd()));
    }
    return rsum + minRms;
  }
  void OnFinish() {
    minRms = CalcRMS();
    size_t cnt = 0;
    double mrms = 1e6;
    while (GroupValidator(RootA, RootB)) {
      double rms = CalcRMSFull();
      if (rms >= mrms) {
        break;
      }
      mrms = rms;
      if (++cnt > 100) {
        TBasicApp::NewLogEntry() << "Matching does not converge, breaking";
      }
    }
    if (mrms != 1e6) {
      minRms = mrms;
    }
  }
  bool GroupValidator(const TEGraphNode<uint64_t, TSAtom*>& n1,
    TEGraphNode<uint64_t, TSAtom*>& n2)
  {
    bool rv = false;
    TSizeList used;
    for (size_t i = 0; i < n1.Count(); i++) {
      if (n1[i].GetGroupIndex() == InvalidIndex ||
        used.IndexOf(n1[i].GetGroupIndex()) != InvalidIndex)
      {
        continue;
      }
      TSizeList pos;
      for (size_t j = 0; j < n2.Count(); j++) {
        if (n1[i].GetGroupIndex() == n2[j].GetGroupIndex())
          pos.Add(j);
      }
      if (pos.Count() < 2) {
        continue;
      }
      if (Validator(n1, n2, pos)) {
        rv = true;
      }
#ifdef _DEBUG
      //TBasicApp::NewLogEntry() << n1.GetObject()->GetLabel() << '_'
      //  << pos.Count();
#endif
      used.Add(n1[i].GetGroupIndex());
    }
    for (size_t i = 0; i < n1.Count(); i++) {
      if (GroupValidator(n1[i], n2[i])) {
        rv = true;
      }
    }
    return rv;
  }
  // since the H-atoms are given a smaller weights...
  bool Validator(const TEGraphNode<uint64_t, TSAtom*>& n1,
    TEGraphNode<uint64_t, TSAtom*>& n2, const TSizeList& pos)
  {
    if (pos.Count() < 2) {
      return false;
    }
    const TAsymmUnit& au = *n2[0].GetObject()->CAtom().GetParent();
    TSizeList permutation, null_permutation(pos.Count());
    const size_t perm_cnt = olx_factorial_t<size_t, size_t>(pos.Count());
    sorted::PrimitiveAssociation<double, size_t> hits;
    TArrayList<vec3d> crds(pos.Count());
    for (size_t i = 0; i < pos.Count(); i++) {
      vec3d v = n2[pos[i]].GetObject()->ccrd();
      if (Invert)  v *= -1;
      crds[i] = bestMatrix * (au.CellToCartesian(v) - bCent);
      null_permutation[i] = i;
    }
    for (size_t i = 0; i < perm_cnt; i++) {
      permutation = null_permutation;
      if (i != 0)  GeneratePermutation(permutation, i);
      double sqd = 0;
      for (size_t j = 0; j < pos.Count(); j++)
        sqd += crds[permutation[j]].QDistanceTo(n1[pos[j]].GetObject()->crd());
      hits.Add(sqd, i);
    }
    bool rv = false;
    permutation = null_permutation;
    if (hits.GetValue(0) != 0) {
      GeneratePermutation(permutation, hits.GetValue(0));
      rv = true;
    }
    TPtrList<TEGraphNode<uint64_t, TSAtom*> > nodes(n2.GetNodes());
    for (size_t i = 0; i < pos.Count(); i++)
      n2.GetNodes()[pos[permutation[i]]] = nodes[pos[i]];
    return rv;
  }
};

//..............................................................................
size_t TNetwork_NodeCounter(const TSAtom& a) {
  size_t nc = 0;
  for (size_t i = 0; i < a.NodeCount(); i++) {
    if (!a.Node(i).IsAvailable() || a.Node(i).GetType() == iHydrogenZ) {
      continue;
    }
    nc++;
  }
  return nc;
}
bool TNetwork::DoMatch(TNetwork& net,
  TTypeList<olx_pair_t<size_t, size_t> >& res,
  bool Invert, double (*weight_calculator)(const TSAtom&))
{
  if (NodeCount() != net.NodeCount()) {
    return false;
  }
  TStopWatch st(__FUNC__);
  TSAtom* thisSa = 0;
  size_t maxbc = 0;
  double maxMw = 0;
  size_t HCount = 0;
  for (size_t i = 0; i < NodeCount(); i++) {
    Node(i).SetTag(0);
    const size_t bc = TNetwork_NodeCounter(Node(i));
    if (bc > maxbc) {
      thisSa = &Node(i);
      maxbc = bc;
      maxMw = Node(i).GetType().GetMr();
    }
    else if (bc == maxbc) {
      if (Node(i).GetType().GetMr() > maxMw) {
        thisSa = &Node(i);
        maxMw = thisSa->GetType().GetMr();
      }
    }
    if (Node(i).GetType() == iHydrogenZ) {
      HCount++;
    }
  }

  if (thisSa == 0) {
    return false;
  }
  TEGraph<uint64_t, TSAtom*> thisGraph(thisSa->GetType().z, thisSa),
    * minGraph = 0;
  BuildGraph(thisGraph.GetRoot());
  double minRMSD = 1e6;
  for (size_t i = 0; i < net.NodeCount(); i++) {
    TSAtom& thatSa = net.Node(i);
    if (maxbc != TNetwork_NodeCounter(thatSa) ||
      thisSa->GetType() != thatSa.GetType())
    {
      continue;
    }
    TEGraph<uint64_t, TSAtom*>* thatGraph =
      new TEGraph<uint64_t, TSAtom*>(thatSa.GetType().z, &thatSa);
    BuildGraph(thatGraph->GetRoot());
    if (thisGraph.GetRoot().DoMatch(thatGraph->GetRoot())) {  // match
      GraphAnalyser ga(thisGraph.GetRoot(), thatGraph->GetRoot(),
        weight_calculator);
      ga.Invert = Invert;
      ga.atomsToMatch = NodeCount();
      ga.CalcRMSForH = ((NodeCount() - HCount) < 4);
      try {
        thisGraph.GetRoot().FullMatchEx(thatGraph->GetRoot(), ga);
      }
      catch (const TExceptionBase& e) {
        TBasicApp::NewLogEntry() << e.GetException()->GetError();
        delete thatGraph;
        throw TFunctionFailedException(__OlxSourceInfo, e);
      }
      if (ga.minRms < minRMSD) {
        if (minGraph != 0) {
          delete minGraph;
        }
        minGraph = thatGraph;
        minRMSD = ga.minRms;
      }
      else {
        delete thatGraph;
      }
    }
    else {
      delete thatGraph;
    }
  }
  if (minGraph == 0) {
    return false;
  }
  GetNodes().ForEach(ACollectionItem::TagSetter(1));
  net.GetNodes().ForEach(ACollectionItem::TagSetter(1));
  ResultCollector(thisGraph.GetRoot(), minGraph->GetRoot(), res);
  delete minGraph;
  return true;
}
//..............................................................................
bool TNetwork::IsSubgraphOf(TNetwork& net,
  TTypeList<olx_pair_t<size_t, size_t> >& res,
  const TSizeList& rootsToSkip)
{
  if (NodeCount() > net.NodeCount()) {
    return false;
  }
  TSAtom* thisSa = 0;
  size_t maxbc = 0;
  for (size_t i = 0; i < NodeCount(); i++) {
    Node(i).SetTag(0);
    if (Node(i).NodeCount() > maxbc) {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
    }
  }
  TEGraph<uint64_t, TSAtom*> thisGraph(thisSa->GetType().z, thisSa);
  BuildGraph(thisGraph.GetRoot());
  TIntList GraphId;
  for (size_t i = 0; i < net.NodeCount(); i++) {
    TSAtom* thatSa = &net.Node(i);
    if (thisSa->NodeCount() > thatSa->NodeCount()) {
      continue;
    }
    if (thisSa->GetType() != thatSa->GetType()) {
      continue;
    }
    if (rootsToSkip.IndexOf(i) != InvalidIndex) {
      continue;
    }
    for (size_t j = 0; j < net.NodeCount(); j++) {
      net.Node(j).SetTag(0);
    }
    TEGraph<uint64_t, TSAtom*> thatGraph(thatSa->GetType().z, thatSa);
    BuildGraph(thatGraph.GetRoot());
    //continue;
    if (thisGraph.GetRoot().IsSubgraphOf(thatGraph.GetRoot())) {
      ResultCollector(thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
int proximity_comparator(const TSAtom &a, const TSAtom &b) {
  return olx_cmp(a.crd().Length(), b.crd().Length());
}

bool proximity_check(const TSAtom& ref, const TSAtom& a,
  bool check_type, bool check_connectivity, double delta)
{
  if (check_type && a.GetType() != ref.GetType()) {
    return false;
  }
  if (check_connectivity) {
    if (check_type) {
      olx_pdict<short, size_t> rc, ac;
      for (size_t i = 0; i < a.NodeCount(); i++) {
        if (a.Node(i).IsAvailable() && a.Node(i).GetType() != iHydrogenZ) {
          ac.Add(a.Node(i).GetType().z)++;
        }
      }
      for (size_t i = 0; i < ref.NodeCount(); i++) {
        if (ref.Node(i).IsAvailable() && ref.Node(i).GetType() != iHydrogenZ) {
          rc.Add(ref.Node(i).GetType().z)++;
        }
      }
      if (ac.Count() != rc.Count()) {
        return false;
      }
      for (size_t i = 0; i < ac.Count(); i++) {
        if (ac.GetKey(i) != rc.GetKey(i) || ac.GetValue(i) != rc.GetValue(i)) {
          return false;
        }
      }
      double d = ref.crd().DistanceTo(a.crd());
      return d < delta;
    }
    else {
      return TNetwork_NodeCounter(ref) == TNetwork_NodeCounter(a)
        && ref.crd().DistanceTo(a.crd()) < delta;
    }
  }
  return true;
}

bool proximity_check_up(size_t ii, const TSAtom &a, double l,
  const TSAtomPList &lb,
  TTypeList< olx_pair_t<size_t, size_t> >& res,
  bool check_atom_types, bool check_connectivity, double delta)
{
  while (ii < lb.Count()) {
    double l1 = lb[ii]->crd().Length();
    if (olx_abs(l1 - l) > delta) {
      break;
    }
    if (lb[ii]->IsProcessed()) {
      ii++;
      continue;
    }
    if (proximity_check(a, *lb[ii], check_atom_types, check_connectivity, delta)) {
      lb[ii]->SetProcessed(true);
      res.AddNew(a.GetTag(), lb[ii]->GetTag());
      return true;
    }
    ii++;
  }
  return false;
}

bool proximity_check_dn(size_t ii, const TSAtom& a, double l,
  const TSAtomPList& lb,
  TTypeList< olx_pair_t<size_t, size_t> >& res,
  bool check_atom_types, bool check_connectivity, double delta)
{
  while (ii != InvalidIndex) {
    double l1 = lb[ii]->crd().Length();
    if (olx_abs(l1 - l) > delta) {
      break;
    }
    if (lb[ii]->IsProcessed()) {
      ii--;
      continue;
    }
    if (proximity_check(a, *lb[ii], check_atom_types, check_connectivity, delta)) {
      lb[ii]->SetProcessed(true);
      res.AddNew(a.GetTag(), lb[ii]->GetTag());
      return true;
    }
    ii--;
  }
  return false;
}

bool TNetwork::ProximityMatch(const TNetwork& net,
  TTypeList< olx_pair_t<size_t, size_t> >& res,
  bool check_atom_types, bool check_connectivity, double delta)
{
  TSAtomPList la = GetNodes(), lb = net.GetNodes();
  la.ForEach(ACollectionItem::IndexTagSetter());
  lb.ForEach(ACollectionItem::IndexTagSetter());

  la.Pack(olx_alg::olx_not(FunctionAccessorAnalyser::Make(
    FunctionAccessor::MakeConst(&TSAtom::IsAvailable))));
  lb.Pack(olx_alg::olx_not(FunctionAccessorAnalyser::Make(
    FunctionAccessor::MakeConst(&TSAtom::IsAvailable))));

  if (lb.Count() < la.Count()) {
    return false;
  }
  FunctionComparator::ComparatorSF_<TSAtom> cmp = FunctionComparator::Make(&proximity_comparator);
  QuickSorter::Sort(lb, cmp);
  
  lb.ForEach(TSAtom::FlagSetter(satom_Processed, false));

  double lb_f = lb[0]->crd().Length(),
    lb_l = lb.GetLast()->crd().Length();
  typedef olx_pair_t<size_t, size_t> pair_t;
  for (size_t i = 0; i < la.Count(); i++) {
    const TSAtom& a = *la[i];
    double l = a.crd().Length();
    bool found = false;
    if (l < lb_f) {
      if (olx_abs(l - lb_f) > delta) {
        return false;
      }
      if (proximity_check_up(0, a, l, lb, res, check_atom_types, check_connectivity, delta)) {
        continue;
      }
    }
    else if (l > lb_l) {
      if (olx_abs(l - lb_l) > delta) {
        return false;
      }
      if (proximity_check_dn(lb.Count() - 1, a, l, lb, res, check_atom_types, check_connectivity, delta)) {
        continue;
      }
    }
    else {
      size_t ii = sorted::FindInsertIndex(lb, cmp, a);
      if (proximity_check_up(ii-1, a, l, lb, res, check_atom_types, check_connectivity, delta)) {
        continue;
      }
      if (proximity_check_dn(ii, a, l, lb, res, check_atom_types, check_connectivity, delta)) {
        continue;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, size_t node, TSAtomPList& ring,
  const ElementPList& ringContent)
{
  sa.SetTag(1);
  return TryRing(sa.Node(node), ring, ringContent, 2);
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, TSAtomPList& ring,
  const ElementPList& ringContent, size_t level)
{
  if (ringContent[level - 1] != 0 && (sa.GetType() != *ringContent[level - 1])) {
    return false;
  }
  sa.SetTag(level);
  for (size_t i = 0; i < sa.NodeCount(); i++) {
    TSAtom& a = sa.Node(i);
    if (a.IsDeleted()) {
      continue;
    }
    if (level >= ringContent.Count() && a.GetTag() == 1) {
      return true;
    }
    if (a.GetTag() != 0 && a.GetTag() < (index_t)level) {
      continue;
    }
    if (level < ringContent.Count() && TryRing(a, ring, ringContent, level + 1)) {
      ring.Add(a);
      return true;
    }
  }
  sa.SetTag(0); // unroll the tags
  return false;
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, size_t node, TSAtomPList& ring)  {
  sa.SetTag(1);
  return TryRing(sa.Node(node), ring, 2);
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, TSAtomPList& ring, size_t level) {
  if (level > 12) { // safety precaution
    return false;
  }
  sa.SetTag(level);
  for (size_t i = 0; i < sa.NodeCount(); i++) {
    TSAtom& a = sa.Node(i);
    if (a.IsDeleted() || a.GetType() == iQPeakZ) {
      continue;
    }
    if (a.GetTag() == 1 && level != 2) {
      return true;
    }
    if (a.GetTag() == 0) {
      if (TryRing(a, ring, level + 1)) {
        ring.Add(a);
        return true;
      }
      else {
        continue;
      }
    }
    if ((a.GetTag() + 1) == (index_t)level) { // previous?
      continue;
    }
    else {
      break;
    }
  }
  sa.SetTag(0); // unroll the tags
  return false;
}
//..............................................................................
void TNetwork::UnifyRings(TTypeList<TSAtomPList>& rings) {
  for (size_t i = 0; i < rings.Count(); i++) {
    QuickSorter::SortSF(rings[i], TNetwork_SortRingAtoms);
  }
  // leave unique rings only
  for (size_t i = 0; i < rings.Count(); i++) {
    if (rings.IsNull(i)) {
      continue;
    }
    for (size_t j = i + 1; j < rings.Count(); j++) {
      if (rings.IsNull(j) || rings[i].Count() != rings[j].Count()) {
        continue;
      }
      bool found = true;
      for (size_t k = 0; k < rings[i].Count(); k++) {
        if (rings[i][k]->GetTag() != rings[j][k]->GetTag()) {
          found = false;
          break;
        }
      }
      if (found) {
        rings.NullItem(j);
      }
    }
  }
}
//..............................................................................
void TNetwork::FindRings(const ElementPList& ringContent,
  TTypeList<TSAtomPList>& res)
{
  if (ringContent.IsEmpty()) {
    return;
  }
  TSAtomPList all;
  all.SetCapacity(NodeCount());
  for (size_t i = 0; i < NodeCount(); i++) {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if (sa.IsDeleted()) {
      continue;
    }
    if (sa.NodeCount() > 1) { // a ring node must have at least two bonds!
      all.Add(sa);
    }
  }
  /* we have to keep the order of the ring atoms, so need an extra 'rings'
  array
  */
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for (size_t i = 0; i < all.Count(); i++) {
    if (all[i]->GetType() != *ringContent[0]) {
      continue;
    }
    ring.Clear();
    Nodes.ForEach(ACollectionItem::TagSetter(0));
    ring.Add(all[i]);
    if (TryRing(*all[i], ring, ringContent)) {
      res.AddCopy(ring);
      rings.AddCopy(ring);
    }
  }
  Nodes.ForEach(ACollectionItem::IndexTagSetter());
  UnifyRings(rings);
  for (size_t i = 0; i < rings.Count(); i++) {
    if (rings.IsNull(i)) {
      res.NullItem(resCount + i);
    }
  }
  res.Pack();
}
//..............................................................................
//struct TreeNode {
//  TSAtom& head;
//  TSAtomPList<TSAtom> branches;
//  TreeNode(TSAtom& _head) : head(_head)  {}
//};
//bool tryRing(TSAtom& src, TSAtom& left, TSAtom& right)  {
//  TTypeList<TreeNode> nodes;
//  for( size_t i=0; i < left.NodeCount(); i++ )  {
//    if( left.Node(i).GetTag() == -1 )
//
//  }
//  for( size_t i=0; i < left.NodeCount(); i++ )  {
//    if( left.Node(i).GetTag() >= src.GetTag() )  {
//    }
//  }
//}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, TTypeList<TSAtomPList>& res) {
  if (ringAtom.NodeCount() < 2 || &ringAtom.GetNetwork() != this) {
    return;
  }
  TSAtomPList all;
  all.SetCapacity(NodeCount());
  for (size_t i = 0; i < NodeCount(); i++) {
    TSAtom& a = Node(i);
    a.SetTag(0);
    if (a.IsDeleted() || a.NodeCount() < 2 || a.GetType() == iQPeakZ) {
      continue;
    }
    all.Add(a);
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for (size_t i = 0; i < ringAtom.NodeCount(); i++) {
    TSAtom& a = ringAtom.Node(i);
    if (a.IsDeleted() || a.NodeCount() < 2 || a.GetType() == iQPeakZ) {
      continue;
    }
    ring.Clear();
    Nodes.ForEach(ACollectionItem::TagSetter(0));
    if (TryRing(ringAtom, i, ring)) {
      ring.Add(a);  // the ring is in reverse order
      ring.Add(ringAtom);
      res.AddCopy(ring);
      rings.AddCopy(ring);
    }
  }
  Nodes.ForEach(ACollectionItem::IndexTagSetter());
  UnifyRings(rings);
  for (size_t i = 0; i < rings.Count(); i++) {
    if (rings.IsNull(i)) {
      res.NullItem(resCount + i);
    }
  }
  for (size_t i = 0; i < res.Count(); i++) {
    if (!res.IsNull(i) && !IsRingPrimitive(res[i])) {
      res.NullItem(i);
    }
  }
  res.Pack();
}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, const ElementPList& ringContent,
  TTypeList<TSAtomPList>& res)
{
  if (ringContent.Count() < 3 || ringAtom.NodeCount() < 2 ||
    &ringAtom.GetNetwork() != this)
  {
    return;
  }
  if (ringContent[0] != NULL && ringAtom.GetType() != *ringContent[0]) {
    return;
  }
  TSAtomPList all;
  all.SetCapacity(NodeCount());
  for (size_t i = 0; i < NodeCount(); i++) {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if (sa.IsDeleted()) {
      continue;
    }
    if (sa.NodeCount() > 1) { // a ring node must have at least two bonds!
      all.Add(sa);
    }
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for (size_t i = 0; i < ringAtom.NodeCount(); i++) {
    TSAtom& a = ringAtom.Node(i);
    if (a.IsDeleted() || a.NodeCount() < 2 ||
      (ringContent[1] != NULL && a.GetType() != *ringContent[1]))
    {
      continue;
    }
    ring.Clear();
    Nodes.ForEach(ACollectionItem::TagSetter(0));
    if (TryRing(ringAtom, i, ring, ringContent)) {
      ring.Add(a);  // the ring is in reverse order
      ring.Add(ringAtom);
      res.AddCopy(ring);
      rings.AddCopy(ring);
    }
  }
  Nodes.ForEach(ACollectionItem::IndexTagSetter());
  UnifyRings(rings);
  for (size_t i = 0; i < rings.Count(); i++) {
    if (rings.IsNull(i)) {
      res.NullItem(resCount + i);
    }
  }
  res.Pack();
}
//..............................................................................
TNetwork::RingInfo& TNetwork::AnalyseRing(const TSAtomPList& ring,
  TNetwork::RingInfo& ri)
{
  double maxmw = 0;
  for (size_t i = 0; i < ring.Count(); i++) {
    if (!ri.HasAfix && ring[i]->CAtom().GetAfix() != 0) {
      ri.HasAfix = true;
    }
    TSAtomPList& al = ri.Substituents.AddNew();
    size_t nhc = 0, // not hydrogen atom count
      rnc = 0;   // of which belong to the ring
    double local_maxmw = 0;
    for (size_t j = 0; j < ring[i]->NodeCount(); j++) {
      TSAtom& ra = ring[i]->Node(j);
      if (ra.IsDeleted()) {
        continue;
      }
      if (ra.GetType().z < 2) {
        continue; // Q, H, D
      }
      const double mw = ra.GetType().GetMr();
      if (ra.crd().DistanceTo(ring[i]->crd()) > 2) {
        continue;  // skip M-E bonds
      }
      if (mw > local_maxmw) {
        local_maxmw = mw;
      }
      if (ring.IndexOf(&ra) != InvalidIndex) {
        rnc++;
      }
      else {
        al.Add(ra);
      }
      nhc++;
    }
    if (nhc != rnc) {
      if (local_maxmw > maxmw) {
        ri.HeaviestSubsIndex = i;
        ri.HeaviestSubsType = &ring[i]->GetType();
        maxmw = local_maxmw;
      }
      ri.Substituted.Add(i);
      if (nhc - rnc > ri.MaxSubsANode) {
        ri.MaxSubsANode = nhc - rnc;
      }
    }
    else if (rnc > 2) {
      ri.Ternary.Add(i);
    }
  }
  // analyse alpha atoms (substituted next to ternary atoms)
  for (size_t i = 0; i < ri.Substituted.Count(); i++) {
    for (size_t j = 0; j < ri.Ternary.Count(); j++) {
      if (ring[ri.Substituted[i]]->IsConnectedTo(*ring[ri.Ternary[j]])) {
        ri.Alpha.Add(ri.Substituted[i]);
        break;
      }
    }
  }
  return ri;
}
//..............................................................................
index_t TNetwork::ShortestDistance(TSAtom &a, TSAtom &b)  {
  if( a.IsConnectedTo(b) )  return 0;
  TSAtomPList &atoms = a.GetNetwork().Nodes;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    atoms[i]->SetProcessed(false);
    atoms[i]->SetTag(0);
  }
  ////
  typedef TLinkedList<TSAtom*> list_t;
  olx_object_ptr<list_t> la_ptr(new list_t), lb_ptr(new list_t);
  la_ptr->Add(&a)->SetTag(1);
  lb_ptr->Add(&b)->SetTag(1);
  while( true )  {
    olx_object_ptr<list_t> laa_ptr(new list_t), lbb_ptr(new list_t);
    list_t::Iterator ia = la_ptr->GetIterator();
    while( ia.HasNext() )  {
      TSAtom *sa = ia.Next();
      if( sa->IsProcessed() || sa->GetTag() < 0 )
        return olx_abs(sa->GetTag())*2-2;
      for( size_t i=0; i < sa->NodeCount(); i++ )  {
        TSAtom *n = &sa->Node(i);
        if( n->GetTag() > 0 )
          return sa->GetTag() + n->GetTag() - 2;
        if( n->GetTag() < 0 )  continue;
        laa_ptr->Add(n)->SetTag(sa->GetTag() + 1);
      }
      // mark the front
      sa->SetTag(-sa->GetTag());
      sa->SetProcessed(true);
    }
    list_t::Iterator ib = lb_ptr->GetIterator();
    while( ib.HasNext() )  {
      TSAtom *sa = ib.Next();
      if( sa->IsProcessed() || sa->GetTag() < 0 )
        return olx_abs(sa->GetTag())*2-2;
      for( size_t i=0; i < sa->NodeCount(); i++ )  {
        TSAtom *n = &sa->Node(i);
        if( n->GetTag() > 0 )
          return n->GetTag() + sa->GetTag() - 2;
        if (n->GetTag() < 0) {
          continue;
        }
        lbb_ptr->Add(n)->SetTag(sa->GetTag() + 1);
      }
      sa->SetTag(-sa->GetTag());
      sa->SetProcessed(true);
    }
    la_ptr = laa_ptr;
    lb_ptr = lbb_ptr;
  }
  throw TFunctionFailedException(__OlxSourceInfo, "Unreachable");
}
//..............................................................................
bool TNetwork::IsRingPrimitive(const TSAtomPList& ring)  {
  if( ring.Count() == 3 )  return true;
  const size_t cnt = ((ring.Count()%2)==0 ? ring.Count()/2 : ring.Count()/2+1);
  for( size_t i=0; i < cnt; i++ )  {
    for( size_t j=2; j < ring.Count()-2; j++ )  {
      size_t ind = (i+j)%ring.Count();
      size_t ds = olx_abs(index_t(ind-i-1));
      size_t rd = olx_min(ds, ring.Count()-ds-2);
      size_t sd = ShortestDistance(*ring[i], *ring[ind]);
      if( sd < rd )
        return false;
    }
  }
  return true;
}
//..............................................................................
bool TNetwork::IsRingRegular(const TSAtomPList& ring)  {
  vec3d cent;
  for( size_t i=0; i < ring.Count(); i++ )
    cent += ring[i]->crd();
  cent /= ring.Count();
  double avAng = 2*M_PI/ring.Count(), avDis = 0;
  for( size_t i=0; i < ring.Count(); i++ )
    avDis += cent.DistanceTo( ring[i]->crd() );
  avDis /= ring.Count();
  for( size_t i=0; i < ring.Count(); i++ )  {
    double d = cent.DistanceTo( ring[i]->crd() );
    if( olx_abs(d-avDis) > 0.2 )  return false;
  }
  vec3d a, b;
  for( size_t i=0; i < ring.Count(); i++ )  {
    a = ring[i]->crd();
    if( (i+1) == ring.Count() )
      b = ring[0]->crd();
    else
      b = ring[i+1]->crd();
    a -= cent;
    b -= cent;
    double ca = a.CAngle(b);
    if( ca < -1 )  ca = -1;
    if( ca > 1 ) ca = 1;
    ca = acos(ca);
    if( olx_abs(ca-avAng) > 5./M_PI )  return false;
  }
  return true;
}
//..............................................................................
void TNetwork::PrepareESDCalc(const TTypeList< olx_pair_t<TSAtom*,TSAtom*> >& atoms,
    bool Inverted,
    TSAtomPList& atoms_out,
    vec3d_alist& crd_out,
    TDoubleList& wght_out,
    double (*weight_calculator)(const TSAtom&))
{
  atoms_out.SetCount(atoms.Count()*2);
  crd_out.SetCount(atoms.Count()*2);
  wght_out.SetCount(atoms.Count()*2);
  if( Inverted )  {
    const TAsymmUnit& au2 = atoms[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
    for(size_t i=0; i < atoms.Count(); i++ )  {
      atoms_out[i] = atoms[i].a;
      atoms_out[atoms.Count()+i] = atoms[i].b;
      wght_out[i] = weight_calculator(*atoms[i].GetA());
      wght_out[atoms.Count()+i] = weight_calculator(*atoms[i].GetB());
      vec3d v = atoms[i].GetB()->ccrd() * -1;
      au2.CellToCartesian(v);
      crd_out[i] = atoms[i].GetA()->crd();
      crd_out[atoms.Count()+i] = v;
    }
  }
  else  {
    for(size_t i=0; i < atoms.Count(); i++ )  {
      atoms_out[i] = atoms[i].a;
      atoms_out[atoms.Count()+i] = atoms[i].b;
      wght_out[i] = weight_calculator(*atoms[i].GetA());
      wght_out[atoms.Count()+i] = weight_calculator(*atoms[i].GetB());
      crd_out[i] = atoms[i].GetA()->crd();
      crd_out[atoms.Count()+i] = atoms[i].GetB()->crd();
    }
  }
}
//..............................................................................
void TNetwork::DoAlignAtoms(const TSAtomPList& atomsToTransform,
  const TNetwork::AlignInfo& ai)
{
  if( atomsToTransform.IsEmpty() )  return;
  mat3d m;
  QuaternionToMatrix(ai.align_out.quaternions[0], m);
  vec3d center = ai.align_out.center_b;
  if (ai.inverted) {
    m *= -1;
    center *= -1;
  }
  DoAlignAtoms(atomsToTransform, center, m.Transpose(), ai.align_out.center_a);
}
//..............................................................................
void TNetwork::DoAlignAtoms(const TSAtomPList& atomsToTransform,
    const vec3d &center,
    const mat3d &m, const vec3d &shift)
{
  if (atomsToTransform.IsEmpty())  return;
  TUnitCell& uc = atomsToTransform[0]->GetNetwork().GetLattice().GetUnitCell();
  const TAsymmUnit& au = uc.GetLattice().GetAsymmUnit();
  for (size_t i=0; i < atomsToTransform.Count(); i++) {
    vec3d v = au.Orthogonalise(atomsToTransform[i]->ccrd());
    atomsToTransform[i]->crd() = m*(v - center) + shift;
    if (atomsToTransform[i]->GetEllipsoid() != NULL) {
      if (atomsToTransform[i]->GetEllipsoid()->GetTag() != 0) {
        TBasicApp::NewLogEntry(logError) << "Ellipsoid has already been rotated for: "
          << atomsToTransform[i]->GetLabel();
        continue;
      }
      uc.GetEllp(atomsToTransform[i]->GetEllipsoid()->GetId());
      atomsToTransform[i]->GetEllipsoid()->SetTag(1);
      atomsToTransform[i]->GetEllipsoid()->Mult(m);
    }
  }
}
//..............................................................................
TNetwork::AlignInfo TNetwork::GetAlignmentRMSD(
  const TTypeList< olx_pair_t<TSAtom*,TSAtom*> >& atoms,
  bool invert,
  double (*weight_calculator)(const TSAtom&),
  bool reset_crd
  )
{
  TArrayList<align::pair> pairs;
  AlignInfo rv;
  rv.align_out = align::FindAlignmentQuaternions(AtomsToPairs(atoms, invert,
     weight_calculator, pairs, reset_crd));
  rv.rmsd = align::CalcRMSD(pairs, rv.align_out);
  rv.inverted = invert;
  return rv;
}
//..............................................................................
TArrayList<align::pair>& TNetwork::AtomsToPairs(
  const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
  bool invert, double (*weight_calculator)(const TSAtom&),
  TArrayList<align::pair>& pairs,
  bool reset_crd
  )
{
  if (atoms.IsEmpty()) return pairs;
  pairs.SetCount(atoms.Count());
  const TAsymmUnit& au1 =
    atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  const TAsymmUnit& au2 =
    atoms[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
  for (size_t i=0; i < atoms.Count(); i++) {
    vec3d v1, v2;
    if (reset_crd) {
      v1 = atoms[i].GetA()->ccrd();
      v2 = atoms[i].GetB()->ccrd();
    }
    else {
      v1 = au1.Fractionalise(atoms[i].GetA()->crd());
      v2 = au2.Fractionalise(atoms[i].GetB()->crd());
    }
    if (invert)  v2 *= -1;
    pairs[i].a.value = au1.CellToCartesian(v1);
    pairs[i].a.weight = weight_calculator(*atoms[i].GetA());
    pairs[i].b.value = au2.CellToCartesian(v2);
    pairs[i].b.weight = weight_calculator(*atoms[i].GetB());
  }
  return pairs;
}
//..............................................................................
align::out TNetwork::GetAlignmentInfo(
  const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
  bool invert, double (*weight_calculator)(const TSAtom&), bool reset_crd)
{
  TArrayList<align::pair> pairs;
  return align::FindAlignmentQuaternions(AtomsToPairs(atoms, invert,
    weight_calculator, pairs, reset_crd));
}
//..............................................................................
bool TNetwork::RingInfo::IsSingleCSubstituted() const  {
  for( size_t i=0; i < Substituents.Count(); i++ )  {
    if( Substituents[i].Count() != 1 )  return false;
    TSAtom& sa = *Substituents[i][0];
    if( sa.GetType() != iCarbonZ )  return false;
    size_t nhc = 0;
    for( size_t j=0; j < sa.NodeCount(); j++ )  {
      TSAtom& ra = sa.Node(j);
      if( ra.GetType().GetMr() < 3 )  continue;  // H,D,Q
      nhc++;
    }
    if( nhc > 1 )  return false;  // only one to ring bond
  }
  return true;
}
//..............................................................................
void TNetwork::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network == NULL ? -1 : Network->GetTag());
  //TDataItem& nodes = item.AddItem("Nodes");
  IndexRange::Builder rb;
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->IsDeleted() )  continue;
    rb << Nodes[i]->GetTag();
    //nodes.AddField("node_id", Nodes[i]->GetTag());
  }
  item.AddField("node_range", rb.GetString(true));
  //TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    rb << Bonds[i]->GetTag();
    //bonds.AddField("bond_id", Bonds[i]->GetTag());
  }
  item.AddField("bond_range", rb.GetString(true));
}
//..............................................................................
void TNetwork::FromDataItem(const TDataItem& item) {
  const int net_id = item.GetFieldByName("net_id").ToInt();
  Network = (net_id == -1 ? 0 : &Lattice->GetFragment(net_id));
  const TDataItem* _nodes = item.FindItem("Nodes");
  ASObjectProvider& objects = Lattice->GetObjects();
  if (_nodes != 0) {
    TStrStrList nodes = _nodes->GetOrderedFieldList();
    Nodes.SetCapacity(nodes.Count());
    for (size_t i = 0; i < nodes.Count(); i++) {
      AddNode(objects.atoms[nodes.GetObject(i).ToInt()]);
    }
    nodes = item.GetItemByName("Bonds").GetOrderedFieldList();
    Bonds.SetCapacity(nodes.Count());
    for (size_t i = 0; i < nodes.Count(); i++) {
      AddBond(objects.bonds[nodes.GetObject(i).ToInt()]);
    }
  }
  else {  // index range then
    IndexRange::RangeItr ai(item.GetFieldByName("node_range"));
    Nodes.SetCapacity(ai.CalcSize());
    while (ai.HasNext()) {
      AddNode(objects.atoms[ai.Next()]);
    }
    IndexRange::RangeItr bi(item.GetFieldByName("bond_range"));
    Bonds.SetCapacity(bi.CalcSize());
    while (bi.HasNext()) {
      AddBond(objects.bonds[bi.Next()]);
    }
  }
}
//..............................................................................
ContentList TNetwork::GetContentList() const {
  ElementDict elms;
  for (size_t i = 0; i < NodeCount(); i++) {
    const TSAtom& a = Node(i);
    if (a.IsDeleted() || a.GetType() == iQPeakZ) {
      continue;
    }
    size_t ind = elms.IndexOf(&a.GetType());
    if (ind == InvalidIndex) {
      elms.Add(&a.GetType(), a.CAtom().GetChemOccu());
    }
    else {
      elms.GetValue(ind) += (a.CAtom().GetChemOccu());
    }
  }
  ContentList rv;
  for (size_t i = 0; i < elms.Count(); i++) {
    rv.AddNew(*elms.GetKey(i), elms.GetValue(i));
  }
  return rv;
}
//..............................................................................
olxstr TNetwork::GetFormula() const {
  const ContentList cl = GetContentList();
  olxstr rv;
  for (size_t i = 0; i < cl.Count(); i++) {
    rv << cl[i].element->symbol;
    if (cl[i].count != 1) {
      rv << cl[i].count;
    }
    if ((i + 1) < cl.Count()) {
      rv << ' ';
    }
  }
  return rv;
}
//..............................................................................
bool TNetwork::HaveSharedMatrix(const TSAtom& sa, const TSAtom& sb) {
  if (sa.GetMatrix().GetId() == sb.GetMatrix().GetId()) {
    return true;
  }
  const TUnitCell &uc = sa.GetNetwork().GetLattice().GetUnitCell();
  for (size_t i = 0; i < sa.CAtom().EquivCount(); i++) {
    uint32_t id = uc.MulMatrixId(sa.CAtom().GetEquiv(i), sa.GetMatrix());
    for (size_t j = 0; j < sb.CAtom().EquivCount(); j++) {
      uint32_t id1 = uc.MulMatrixId(sb.CAtom().GetEquiv(j), sb.GetMatrix());
      if (id == id1) {
        return true;
      }
    }
  }
  return false;
}
//..............................................................................
void TNetwork::AddNode(TSAtom& N) {
  /*
  THIS GOES WRONG ON MAC- it adds first and the setsId
  !!! Nodes.Add(N)->SetIdInNetwork(NodeCount()) - do not use
  */
  N.SetIdInNetwork(NodeCount());
  Nodes.Add(N);
}
//..............................................................................
void TNetwork::AddBond(TSBond& B) {
  B.SetIdInNetwork(BondCount());
  Bonds.Add(B);
}
