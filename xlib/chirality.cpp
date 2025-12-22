/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "analysis.h"
#include "complex_id.h"

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
      if (a.GetAttachedAtom(i).GetType().z >= 1) {
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
      if (to.atom->GetAttachedAtom(i).GetType().z >= 1) {
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
              if (b.atom->GetType().z < 1) {
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

typedef olx_pair_t<const TCAtom::Site*, const cm_Element*> SiteInfo;
typedef TTypeList<SiteInfo> AtomEnvList;
int RSA_CompareSites(const SiteInfo& a, const SiteInfo& b) {
  return olx_cmp(a.GetB()->z, b.GetB()->z);
}

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

int RSA_GetAtomPriorityX(RSA_BondOrder &boa, AtomEnvList& a, AtomEnvList& b, olxstr_buf *bf) {
  size_t sz = a.Count();
  if (sz == 0) {
    return 0;
  }
  for (size_t i = 0; i < sz; i++) {
    size_t ai = a.Count() - i - 1;
    size_t bi = b.Count() - i - 1;
    int res = RSA_CompareSites(a[ai], b[bi]);
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
      if (bf != 0 && false) {
        (*bf) << "\n      "
          << strof(a[ai]) << (res < 0 ? "< " : "> ") << strof(b[bi]);
      }
      return res;
    }
  }
  // equal? expand further
  TTypeList<AtomEnvList> al, bl;
  for (size_t i = 0; i < sz; i++) {
    AtomEnvList &aa = al.AddNew(),
      &bb = bl.AddNew();
    if (a[i].GetA() != 0) {
      TCAtom& atomA = *a[i].GetA()->atom;
      for (size_t j = 0; j < atomA.AttachedSiteCount(); j++) {
        TCAtom::Site& s = atomA.GetAttachedSite(j);
        if (s.atom->GetType().z < 1 || s.atom->IsDeleted()) {
          continue;
        }
        // stop propagation as the site is in use
        if (s.atom->GetTag() != 0) {
          if ((s.atom->GetTag() & 2) == 0) {
            aa.Add(new SiteInfo(0, &s.atom->GetType()));
          }
          else {
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
        if (s.atom->GetType().z < 1 || s.atom->IsDeleted()) {
          continue;
        }
        // stop propagation as the site is in use
        if (s.atom->GetTag() != 0) {
          if ((s.atom->GetTag() & 4) == 0) {
            bb.Add(new SiteInfo(0, &s.atom->GetType()));
          }
          else {
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
    BubbleSorter::Sort(al,FunctionComparator::Make(&RSA_CompareSubs),
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
  else {
    BubbleSorter::SortSF(al, &RSA_CompareSubs);
    BubbleSorter::SortSF(bl, &RSA_CompareSubs);
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
  return RSA_GetAtomPriorityX(boa, a, b, bf);
}

struct RSA_EnviSorter {
  TCAtom& center;
  bool debug;
  mutable olxstr_buf out;
  mutable olx_pset<uint64_t> reported;
  RSA_BondOrder& boa;
  RSA_EnviSorter(RSA_BondOrder &boa, TCAtom& center, bool debug)
    : boa(boa), center(center),
    debug(debug)
  {}

  int Comparator(const TCAtom::Site& a, const TCAtom::Site& b) const {
    a.atom->GetParent()->GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    center.SetTag(6);
    AtomEnvList ea, eb;
    ea.Add(new SiteInfo(&a, &a.atom->GetType()));
    eb.Add(new SiteInfo(&b, &b.atom->GetType()));
    olxstr_buf da;
    int res = RSA_GetAtomPriorityX(boa, ea, eb, debug ? &da: 0);
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
};
//.............................................................................
olxstr xlib::olx_analysis::chirality::rsa_analyse(TCAtom& a, bool debug) {
  if (a.IsDeleted() || a.GetType() < 2) {
    return EmptyString();
  }
  TPtrList<TCAtom::Site> attached;
  for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
    TCAtom& aa = a.GetAttachedAtom(j);
    if (aa.IsDeleted() || aa.GetType() == iQPeakZ) {
      continue;
    }
    attached.Add(a.GetAttachedSite(j));
  }
  RSA_BondOrder boa;
  olxstr w;
  if (attached.Count() == 4) {
    a.ClearChiralFlag();
    RSA_EnviSorter es(boa, a, debug);
    BubbleSorter::SortMF(attached, es, &RSA_EnviSorter::Comparator);
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
      TBasicApp::NewLogEntry() << "For " << a.GetLabel()
        << es.out;
    }
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
  return w;
}
//.............................................................................
