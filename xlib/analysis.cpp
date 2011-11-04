#include "analysis.h"

using namespace olx_analysis;

//.............................................................................
double alg::mean_peak(const TCAtomPList &peaks) {
  double v = 0;
  size_t cnt = 0;
  for (size_t i=0; i < peaks.Count(); i++) {
    if(peaks[i]->IsDeleted()) continue;
    cnt++;
    v += peaks[i]->GetQPeak();
  }
  return (cnt > 1 ? v/cnt : v);
}
//.............................................................................
double alg::mean_u_eq(const TCAtomPList &atoms) {
  double v = 0;
  size_t cnt = 0;
  for (size_t i=0; i < atoms.Count(); i++) {
    if(atoms[i]->IsDeleted()|| atoms[i]->GetType().z < 2 ) continue;
    cnt++;
    v += atoms[i]->GetUiso();
  }
  return (cnt > 1 ? v/cnt : v);
}
//.............................................................................
//.............................................................................
void peaks::range::delete_all() {
  for (size_t i=0; i < peaks.Count(); i++)
    peaks[i]->SetDeleted(true);
}
//.............................................................................
//.............................................................................
ConstTypeList<peaks::range> peaks::analyse(const TCAtomPList &_peaks) {
  TTypeList<range> ranges;
  TCAtomPList peaks = _peaks;
  peaks.QuickSorter.SortSF(peaks, peak_sort);
  if (!peaks.IsEmpty())
    ranges.AddNew().peaks.Add(peaks.GetLast());
  for (size_t i=peaks.Count()-2; i != InvalidIndex; i--) {
    TCAtomPList &range = ranges.GetLast().peaks;
    TCAtom *cmpr = range.GetLast();
    if (peaks[i]->GetQPeak()/cmpr->GetQPeak() < 1.25 &&
      peaks[i]->GetQPeak()/range[0]->GetQPeak() < 2)
      range.Add(peaks[i]);
    else
      ranges.AddNew().peaks.Add(peaks[i]);
  }
  return ranges;
}
//.............................................................................
ConstPtrList<TCAtom> peaks::extract(TAsymmUnit &au,
    bool *_all_peaks)
{
  bool all_peaks = true;
  TCAtomPList peaks;
  peaks.SetCapacity(au.AtomCount());
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.IsDeleted()) continue;
    if (a.GetType() == iQPeakZ)
      peaks.Add(a);
    else
      all_peaks = false;
  }
  if (_all_peaks != NULL)
    *_all_peaks = all_peaks;
  return peaks;
}
//.............................................................................
peaks::result peaks::analyse_full(TAsymmUnit &au) {
  peaks::result res;
  TCAtomPList peaks = extract(au, &res.only_peaks);
  res.mean_peak = alg::mean_peak(peaks);
  res.peak_count = peaks.Count();
  res.peak_ranges = analyse(peaks);
  return res;
}
//.............................................................................
TCAtomPList &peaks::proximity_clean(TCAtomPList &peaks) {
  if (peaks.IsEmpty()) return peaks;
  const TAsymmUnit &au = *peaks[0]->GetParent();
  for (size_t i=0; i < peaks.Count(); i++) {
    if (peaks[i]->IsDeleted()) continue;
    for (size_t si=0; si < peaks[i]->AttachedSiteCount(); si++) {
      TCAtom::Site &st = peaks[i]->GetAttachedSite(si);
      double d = au.Orthogonalise(
        peaks[i]->ccrd()-st.matrix*st.atom->ccrd()).Length();
      if (d < 0.95) {
        if (st.atom->GetType() == iQPeakZ) {
          if( peaks[i]->GetQPeak() < st.atom->GetQPeak())
            peaks[i]->SetDeleted(true);
          else
            st.atom->SetDeleted(true);
        }
        else
          peaks[i]->SetDeleted(true);
      }
      if (peaks[i]->IsDeleted()) break;
    }
  }
  return peaks.Pack(TCAtom::FlagsAnalyser<>(catom_flag_Deleted));
}
//.............................................................................
//.............................................................................
void fragments::fragment::build_coordinate(
  TCAtom &a, const smatd &m_, vec3d_list &res)
{
  const vec3d v = m_.IsFirst() ? a.ccrd() : m_*a.ccrd();
  res[a.GetTag()] = a.GetParent()->Orthogonalise(v);
  a.SetTag(-1);
  const TUnitCell &uc = a.GetParent()->GetLattice().GetUnitCell();
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom::Site &st = a.GetAttachedSite(i);
    if (st.atom->GetTag() < 0) continue;
    smatd m = uc.MulMatrix(m_, st.matrix);
    build_coordinate(*st.atom, m, res);
  }
}
//.............................................................................
ConstTypeList<vec3d> fragments::fragment::build_coordinates() const {
  //pack();
  vec3d_list set(atoms_.Count(), true);
  if (atoms_.IsEmpty()) return set;
  atoms_.ForEach(ACollectionItem::IndexTagSetter<>());
  build_coordinate(*atoms_[0],
    atoms_[0]->GetParent()->GetLattice().GetUnitCell().GetMatrix(0),
    set);
  return set;
}
//.............................................................................
void fragments::fragment::init_generators() {
  if (atoms_.IsEmpty()) return;
  generators.AddList(
    atoms_[0]->GetParent()->GetLattice().GetFragmentGrowMatrices(atoms_));
}
//.............................................................................
bool fragments::fragment::is_regular() const {
  size_t ci = find_central_index();
  if (ci == InvalidIndex) return false;
  vec3d_list crds = build_coordinates();
  if (is_polymeric()) { // grow missing coordinates
    
  }
  if (crds.Count() < 3) return true;
  if (crds.Count() == 3) { // check if 180 +- 10 degrees, linear arrangement
    TSizeList indices(3, list_init::index());
    indices.Remove(ci);
    double ca = (crds[indices[0]]-crds[ci]).CAngle(crds[indices[1]]-crds[ci]);
    return (ca+1) < 0.03;
  }
  else if (crds.Count() == 4) { // check flat or tetrahedral arrangement
    TSizeList indices(4, list_init::index());
    indices.Remove(ci);
    vec3d cnt = (crds[indices[0]]+crds[indices[1]]+crds[indices[2]])/3;
    vec3d rc = crds[ci]-cnt;
    // check rc to perepheral atoms is 90 +- 10 (if not flat)
    if (rc.Length() > 0.1) {
      for (size_t i=0; i < 3; i++) {
        double ca = (crds[indices[i]]-cnt).CAngle(rc);
        if (olx_abs(ca) > 0.17) return false;
      }
    }
    // finally - check the angles...
    vec3d v1 = crds[indices[0]]-cnt;
    // cos 120 = -0.5, +- 10
    if ((v1.CAngle(crds[indices[1]]-cnt)+0.5) > 0.16) return false; 
    if ((v1.CAngle(crds[indices[2]]-cnt)+0.5) > 0.16) return false; 
    return true;
  }
  else { // spherical arrangement test
    TSizeList indices(atoms_.Count()-1, list_init::index());
    indices.Remove(ci);
    vec3d cnt = olx_mean<vec3d>::calc(
      indices, ConstIndexAccessor<vec3d, vec3d_list>(crds));

  }
  return false;
}
//.............................................................................
size_t fragments::fragment::find_central_index() const {
  size_t idx = InvalidIndex;
  for (size_t i=0; i < atoms_.Count(); i++) {
    size_t sc = atoms_[i]->AttachedSiteCount();
    if (sc == atoms_.Count())
      idx = i;
    else if (sc != 1)
      return InvalidIndex;
  }
  return idx;
}
//.............................................................................
bool fragments::fragment::is_flat() const {
  if (atoms_.IsEmpty()) return true;
  vec3d n;
  vec3d_list crds = build_coordinates();
  double rmsd = 0;
  const TAsymmUnit &au = *atoms_[0]->GetParent();
  if (atoms_.Count() > 2) { 
    TArrayList<AnAssociation2<vec3d, double> > points(crds.Count());
    vec3d center;
    rmsd = TSPlane::CalcPlane(points, n, center, plane_best);
  }
  else {
    if (atoms_.Count() == 2)
      n = crds[0]-crds[1];
    else if (is_polymeric()) { // 1
      n = crds[0] - au.Fractionalise(generators[0]*atoms_[0]->ccrd());
    }
  }
  if (is_polymeric()) {
    for (size_t i=0; i < generators.Count(); i++) {
      vec3d tn = au.Orthogonalise(generators[i]*au.Fractionalise(n));
      // are the transformed and the original normals colinear?
      if (olx_abs(olx_abs(tn.CAngle(n))-1) > 0.02)
        return false;
    }
    return true;
  }
  else {
    return rmsd < 0.05;
  }
}
//.............................................................................
//.............................................................................
void fragments::expand_node(TCAtom &a, TCAtomPList &atoms) {
  atoms.Add(a)->SetTag(1);
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.IsDeleted() || aa.GetTag() != 0) continue;
    expand_node(aa, atoms);
  }
}
//.............................................................................
ConstTypeList<fragments::fragment> fragments::extract(TAsymmUnit &au) {
  TTypeList<fragments::fragment> rv;
  TCAtomPList &atoms = au.GetAtoms();
  atoms.ForEach(ACollectionItem::TagSetter<>(0));
  size_t cnt = 0;
  for (size_t i=0; i < atoms.Count(); i++) {
    if (atoms[i]->IsDeleted()) continue;
    if (atoms[i]->GetTag() == 0) {
      TCAtomPList catoms;
      expand_node(*atoms[i], catoms);
      rv.AddNew().set_atoms(catoms);
      if ( (cnt+=catoms.Count()) == atoms.Count())
        break;
    }
  }
  return rv;
}

//.............................................................................
//.............................................................................
bool Analysis::trim_18(TAsymmUnit &au) {
  size_t mult = (au.MatrixCount()+1)*
    TCLattice::GetLattMultiplier(au.GetLatt());
  size_t mn = olx_round(au.CalcCellVolume()/(mult*18));
  if (au.AtomCount() <= mn)
    return false;
  TCAtomPList atoms;
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.GetType().z < 2 || a.IsDeleted()) continue;
    atoms.Add(a);
  }
  TCAtomPList peaks = peaks::extract(au);
  peaks::proximity_clean(peaks);
  TTypeList<peaks::range> peak_ranges = peaks::analyse(
    peaks::extract(au));
  if (mn > atoms.Count()) {
    size_t i = peak_ranges.Count();
    while (--i !=InvalidIndex && mn > atoms.Count()) {
      if (peak_ranges[i].get_mean() < 1.5) break;
      atoms.AddList(peak_ranges[i].peaks);
    }
    for (; i != InvalidIndex; i--)
      peak_ranges[i].delete_all();
  }
  else {
    size_t p_i = peak_ranges.Count();
    while (--p_i !=InvalidIndex) {
      if (peak_ranges[p_i].get_mean() > 2)
        atoms.AddList(peak_ranges[p_i].peaks);
      else break;
    }
    for (; p_i != InvalidIndex; p_i--)
      peak_ranges[p_i].delete_all();
    //for (size_t i=mn; i < atoms.Count(); i++)
    //  atoms[i]->SetDeleted(true);
  }
  if (atoms.Count() > mn*1.25) {
    for (size_t i=mn; i < atoms.Count(); i++)
      atoms[i]->SetDeleted(true);
  }
  return true;
}
//.............................................................................
double Analysis::find_scale(TLattice &latt) {
  ConstTypeList<TAutoDB::TAnalysisResult> ares =
    TAutoDB::GetInstance().AnalyseStructure(latt);
  double res = 0, wght = 0;;
  for (size_t i=0; i < ares.Count(); i++)  {
    if (ares[i].atom->GetType() == iQPeakZ) {
      TTypeList<TAutoDB::Type> hits;
      if(!ares[i].list3.IsEmpty())
        hits = ares[i].list3;
      else if(!ares[i].list2.IsEmpty())
        hits = ares[i].list2;
      else  {
        hits = ares[i].list1;
        hits.AddList(ares[i].enforced);
      }
      size_t m = olx_min(2, hits.Count());
      for (size_t j=0; j < m; j++) {
        res += hits[j].type.z*hits[j].fom/ares[i].atom->GetQPeak();
        wght += hits[j].fom;
      }
    }
  }
  if (wght!=0)
    res /= wght;
  return res;
}
//.............................................................................
bool Analysis::analyse_u_eq(TAsymmUnit &au) {
  TTypeList<fragments::fragment> frags =
    fragments::extract(au);
  for (size_t i=0; i < frags.Count(); i++) {
    TBasicApp::NewLogEntry() << frags[i].get_mean_u_eq();
  }
  return false;
}
//.............................................................................
//.............................................................................
//.............................................................................
