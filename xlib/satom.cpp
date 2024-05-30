/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "satom.h"
#include "network.h"
#include "lattice.h"
#include "residue.h"
#include "symmparser.h"
#include "unitcell.h"
#include "pers_util.h"
#include "index_range.h"
#include "xapp.h"

TSAtom::TSAtom(TNetwork *N)
  : TBasicNode<TNetwork, TSAtom, TSBond>(N),
    Matrix(0), FCAtom(0), FEllipsoid(0)
{
  SetType(sotAtom);
  Flags = 0;
}
//..............................................................................
TSAtom::TSAtom(TNetwork* N, TCAtom::Site& site)
  : TBasicNode<TNetwork, TSAtom, TSBond>(N)
{
  CAtom(*site.atom);
  Matrix = &site.matrix;
  FCCenter = site.matrix * FCAtom->ccrd();
  FCenter = FCAtom->GetParent()->Orthogonalise(FCCenter);
}
//..............................................................................
TSAtom::TSAtom(TNetwork* N, ExplicitCAtomRef& aref)
  : TBasicNode<TNetwork, TSAtom, TSBond>(N)
{
  CAtom(aref.GetAtom());
  Matrix = aref.GetMatrix();
  if (Matrix != 0) {
    FCCenter = (*Matrix) * FCAtom->ccrd();
  }
  else {
    FCCenter = FCAtom->ccrd();
  }
  FCenter = FCAtom->GetParent()->Orthogonalise(FCCenter);
}
//..............................................................................
void  TSAtom::CAtom(TCAtom& S) {
  FCAtom = &S;
  FCCenter = S.ccrd();
  FEllipsoid = S.GetEllipsoid();
  SetMasked(S.IsMasked());
}
//..............................................................................
int TSAtom::_SortBondsByLengthAsc(const TSBond &b1, const TSBond &b2)  {
  return olx_cmp(b1.QLength(), b2.QLength());
}
//..............................................................................
int TSAtom::_SortBondsByLengthDsc(const TSBond &b1, const TSBond &b2)  {
  return olx_cmp(b2.QLength(), b1.QLength());
}
//..............................................................................
void TSAtom::Assign(const TSAtom& S)  {
  TSObject<TNetwork>::Assign(S);
  FEllipsoid = S.GetEllipsoid();
  FCenter    = S.crd();
  FCCenter   = S.ccrd();
  FCAtom     = &S.CAtom();
  Matrix = S.Matrix;
}
//..............................................................................
olxstr TSAtom::GetGuiLabel() const {
  olxstr rv = FCAtom->GetResiLabel();
  if (Network == 0 || Matrix->IsFirst()) {
    return rv;
  }
  return rv << '.' << TSymmParser::MatrixToSymmCode(
    Network->GetLattice().GetUnitCell().GetSymmSpace(), *Matrix);
}
//..............................................................................
void TSAtom::RemoveNode(TSAtom& node) {
  size_t ind = Nodes.IndexOf(&node);
  if (ind == InvalidIndex) {
    return;
  }
  node.Nodes.Remove(this);
  Nodes.Delete(ind);
}
//..............................................................................
olxstr TSAtom::GetGuiLabelEx() const {
  olxstr rv = FCAtom->GetResiLabel();
  if (Network == 0 || Matrix->IsFirst()) {
    return rv;
  }
  return rv << '(' << TSymmParser::MatrixToSymmEx(*Matrix) << ')';
}
//..............................................................................
void TSAtom::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network->GetTag());
  // need to save it for overlayed images etc
  item.AddField("crd", PersUtil::VecToStr(FCenter));
  item.AddField("flags", Flags);
  if (Network->GetTag() >= 0) {
    IndexRange::Builder rb;
    for (size_t i = 0; i < Nodes.Count(); i++) {
      if (Nodes[i]->IsDeleted()) {
        continue;
      }
      rb << Nodes[i]->GetTag();
    }
    item.AddField("node_range", rb.GetString(true));
    for (size_t i = 0; i < Bonds.Count(); i++) {
      if (Bonds[i]->IsDeleted()) {
        continue;
      }
      rb << Bonds[i]->GetTag();
    }
    item.AddField("bond_range", rb.GetString(true));
  }
  item.AddField("atom_id", FCAtom->GetTag());
#ifdef _DEBUG
  const TAsymmUnit& au = *FCAtom->GetParent();
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).GetTag() == FCAtom->GetTag() &&
      &au.GetAtom(i) != FCAtom)
    {
      throw TFunctionFailedException(__OlxSourceInfo, "assert");
    }
  }
#endif
  item.AddField("matrix_id", Matrix->GetId());
}
//..............................................................................
void TSAtom::FromDataItem(const TDataItem& item, TLattice& parent) {
  const size_t net_id = item.GetFieldByName("net_id").ToInt();
  Network = &parent.GetFragment(net_id);
  if (net_id != InvalidIndex) {
    ASObjectProvider& objects = parent.GetObjects();
    const TDataItem* _nodes = item.FindItem("Nodes");
    if (_nodes != 0) {
      TStrStrList nodes = _nodes->GetOrderedFieldList();
      Nodes.SetCapacity(nodes.Count());
      for (size_t i = 0; i < nodes.Count(); i++) {
        Nodes.Add(objects.atoms[nodes.GetObject(i).ToSizeT()]);
      }
      nodes = item.GetItemByName("Bonds").GetOrderedFieldList();
      Bonds.SetCapacity(nodes.Count());
      for (size_t i = 0; i < nodes.Count(); i++) {
        Bonds.Add(objects.bonds[nodes.GetObject(i).ToSizeT()]);
      }
    }
    else {  // index range then
      IndexRange::RangeItr ai(item.GetFieldByName("node_range"));
      Nodes.SetCapacity(ai.CalcSize());
      while (ai.HasNext()) {
        Nodes.Add(objects.atoms[ai.Next()]);
      }
      IndexRange::RangeItr bi(item.GetFieldByName("bond_range"));
      Bonds.SetCapacity(bi.CalcSize());
      while (bi.HasNext()) {
        Bonds.Add(objects.bonds[bi.Next()]);
      }
    }
  }
  TLattice& latt = Network->GetLattice();
  const size_t ca_id = item.GetFieldByName("atom_id").ToSizeT();
  CAtom(latt.GetAsymmUnit().GetAtom(ca_id));
  TDataItem* matrices = item.FindItem("Matrices");
  if (matrices == 0) {
    Matrix = &latt.GetMatrix(item.GetFieldByName("matrix_id").ToSizeT());
  }
  else {
    const size_t mi = matrices->GetFieldByIndex(0).ToSizeT();
    Matrix = &latt.GetMatrix(mi);
  }
  FCCenter = GetMatrix() * FCCenter;
  PersUtil::VecFromStr(item.GetFieldByName("crd"), FCenter);
  Flags = item.GetFieldByName("flags").ToInt();
  if (CAtom().GetEllipsoid() != 0) {
    SetEllipsoid(&latt.GetUnitCell().GetEllipsoid(
      GetMatrix().GetContainerId(), CAtom().GetId()));
  }
}
//..............................................................................
void TSAtom::_SetMatrix(const smatd &M) {
  Matrix = &M;
}
//..............................................................................
TSAtom::Ref TSAtom::GetRef() const {
  return GetRef(*FCAtom, *Matrix);
}
//..............................................................................
bool TSAtom::operator == (const TSAtom::Ref& id) const {
  return FCAtom->GetParent()->GetId() == id.au_id &&
    FCAtom->GetId() == id.atom_id &&
    IsGenerator(id.matrix_id);
}
//..............................................................................
bool TSAtom::IsGenerator(uint32_t m_id) const {
  if (m_id == GetMatrix().GetId()) {
    return true;
  }
  const TUnitCell &uc = GetNetwork().GetLattice().GetUnitCell();
  for (size_t i=0; i < CAtom().EquivCount(); i++) {
    uint32_t id = uc.MulMatrixId(CAtom().GetEquiv(i), GetMatrix());
    if (id == m_id) {
      return true;
    }
  }
  return false;
}
//..............................................................................
TSAtom::Ref TSAtom::GetRef(const TCAtom &a, const smatd &generator) {
  if (generator.IsFirst()) {
    return Ref(a, generator.GetId());
  }
  uint32_t m_id = generator.GetId();
  const TUnitCell &uc = a.GetParent()->GetLattice().GetUnitCell();
  for (size_t i=0; i < a.EquivCount(); i++) {
    uint32_t n_id = uc.MulMatrixId(a.GetEquiv(i), generator);
    if (smatd::IsFirst(n_id)) {
      return Ref(a, n_id);
    }
    if (n_id < m_id) {
      m_id = n_id;
    }
  }
  return Ref(a, m_id);
}
//..............................................................................
TSAtom::Ref::Ref(const TCAtom& a, uint32_t m_id)
  : au_id(a.GetParent()->GetId()),
  atom_id(a.GetId()),
  matrix_id(m_id)
{}
//..............................................................................
int TSAtom::Ref::Compare(const Ref& r) const {
  int rv = olx_cmp(au_id, r.au_id);
  if (rv == 0) {
    rv = olx_cmp(atom_id, r.atom_id);
    if (rv == 0) {
      rv = olx_cmp(matrix_id, r.matrix_id);
    }
  }
  return rv;
}
//..............................................................................
void TSAtom::Ref::ToDataItem(TDataItem& item, const class TXApp& app, bool use_id) const {
  item.AddField("au_id", au_id)
    .AddField("m_id", matrix_id);
  if (use_id) {
    item.AddField("a_id", atom_id);
  }
  else {
    item.AddField("a_id", app.XFiles()[au_id].GetAsymmUnit().GetAtom(atom_id).GetTag());
  }
}
//..............................................................................
bool TSAtom::Ref::IsValid(const TXApp& app) const {
  const TAsymmUnit& au = app.XFiles()[au_id].GetAsymmUnit();
  index_t tag = au.GetAtom(atom_id).GetTag();
  return tag >= 0 && tag < au.AtomCount();
}
//..............................................................................
void TSAtom::Ref::FromDataItem(const TDataItem& item, const TXApp& app) {
  au_id = item.FindField("au_id", "0").ToSizeT();
  if (au_id >= app.XFiles().Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "au_id");
  }
  const TAsymmUnit& au = app.XFiles()[au_id].GetAsymmUnit();
  atom_id = item.GetFieldByName("a_id").ToSizeT();
  if (atom_id >= au.AtomCount()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "a_id");
  }
  matrix_id = item.GetFieldByName("m_id").ToUInt();
}
//..............................................................................
TCAtom& TSAtom::Ref::GetCAtom(const TXApp& app) const {
  return app.XFiles()[au_id].GetAsymmUnit().GetAtom(atom_id);
}
//..............................................................................
TLattice& TSAtom::Ref::GetLattice(const TXApp& app) const {
  return app.XFiles()[au_id].GetLattice();
}
//..............................................................................
TLattice &TSAtom::GetParent() const { return GetNetwork().GetLattice(); }
//..............................................................................
