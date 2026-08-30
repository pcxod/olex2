/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "analysis.h"
#include "unitcell.h"

BeginXlibNamespace()

struct TASymmNode : public ACollectionItem {
  const TCAtom& atom;
  TPtrList<struct TSymmNode> children;

  TASymmNode(const TCAtom& a)
    : atom(a)
  {}
};

struct TSymmNode : public ACollectionItem {
  const TASymmNode& aun;
  uint32_t matrix;
  vec3d crd;

  TSymmNode(const TASymmNode& au_node, uint32_t m_id)
    : aun(au_node), matrix(m_id)
  {}

  TSymmNode(const TASymmNode& au_node, const smatd& m)
    : aun(au_node), matrix(m.GetId())
  {}

  void init(const TUnitCell &uc) {
    crd = aun.atom.GetParent()->Orthogonalise(uc.GetMatrixById(matrix) * aun.atom.ccrd());
  }

  static uint64_t build_id(uint64_t au_id, uint64_t m_id) {
    return (au_id << 32) | m_id;
  }
  static uint64_t build_id(const TASymmNode& a, uint32_t m_id) {
    return build_id(a.atom.GetId(), m_id);
  }
  static uint64_t build_id(const TASymmNode& a, const smatd& m) {
    return build_id(a, m.GetId());
  }
  static uint64_t build_id(const TASymmNode& a) {
    return build_id(a, FirstMatrixRawId);
  }
  static uint64_t build_id(const TCAtom::Site& s) {
    return build_id(s.atom->GetId(), s.matrix.GetId());
  }
  uint64_t build_id() const {
    return build_id(aun, matrix);
  }

  const olxstr& get_label() const {
    return aun.atom.GetLabel();
  }
  const cm_Element& get_type() const { return aun.atom.GetType(); }
};

struct TSymmNodeRegistry {
  TTypeList<TASymmNode> au_nodes;
  mutable olx_pdict<uint64_t, TSymmNode*> registry;
  const TUnitCell& unit_cell;

  TSymmNodeRegistry(const TAsymmUnit& au);

  ~TSymmNodeRegistry();

  TSymmNode* find(uint64_t id) const {
    return registry.Find(id, 0);
  }
  // def_tag - tag for the new node, new node gets its children intialised
  TSymmNode* find_or_add(const TSymmNode& parent, const TSymmNode& child,
    index_t def_tag = 0) const;

  olx_pair_t<TCAtom*, TCAtom::Site> remap(const TSymmNode& parent,
    const TSymmNode& child) const
  {
    if (parent.matrix == FirstMatrixRawId) {
      return olx_pair::make(const_cast<TCAtom*>(&parent.aun.atom),
        TCAtom::Site(const_cast<TCAtom*>(&child.aun.atom),
          unit_cell.GetMatrixById(child.matrix)));
    }
    return olx_pair::make(const_cast<TCAtom*>(&parent.aun.atom),
      TCAtom::Site(const_cast<TCAtom*>(&child.aun.atom),
        unit_cell.MulMatrix(child.matrix, unit_cell.InvMatrixId(parent.matrix))));
  }

  olx_pair_t<TCAtom*, TCAtom::Site> remap(const TSymmNode& parent,
    const TSymmNode& child, uint32_t parent_im) const
  {
    if (parent_im == FirstMatrixRawId) {
      return olx_pair::make(const_cast<TCAtom*>(&parent.aun.atom),
        TCAtom::Site(const_cast<TCAtom*>(&child.aun.atom),
          unit_cell.GetMatrixById(child.matrix)));
    }
    return olx_pair::make(const_cast<TCAtom*>(&parent.aun.atom),
      TCAtom::Site(const_cast<TCAtom*>(&child.aun.atom),
        unit_cell.MulMatrix(child.matrix, parent_im)));
  }

  template <class Functor>
  void ForEach(const Functor& f) {
    for (size_t i = 0; i < registry.Count(); i++) {
      f.OnItem(registry.GetValue(i), i);
    }
  }
};
EndXlibNamespace()
