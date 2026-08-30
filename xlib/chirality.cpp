/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "analysis.h"
#include "complex_id.h"
#include "symgraph.h"
#include "fixed_id.h"


typedef FixedId<4> BigId;

// note max atom id is uint16_t
struct RSA_BondOrder {
  int get_order(const TCAtom& a, const TCAtom::Site& to) {
    id_t id = get_id(a, to);
    int o = orders.Find(id, -1);
    if (o == -1) {
      o = evaluate_order(a, to);
      orders(id, o);
    }
    else {
      return o;
    }
    return o;
  }
protected:
  typedef olx_object_ptr<complex_id_t> id_t;
  static olx_object_ptr<complex_id_t> get_id(const TCAtom& a, const TCAtom::Site& to) {
    return new complex_id_t(a.GetId(), to.atom->GetId(), to.matrix.GetId());
  }

  olxdict<id_t, int, TComparableComparator> orders;

  int evaluate_order(const TCAtom& a, const TCAtom::Site& to) {
    if (a.GetType() == iHydrogenZ || to.atom->GetType() == iHydrogenZ) {
      return 1;
    }
    // analyse "from" atom
    size_t a_cnt = 0;
    for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
      if (a.GetAttachedAtom(i).GetType().z >= 1 &&
        a.GetAttachedAtom(i).IsAvailable())
      {
        a_cnt++;
      }
    }
    if (a.GetType().z == iCarbonZ && a_cnt == 4) {
      return 1;
    }
    if (a.GetType().z == iNitrogenZ && a_cnt == 3) {
      return 1;
    }
    if (a.GetType().z == iOxygenZ) {
      return a_cnt == 2 ? 1 : 2;
    }
    // analyse "to" atom
    size_t b_cnt = 0;
    for (size_t i = 0; i < to.atom->AttachedSiteCount(); i++) {
      if (to.atom->GetAttachedAtom(i).GetType().z >= 1 &&
        to.atom->GetAttachedAtom(i).IsAvailable())
      {
        b_cnt++;
      }
    }
    if (to.atom->GetType().z == iCarbonZ && b_cnt == 4) {
      return 1;
    }
    if (to.atom->GetType().z == iNitrogenZ && b_cnt == 3) {
      return 1;
    }
    if (to.atom->GetType().z == iOxygenZ) {
      return b_cnt == 2 ? 1 : 2;
    }
    // bond length analysis
    double d = a.GetParent()->Orthogonalise(a.ccrd() - to.matrix * to.atom->ccrd())
      .Length();
    if (a.GetType().z == iCarbonZ || to.atom->GetType().z == iCarbonZ) {
      const cm_Element& other = (a.GetType().z == iCarbonZ ? to.atom->GetType()
        : a.GetType());
      if (other.z == iCarbonZ) { //C-C
        if (d < 1.27) {
          return 3;
        }
        if (d < 1.44) {
          if (a_cnt == 3) { // only the shortest bond can be 2
            typedef olx_pair_t<double, const TCAtom::Site*> pair_t;
            TTypeList<pair_t> bonds;
            for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
              const TCAtom::Site& b = a.GetAttachedSite(i);
              if (b.atom->GetType().z < 2 || !b.atom->IsAvailable()) {
                continue;
              }
              if (b == to) {
                bonds.AddNew(d, &b);
                continue;
              }
              double d1 = a.GetParent()->Orthogonalise(a.ccrd() - b.matrix * b.atom->ccrd())
                .Length();
              bonds.AddNew(d1, &b);
            }
            BubbleSorter::Sort(bonds, ComplexComparator::Make(
              FunctionAccessor::MakeConst(&pair_t::GetA), TPrimitiveComparator()));
            if (to == *bonds[0].b) {
              return 2;
            }
            return 1;
            //return to == *bonds[0].b ? 2 : 1;
          }
          return 2;
        }
      }
      if (other.z == iNitrogenZ) { //C-N
        if (d < 1.27) {
          return 3;
        }
        if (d < 1.405) {
          return 2;
        }
      }
    }
    if (a.GetType().z == iNitrogenZ || to.atom->GetType().z == iNitrogenZ) {
      const cm_Element& other = (a.GetType().z == iNitrogenZ ? to.atom->GetType()
        : a.GetType());
      if (other.z == iNitrogenZ) { //N-N
        if (d < 1.15) {
          return 3;
        }
        if (d < 1.32) {
          return 2;
        }
        return 1;
      }
    }
    return 1;
  }
};
//.............................................................................
struct CIPElem {
  const TSymmNode* node;      // 0 = phantom
  const cm_Element* type;
  const TSymmNode* from;      // arrival node, 0 at root
  int from_order;
  bool terminal;

  CIPElem(const TSymmNode* n, const cm_Element* t, const TSymmNode* f, int fo, bool term)
    : node(n), type(t), from(f), from_order(fo), terminal(term)
  {}

  olxstr strof() const {
    olxstr sa;
    if (node == 0) {
      sa << "{" << type->symbol << "}";
    }
    else {
      sa << node->get_label();
    }
    return sa.RightPadding(5, ' ');
  }
};
//.............................................................................
struct CIPCompare {
  RSA_BondOrder& boa;
  TSymmNodeRegistry& registry;
  vec3d root_crd;
  double MaxCIPRadiusSq; // cut-off limit
  olxstr_buf* bf;
  mutable olxdict<BigId, int, TComparableComparator> order;

  static uint64_t node_id(const TSymmNode* n) {
    return n != 0 ? n->build_id() : 0;
  }

  static olx_pair_t<BigId, bool> make_cache_key(const CIPElem& na, const CIPElem& nb) {
    uint64_t a_from = node_id(na.from), a_node = node_id(na.node);
    uint64_t b_from = node_id(nb.from), b_node = node_id(nb.node);
    BigId fwd(a_from, a_node, b_from, b_node);
    BigId rev(b_from, b_node, a_from, a_node);
    return fwd.Compare(rev) <= 0
      ? olx_pair::make(fwd, true)
      : olx_pair::make(rev, false);
  }

  CIPCompare(RSA_BondOrder& b, TSymmNodeRegistry& registry, olxstr_buf* bf)
    : boa(b), registry(registry), bf(bf)
  {
    MaxCIPRadiusSq = 225;
  }

  // Expands one node one sphere, pushing/popping its own bit only.
  // side = 2 or 4. Returns children in raw (unsorted) order.
  void Expand(const CIPElem& e, int side, TTypeList<CIPElem>& out) const {
    if (e.terminal || e.node == 0) {
      return;
    }
    const TSymmNode& n = *e.node;
    n.SetTag(n.GetTag() | side);
    if (e.from != 0) {
      for (int k = 1; k < e.from_order; k++) {
        out.Add(new CIPElem(0, &e.from->get_type(), (const TSymmNode*)0, 0, true));
      }
    }
    uint32_t im = registry.unit_cell.InvMatrixId(n.matrix);
    for (size_t i = 0; i < n.aun.children.Count(); i++) {
      TSymmNode* child = registry.find_or_add(n, *n.aun.children[i]);
      if (e.from != 0 && child == e.from) {
        continue;   // safe: registry guarantees one canonical TSymmNode* per position
      }
      if ((child->crd - root_crd).QLength() > MaxCIPRadiusSq) {
        continue;
      }
      olx_pair_t<TCAtom*, TCAtom::Site> r = registry.remap(n, *child, im);
      int bo = boa.get_order(*r.a, r.b);
      bool closure = (child->GetTag() & side) != 0;
      out.AddNew(child, &child->get_type(), &n, bo, closure);
      if (!closure) {
        for (int k = 1; k < bo; k++) {
          out.Add(new CIPElem(0, &child->get_type(), (const TSymmNode*)0, 0, true));
        }
      }
    }
  }

  static const cm_Element& PhantomZeroType() {
    return XElementLib::GetByIndex(iQPeakIndex);  // z=0 sentinel — confirm actual entry
  }
  static const cm_Element& PhantomHType() {
    return XElementLib::GetByIndex(iHydrogenIndex);
  }

  // Sibling ranking: expand+rank a single node's own children among
  // themselves, using this SAME comparator recursively. Local, bounded,
  // no cross-side interaction — safe to recurse.
  void SortSiblings(const CIPElem& parent, int side, TTypeList<CIPElem>& children, int level=0) const {
    Expand(parent, side, children);
    if (children.Count() < 2) {
      return;
    }
    BubbleSorter::Sort(children, ComparatorAdapter(*this, side, level));
  }

  // Adapter so BubbleSorter can call CompareOne descending, reusing `side`.
  struct ComparatorAdapter {
    const CIPCompare& self;
    int side, level;
    ComparatorAdapter(const CIPCompare& s, int sd, int level)
      : self(s), side(sd), level(level)
    {}
    int Compare(const CIPElem* a, const CIPElem* b) const {
      return -self.CompareOne(*a, side, *b, side, level);  // descending: highest first
    }
  };
  // NOTE: verify BubbleSorter::SortSF's expected comparator signature/call
  // convention against your codebase (function pointer vs functor vs
  // ComparatorCMF_-style wrapper) — adjust the adapter shape accordingly.

  // The actual fix: true sphere-by-sphere BFS, not per-position recursion.
  // sideA/sideB let this be reused for sibling-vs-sibling (side,side) or
  // top-level branch-vs-branch (2,4) comparisons uniformly.
  olxstr strof(const CIPElem& root, const TTypeList<CIPElem> &ca, size_t sz) const {
    olxstr_buf bf;
    bf << root.strof() << " -> [";
    for (size_t i = 0; i < sz; i++) {
      if (i < ca.Count()) {
        bf << ca[i].strof() << ", ";
      }
      else {
        bf << '{' << (root.terminal ? 'Q' : 'H') << " }  , ";
      }
    }
    olxstr rv = olxstr(bf);
    if (sz > 0) {
      rv.SetLength(rv.Length() - 2);
    }
    return rv << ']';
  }

  int CompareOne(const CIPElem& ea, int sideA, const CIPElem& eb, int sideB, int level) const {
    if (ea.type->z != eb.type->z) {
      return olx_cmp(ea.type->z, eb.type->z);
    }
    typedef olx_pair_t<CIPElem, CIPElem> Pair;
    TQueue<Pair> queue;
    queue.PushLast(Pair(ea, eb));
    while (!queue.IsEmpty()) {
      Pair p = queue.PopFirst();
      const CIPElem& na = p.GetA();
      const CIPElem& nb = p.GetB();

      if (na.node != 0 && nb.node != 0) {
        olx_pair_t<BigId, bool> key = make_cache_key(na, nb);
        int v = order.Find(key.a, 0);
        if (v != 0) {
          return key.b ? v : -v;
        }
      }
      TTypeList<CIPElem> ca, cb;
      SortSiblings(na, sideA, ca, level+1);
      SortSiblings(nb, sideB, cb, level+1);
      size_t sz = olx_max(ca.Count(), cb.Count());
      for (size_t j = 0; j < sz; j++) {
        CIPElem pa = j < ca.Count() ? ca[j]
          : CIPElem(0, na.terminal ? &PhantomZeroType() : &PhantomHType(), 0, 0, true);
        CIPElem pb = j < cb.Count() ? cb[j]
          : CIPElem(0, nb.terminal ? &PhantomZeroType() : &PhantomHType(), 0, 0, true);
        if (pa.type->z != pb.type->z) {
          int rv = olx_cmp(pa.type->z, pb.type->z);
          if (bf != 0) {
            olxstr padding = olxstr::CharStr(' ', level*2);
            (*bf) << "\n" << padding
              << ea.strof() << (rv < 0 ? "< " : "> ") << eb.strof()
              << " (" << na.strof() << (rv < 0 ? "< " : "> ") << nb.strof() << ") "
              << " (" << pa.strof() << (rv < 0 ? "< " : "> ") << pb.strof() << ") ";
            (*bf) << "\n " << padding << strof(na, ca, sz);
            (*bf) << "\n " << padding<< strof(nb, cb, sz);
          }
          const CIPElem* elms[] = {&ea, &eb, &na, &nb, &pa, &pb};
          for (int pass = 0; pass < 6; pass+=2) {
            if (elms[pass]->node != 0 && elms[pass+1]->node != 0) {
              olx_pair_t<BigId, bool> key = make_cache_key(*elms[pass], *elms[pass+1]);
              order.Add(key.a, key.b ? rv : -rv);
            }
          }
          return rv;
        }
        queue.PushLast(Pair(pa, pb));
      }
    }
    return 0;
  }
};
//.............................................................................
struct RSA_Full_EnviSorter {
  RSA_Full_EnviSorter(RSA_BondOrder& boa_, const TCAtom &c, bool debug_)
    : registry(*c.GetParent()),
    cip(boa_, registry, debug_ ? &out : 0), center(0)
  {
    center = cip.registry.find(TSymmNode::build_id(c.GetId(), FirstMatrixRawId));
    if (center == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "__assert__");
    }
    cip.root_crd = center->crd;
  }

  //RSA_Full_EnviSorter(RSA_BondOrder& boa_, const TSAtom& c, bool debug_)
  //  : registry(c.GetNetwork()),
  //  cip(boa_, registry, debug_ ? &out : 0), center(0)
  //{
  //  center = cip.registry.find(
  //    TSymmNode::build_id(c.CAtom().GetId(), c.GetMatrix().GetId()));
  //  if (center == 0) {
  //    throw TFunctionFailedException(__OlxSourceInfo, "__assert__");
  //  }
  //  cip.root_crd = center->crd;
  //}

  int Comparator(const TCAtom::Site& a, const TCAtom::Site& b) const {
    if (center == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "set centere before call!");
    }
    cip.registry.ForEach(ACollectionItem::TagSetter(0));
    center->SetTag(2 | 4);   // closes ring back to center correctly on BOTH sides
    TSymmNode* na = cip.registry.find(TSymmNode::build_id(a));
    TSymmNode* nb = cip.registry.find(TSymmNode::build_id(b));
    if (na == 0 || nb == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "__assert__");
    }
    CIPElem ea(na, &a.atom->GetType(), center, 1, false);
    CIPElem eb(nb, &b.atom->GetType(), center, 1, false);
    cip.order.Clear();
    return cip.CompareOne(ea, 2, eb, 4, 0);
  }
public:
  mutable olxstr_buf out;
protected:
  const TSymmNode* center;
  CIPCompare cip;
  TSymmNodeRegistry registry;
};

/* Human - like original algorithm, the latest bug to fix was to sort the whole
 expanded shell together... Before it was fixed - the Full tree expansion above
 was implemented with Claude assistance. This algorithm provides more comprehensive
 debug output - good for teaching
*/
//.............................................................................
//.............................................................................
//.............................................................................
typedef olx_pair_t<const TCAtom::Site*, const cm_Element*> SiteInfo;
typedef TTypeList<SiteInfo> AtomEnvList;

int RSA_CompareSites(const SiteInfo& a, const SiteInfo& b) {
  int res = olx_cmp(a.GetB()->z, b.GetB()->z);
  if (res == 0) {
    if (a.GetA() != 0) {
      if (b.GetA() == 0) {
        return 1;
      }
    }
    else if (b.GetA() != 0) {
      return -1;
    }
    return 0;
  }
  return res;
}
//.............................................................................
// for debugging output only
int RSA_CompareSubs(const AtomEnvList& a, const AtomEnvList& b) {
  size_t sz = olx_min(a.Count(), b.Count());
  if (sz == 0) {
    if (!a.IsEmpty()) {
      return 1;
    }
    if (b.IsEmpty()) {
      return -1;
    }
    return 0;
  }
  for (size_t i = 0; i < sz; i++) {
    size_t ai = a.Count() - i - 1;
    size_t bi = b.Count() - i - 1;
    int res = RSA_CompareSites(a[ai], b[bi]);
    if (res != 0) {
      return res;
    }
  }
  return olx_cmp(a.Count(), b.Count());
}
//.............................................................................
olxstr strof(const SiteInfo& a) {
  olxstr sa;
  if (a.a == 0) {
    sa << "{" << a.b->symbol << "}";
  }
  else {
    sa << a.a->atom->GetLabel();
  }
  return sa.RightPadding(5, ' ');
}
//.............................................................................
int RSA_GetAtomPriorityX(RSA_BondOrder& boa, AtomEnvList& a, AtomEnvList& b,
  olxstr_buf* bf)
{
  size_t sz = a.Count();
  if (sz == 0) {
    return 0;
  }
  for (size_t i = 0; i < sz; i++) {
    size_t ai = a.Count() - i - 1;
    size_t bi = b.Count() - i - 1;
    int res = olx_cmp(a[ai].GetB()->z, b[ai].GetB()->z);
    if (a[ai].GetA() != 0) {
      if ((a[ai].a->atom->GetTag() & 2) == 0) {
        a[ai].a->atom->SetTag(a[ai].a->atom->GetTag() | 2);
      }
    }
    if (b[bi].GetA() != 0) {
      if ((b[bi].a->atom->GetTag() & 4) == 0) {
        b[bi].a->atom->SetTag(b[bi].a->atom->GetTag() | 4);
      }
    }
    if (res != 0) {
      return res;
    }
  }
  // equal? expand further
  TTypeList<AtomEnvList> al, bl;
  for (size_t i = 0; i < sz; i++) {
    AtomEnvList& aa = al.AddNew(),
      & bb = bl.AddNew();
    if (a[i].GetA() != 0) {
      TCAtom& atomA = *a[i].GetA()->atom;
      for (size_t j = 0; j < atomA.AttachedSiteCount(); j++) {
        TCAtom::Site& s = atomA.GetAttachedSite(j);
        if (s.atom->GetType().z < 1 || !s.atom->IsAvailable()) {
          continue;
        }
        // stop propagation as the site is in use
        if (s.atom->GetTag() != 0) {
          if ((s.atom->GetTag() & 2) == 0) {
            aa.Add(new SiteInfo(0, &s.atom->GetType()));
          }
          else {
            int bo = boa.get_order(atomA, s);
            for (int k = 1; k < bo; k++) {
              aa.Add(new SiteInfo(0, &s.atom->GetType()));
            }
            continue;
          }
        }
        else {
          aa.Add(new SiteInfo(&s, &s.atom->GetType()));
        }
        int bo = boa.get_order(atomA, s);
        for (int k = 1; k < bo; k++) {
          aa.Add(new SiteInfo(0, &s.atom->GetType()));
        }
      }
    }
    if (b[i].GetA() != 0) {
      TCAtom& atomB = *b[i].GetA()->atom;
      for (size_t j = 0; j < atomB.AttachedSiteCount(); j++) {
        TCAtom::Site& s = atomB.GetAttachedSite(j);
        if (s.atom->GetType().z < 1 || !s.atom->IsAvailable()) {
          continue;
        }
        // stop propagation as the site is in use
        if (s.atom->GetTag() != 0) {
          if ((s.atom->GetTag() & 4) == 0) {
            bb.Add(new SiteInfo(0, &s.atom->GetType()));
          }
          else {
            int bo = boa.get_order(atomB, s);
            for (int k = 1; k < bo; k++) {
              bb.Add(new SiteInfo(0, &s.atom->GetType()));
            }
            continue;
          }
        }
        else {
          bb.Add(new SiteInfo(&s, &s.atom->GetType()));
        }
        int bo = boa.get_order(atomB, s);
        for (int k = 1; k < bo; k++) {
          bb.Add(new SiteInfo(0, &s.atom->GetType()));
        }
      }
    }
    // padd the branches
    while (aa.Count() < bb.Count()) {
      aa.Add(new SiteInfo(0, &XElementLib::GetByIndex(iHydrogenIndex)));
    }
    while (bb.Count() < aa.Count()) {
      bb.Add(new SiteInfo(0, &XElementLib::GetByIndex(iHydrogenIndex)));
    }
    BubbleSorter::SortSF(aa, &RSA_CompareSites);
    BubbleSorter::SortSF(bb, &RSA_CompareSites);
  }
  if (bf != 0) {
    TSizeList ali(al.Count(), olx_list_init::index());
    TSizeList bli(bl.Count(), olx_list_init::index());
    BubbleSorter::Sort(al, FunctionComparator::Make(&RSA_CompareSubs),
      SyncSortListener::MakeSingle(ali));
    BubbleSorter::Sort(bl, FunctionComparator::Make(&RSA_CompareSubs),
      SyncSortListener::MakeSingle(bli));

    olxstr_buf out1, out2;
    for (size_t i = 0; i < sz; i++) {
      out1 << strof(a[i]);
      out2 << strof(b[i]);
    }
    out1 << " -> ";
    out2 << " -> ";
    for (size_t i = 0; i < al.Count(); i++) {
      if (!al[i].IsEmpty()) {
        out1 << strof(a[ali[i]]) << '[';
        for (size_t j = 0; j < al[i].Count(); j++) {
          out1 << strof(al[i][j]);
        }
        out1.TrimTail() << "] ";
      }
      if (!bl[i].IsEmpty()) {
        out2 << strof(b[bli[i]]) << '[';
        for (size_t j = 0; j < bl[i].Count(); j++) {
          out2 << strof(bl[i][j]);
        }
        out2.TrimTail() << "] ";
      }
    }
    (*bf) << "\n    " << out1.TrimTail();
    (*bf) << "\n    " << out2.TrimTail();
  }
  for (size_t i = 0; i < al.Count(); i++) {
    for (size_t j = 0; j < al[i].Count(); j++) {
      a.Add(al[i][j]);
    }
    al[i].ReleaseAll();
  }
  for (size_t i = 0; i < bl.Count(); i++) {
    for (size_t j = 0; j < bl[i].Count(); j++) {
      b.Add(bl[i][j]);
    }
    bl[i].ReleaseAll();
  }
  a.DeleteRange(0, sz);
  b.DeleteRange(0, sz);
  QuickSorter::SortSF(a, &RSA_CompareSites);
  QuickSorter::SortSF(b, &RSA_CompareSites);
  return RSA_GetAtomPriorityX(boa, a, b, bf);
}
//.............................................................................
struct RSA_Digraph_EnviSorter {
  RSA_Digraph_EnviSorter(RSA_BondOrder& boa, TCAtom &a, bool debug)
    : boa(boa), center(a),
    debug(debug)
  {}

  int Comparator(const TCAtom::Site& a, const TCAtom::Site& b) const {
    if (center == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "set centere before call!");
    }
    a.atom->GetParent()->GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    center.SetTag(6);
    AtomEnvList ea, eb;
    ea.Add(new SiteInfo(&a, &a.atom->GetType()));
    eb.Add(new SiteInfo(&b, &b.atom->GetType()));
    olxstr_buf da;
    int res = RSA_GetAtomPriorityX(boa, ea, eb, debug ? &da : 0);
    if (res != 0) {
      size_t ref;
      if (a.atom->GetId() < b.atom->GetId()) {
        ref = ((uint64_t)a.atom->GetId()) << 32 | b.atom->GetId();
      }
      else {
        ref = ((uint64_t)b.atom->GetId()) << 32 | a.atom->GetId();
      }
      if (reported.Add(ref)) {
        out << "\n   " << a.atom->GetLabel() << (res < 0 ? " < " : " > ")
          << b.atom->GetLabel();
        out << da;
      }
    }
    return res;
  }
public:
  mutable olxstr_buf out;
protected:
  TCAtom& center;
  bool debug;
  mutable olx_pset<uint64_t> reported;
  RSA_BondOrder& boa;
};
//.............................................................................
//.............................................................................
//.............................................................................
template <class envi_sorter_t>
olx_pair_t<olxstr, olxstr> rsa_analyse(TCAtom& a, bool debug, bool dry_run) {
  if (a.IsDeleted() || a.GetType() < 2) {
    return EmptyString();
  }
  TPtrList<TCAtom::Site> attached;
  for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
    TCAtom& aa = a.GetAttachedAtom(j);
    if (!aa.IsAvailable() || aa.GetType() == iQPeakZ) {
      continue;
    }
    attached.Add(a.GetAttachedSite(j));
  }
  olxstr w, di;
  if (attached.Count() == 4) {
    RSA_BondOrder boa;
    envi_sorter_t es(boa, a, debug);
    BubbleSorter::SortMF(attached, es, &envi_sorter_t::Comparator);
    bool chiral = true;
    for (size_t j = 0; j < attached.Count(); j++) {
      w << attached[j]->atom->GetLabel();
      if ((j + 1) < 4) {
        w << " < ";
      }
      if (j == 0) {
        continue;
      }
      if (es.Comparator(*attached[j - 1], *attached[j]) == 0) {
        chiral = false;
        break;
      }
    }
    if (!chiral) {
      return EmptyString();
    }
    else if (debug) {
      di = olxstr("For ") << a.GetLabel() << olxstr(es.out);
    }
    if (!dry_run) {
      a.ClearChiralFlag();
      vec3d_alist crds(4);
      for (int j = 0; j < 4; j++) {
        crds[j] = a.GetParent()->Orthogonalise(
          attached[j]->matrix * attached[j]->atom->ccrd());
      }
      vec3d cnt = (crds[1] + crds[2] + crds[3]) / 3;
      vec3d n = (crds[1] - crds[2]).XProdVec(crds[3] - crds[2]).Normalise();
      if ((crds[0] - cnt).DotProd(n) < 0) {
        n *= -1;
      }
      vec3d np = (crds[1] - cnt).XProdVec(n);
      if ((crds[3] - cnt).DotProd(np) < 0) { //clockwise
        a.SetChiralR(true);
      }
      else {
        a.SetChiralS(true);
      }
    }
  }
  return olx_pair::make(w, di);
}
//.............................................................................
olx_pair_t<olxstr, olxstr>  xlib::olx_analysis::chirality
::rsa_analyse_digraph(TCAtom& a, bool debug, bool dry_run)
{
  return rsa_analyse<RSA_Digraph_EnviSorter>(a, debug, dry_run);
}
//.............................................................................
olx_pair_t<olxstr, olxstr>  xlib::olx_analysis
::chirality::rsa_analyse_full(TCAtom& a, bool debug, bool dry_run)
{
  return rsa_analyse<RSA_Full_EnviSorter>(a, debug, dry_run);
}
//.............................................................................
