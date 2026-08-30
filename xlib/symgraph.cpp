/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "symgraph.h"

TSymmNodeRegistry::TSymmNodeRegistry(const TAsymmUnit& au)
  : au_nodes(olx_reserve(au.AtomCount())),
  unit_cell(au.GetLattice().GetUnitCell())
{
  smatd I = smatd::Identity();
  I.SetRawId(FirstMatrixRawId);
  TPtrList<TSymmNode> sym_nodes(olx_reserve(au.AtomCount()));
  for (size_t i = 0; i < au.AtomCount(); i++) {
    const TCAtom& ca = au.GetAtom(i);
    if (ca.GetType().z < 1 || !ca.IsAvailable()) {
      continue;
    }
    TSymmNode* pn = sym_nodes.Add(
      new TSymmNode(au_nodes.AddNew(ca), I));
    registry.Add(pn->build_id(), pn)->init(unit_cell);
    ca.SetTag(i);
  }
  for (size_t i = 0; i < au_nodes.Count(); i++) {
    const TCAtom& ca = au_nodes[i].atom;
    for (size_t j = 0; j < ca.AttachedSiteCount(); j++) {
      TCAtom::Site& s = ca.GetAttachedSite(j);
      if (s.atom->GetType().z < 1 || !s.atom->IsAvailable()) {
        continue;
      }
      TSymmNode cn(au_nodes[s.atom->GetTag()], s.matrix);
      au_nodes[i].children << find_or_add(*sym_nodes[i], cn);
    }
  }
}
//..............................................................................
TSymmNodeRegistry::~TSymmNodeRegistry() {
  for (size_t i = 0; i < registry.Count(); i++) {
    delete registry.GetValue(i);
  }
}
//..............................................................................
TSymmNode* TSymmNodeRegistry::find_or_add(const TSymmNode& parent,
  const TSymmNode& child, index_t def_tag) const
{
  //olx_pair_t<size_t, bool> ii;
  
  for (size_t i = 0; i < child.aun.atom.EquivCount() + 1; i++) {
    uint32_t cm = i == 0 ? child.matrix
      : unit_cell.MulMatrixId(child.matrix, child.aun.atom.GetEquiv(i - 1).GetId());
    for (size_t j = 0; j < parent.aun.atom.EquivCount() + 1; j++) {
      uint32_t pm = j == 0 ? parent.matrix
        : unit_cell.MulMatrixId(parent.matrix, parent.aun.atom.GetEquiv(j - 1).GetId());
      uint32_t m_id = unit_cell.MulMatrixId(cm, pm);
      uint64_t key = TSymmNode::build_id(child.aun, m_id);
      TSymmNode* sn = registry.Find(key, 0);
      if (sn != 0) {
        return sn;
      }
    }
  }
  TSymmNode* sn = new TSymmNode(child.aun,
    unit_cell.MulMatrixId(child.matrix, parent.matrix));
  registry.Add(sn->build_id(), sn)->init(unit_cell);
  sn->SetTag(def_tag);
  return sn;
}
//..............................................................................
