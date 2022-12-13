#include "analysis.h"

int RSA_BondOrder(const TCAtom& a, const TCAtom::Site& to) {
  if (a.GetType() == iHydrogenZ || to.atom->GetType() == iHydrogenZ) {
    return 1;
  }
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
    return a_cnt == 2 ? 1 : 2;
  }
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

typedef olx_pair_t<const TCAtom::Site*, const cm_Element*> SiteInfo;
typedef TTypeList<SiteInfo > AtomEnvList;
int RSA_CompareSites(const SiteInfo& a, const SiteInfo& b) {
  return olx_cmp(a.GetB()->z, b.GetB()->z);
}
int RSA_GetAtomPriorityX(AtomEnvList& a, AtomEnvList& b, bool debug) {
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
      return res;
    }
  }
  // equal? expand further
  for (size_t i = 0; i < sz; i++) {
    AtomEnvList aa, bb;
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
        int bo = RSA_BondOrder(atomA, s);
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
        int bo = RSA_BondOrder(atomB, s);
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
    for (size_t ai = 0; ai < aa.Count(); ai++) {
      a.Add(aa[ai]);
    }
    aa.ReleaseAll();
    BubbleSorter::SortSF(bb, &RSA_CompareSites);
    for (size_t ai = 0; ai < bb.Count(); ai++) {
      b.Add(bb[ai]);
    }
    bb.ReleaseAll();
  }
  if (debug) {
    olxstr_buf out1, out2;
    for (size_t i = 0; i < a.Count(); i++) {
      if (i == sz) {
        out1 << " -> ";
        out2 << " -> ";
      }
      olxstr sa, sb;
      if (a[i].a == 0) {
        sa << "{" << a[i].b->symbol << "}";
      }
      else {
        sa << a[i].a->atom->GetLabel();
      }
      if (b[i].a == 0) {
        sb << "{" << b[i].b->symbol << "}";
      }
      else {
        sb << b[i].a->atom->GetLabel();
      }
      out1 << sa.RightPadding(5, ' ');
      out2 << sb.RightPadding(5, ' ');
    }
    TBasicApp::NewLogEntry() << out1;
    TBasicApp::NewLogEntry() << out2;
  }
  a.DeleteRange(0, sz);
  b.DeleteRange(0, sz);
  return RSA_GetAtomPriorityX(a, b, debug);
}
struct RSA_EnviSorter {
  TCAtom& center;
  bool debug;
  RSA_EnviSorter(TCAtom& center, bool debug)
    : center(center),
    debug(debug)
  {}

  int Comparator(const TCAtom::Site& a, const TCAtom::Site& b) const {
    a.atom->GetParent()->GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    center.SetTag(6);
    AtomEnvList ea, eb;
    ea.Add(new SiteInfo(&a, &a.atom->GetType()));
    eb.Add(new SiteInfo(&b, &b.atom->GetType()));
    return RSA_GetAtomPriorityX(ea, eb, debug);
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
  olxstr w;
  if (attached.Count() == 4) {
    a.ClearChiralFlag();
    if (debug) {
      TBasicApp::NewLogEntry() << "For " << a.GetLabel();
    }
    RSA_EnviSorter es(a, debug);
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
