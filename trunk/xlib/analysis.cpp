#include "analysis.h"
#include "equeue.h"
#include "auto.h"
#include "eset.h"

using namespace olx_analysis;

//.............................................................................
double alg::mean_peak(const TCAtomPList &peaks) {
  double v = 0;
  size_t cnt = 0;
  for (size_t i=0; i < peaks.Count(); i++) {
    if (peaks[i]->IsDeleted()) {
      continue;
    }
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
    if (atoms[i]->IsDeleted() || atoms[i]->GetType().z < 2) {
      continue;
    }
    cnt++;
    v += atoms[i]->GetUiso();
  }
  return (cnt > 1 ? v/cnt : v);
}
//.............................................................................
olxstr alg::formula(const TCAtomPList &atoms, double mult) {
  ElementPList elements;
  ContentList cl;
  for (size_t i = 0; i < atoms.Count(); i++) {
    const cm_Element& elm = atoms[i]->GetType();
    if (atoms[i]->IsDeleted() || elm == iQPeakZ) {
      continue;
    }
    const size_t ind = elements.IndexOf(elm);
    if (ind == InvalidIndex) {
      cl.AddNew(elm, atoms[i]->GetOccu()*mult);
      elements.Add(elm);
    }
    else {
      cl[ind] += atoms[i]->GetOccu()*mult;
    }
  }
  XElementLib::SortContentList(cl);
  olxstr rv;
  for (size_t i = 0; i < cl.Count(); i++) {
    rv << cl[i].element.symbol;
    if (olx_abs(cl[i].count - 1.0) > 1e-3) {
      rv << olxstr::FormatFloat(3, cl[i].count).TrimFloat();
    }
    if ((i + 1) < cl.Count()) {
      rv << ' ';
    }
  }
  return rv;
}
//.............................................................................
olxstr alg::label(const TCAtomPList &atoms, const olxstr &sp) {
  olxstr_buf rv;
  if (atoms.IsEmpty()) {
    return rv;
  }
  for( size_t i=0; i < atoms.Count(); i++ ) {
    rv << sp << atoms[i]->GetResiLabel();
  }
  return olxstr(rv).SubStringFrom(sp.Length());
}
//.............................................................................
olxstr alg::label(const TCAtomGroup &atoms, const olxstr &sp) {
  olxstr_buf rv;
  if (atoms.IsEmpty()) {
    return rv;
  }
  RefinementModel &rm = *atoms[0].GetAtom()->GetParent()->GetRefMod();
  for( size_t i=0; i < atoms.Count(); i++ ) {
    rv << sp << atoms[i].GetFullLabel(rm);
  }
  return olxstr(rv).SubStringFrom(sp.Length());
}
//.............................................................................
const cm_Element &alg::find_heaviest(const TCAtomPList &atoms) {
  if (atoms.IsEmpty())
    return XElementLib::GetByIndex(iQPeakIndex);
  size_t ind=0;
  for (size_t i=1; i < atoms.Count(); i++) {
    if (atoms[i]->GetType().z > atoms[ind]->GetType().z) {
      ind = i;
    }
  }
  return atoms[ind]->GetType();
}
//.............................................................................
bool alg::check_geometry(const TCAtom &a, const cm_Element &e) {
  double diff = rate_envi(a.GetParent()->GetLattice().GetUnitCell(),
    a.ccrd(), e.r_bonding+1.3) -
      rate_envi(a.GetParent()->GetLattice().GetUnitCell(),
      a.ccrd(), a.GetType().r_bonding+1.3);
  return diff > -0.05;
}
//.............................................................................
bool alg::check_connectivity(const TCAtom &a, const cm_Element &e) {
  size_t cc = 0, metal_c = 0;
  size_t halogen_c = 0, calcogen_c = 0, oxygen_c=0;
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.GetType() != iQPeakZ && !aa.IsDeleted()) {
      cc++;
      if (XElementLib::IsMetal(aa.GetType())) {
        metal_c++;
      }
      else if (XElementLib::IsHalogen(aa.GetType())) {
        halogen_c++;
      }
      else if (XElementLib::IsChalcogen(aa.GetType())) {
        calcogen_c ++;
        if (aa.GetType() == iOxygenZ) {
          oxygen_c++;
        }
      }
    }
  }
  if (e == iHydrogenZ) {
    return cc == 1;
  }
  if (XElementLib::IsMetal(e)) {
    if (cc <= 1) {
      return false;
    }
    if (cc == 2) {
      return XElementLib::IsTtransitionalGroup(1, e);
    }
  }
  else {
    cc -= metal_c;
    if (XElementLib::IsChalcogen(e)) {
      if (e == iOxygenZ) {
        return cc <= 2;
      }
      return cc > 0;
    }
    if (XElementLib::IsHalogen(e)) {
      cc -= halogen_c; // I3 I-Cl-I etc
      if (e.z == iChlorineZ) { // ClO4 etc
        cc -= oxygen_c;
      }
      else { // BrSe3 is common
        cc -= (calcogen_c - oxygen_c);
      }
      return cc <= 1;
    }
    if (XElementLib::IsGroup4(e)) {
      return cc > 0 || metal_c > 1; // carbides?
    }
    if (XElementLib::IsGroup5(e) && e != iNitrogenZ) {
      return cc > 0;
    }
  }
  return true;
}
//.............................................................................
ConstArrayList<size_t> alg::find_hetero_indices(const TCAtomPList &atoms,
  const cm_Element *re)
{
  const short z = (re == NULL ? iCarbonZ : re->z);
  return &olx_list_filter::FilterIndices(
    atoms,
    *(new TArrayList<size_t>),
    olx_alg::olx_not(TCAtom::TypeAnalyser(z)));
}
//.............................................................................
double alg::rate_envi(const TUnitCell &uc, const vec3d& fcrd, double r) {
  TArrayList<olx_pair_t<TCAtom const*, vec3d> > res;
  uc.FindInRangeAC(fcrd, r, res);
  const vec3d center = uc.GetLattice().GetAsymmUnit().Orthogonalise(fcrd);
  for (size_t j = 0; j < res.Count(); j++) {
    if (center.QDistanceTo(res[j].GetB()) < 1e-4 ||
      res[j].GetA()->GetType() < 2)
    {
      res.Delete(j--);
    }
  }
  double wght = 1;
  if (res.Count() > 1) {
    double awght = 1. / (res.Count()*(res.Count() - 1));
    for (size_t j = 0; j < res.Count(); j++) {
      if (res[j].GetB().QLength() < 0.8)
        wght -= 0.5 / res.Count();
      for (size_t k = j + 1; k < res.Count(); k++) {
        double cang = (res[j].GetB() - center).CAngle(res[k].GetB() - center);
        if (cang > 0.588) { // less than 56 degrees
          wght -= awght;
        }
      }
    }
  }
  else if (res.Count() == 1) {  // just one bond
    if (res[0].GetB().QLength() < 0.8) {
      wght = 0;
    }
  }
  //else  // no bonds, cannot say anything
  //  wght = 0;
  return wght;
}
//.............................................................................
//.............................................................................
ConstSortedElementPList helper::get_user_elements() {
  TXApp& xapp = TXApp::GetInstance();
  const ContentList& cl = xapp.XFile().GetRM().GetUserContent();
  SortedElementPList rv;
  for (size_t i = 0; i < cl.Count(); i++) {
    rv.AddUnique(&cl[i].element);
  }
  return rv;
}
//.............................................................................
void helper::reset_u(TCAtom &a, double r) {
  a.SetUiso(r);
  if (a.GetEllipsoid() != 0) {
    a.GetEllipsoid()->ToSpherical(r);
  }
}
//.............................................................................
void helper::delete_atom(TCAtom &a) {
  a.SetDeleted(true);
  TPtrList<TAfixGroup> ags;
  if (a.GetDependentAfixGroup() != 0) {
    ags.Add(a.GetDependentAfixGroup());
  }
  for (size_t i = 0; i < a.DependentHfixGroupCount(); i++) {
    ags.Add(a.GetDependentHfixGroup(i));
  }
  for (size_t i=0; i < ags.Count(); i++) {
    for (size_t j = 0; j < ags[i]->Count(); j++) {
      delete_atom((*ags[i])[j]);
    }
  }
}
//.............................................................................
size_t helper::get_demoted_index(const cm_Element &e, const SortedElementPList &elms) {
  size_t idx = elms.IndexOf(e);
  if (idx == InvalidIndex) {
    for (size_t i=0; i < elms.Count(); i++) {
      if (elms[i]->z > e.z) {
        if (i == 0) {
          return InvalidIndex;
        }
        if (i > 0 && elms[i - 1]->z == iHydrogenZ) {
          return InvalidIndex;
        }
        return i;
      }
    }
    return InvalidIndex;
  }
  else {
    if ((idx == 1 && elms[0]->z == iHydrogenZ) || idx == 0) {
      return InvalidIndex;
    }
    return idx;
  }
}
//.............................................................................
bool helper::can_demote(const cm_Element &e, const SortedElementPList &elms) {
  return get_demoted_index(e, elms) != InvalidIndex;
}
//.............................................................................
bool helper::can_demote(const TCAtom &a, const SortedElementPList &elms) {
  const bool return_any = !alg::check_connectivity(a, a.GetType());
  size_t idx = elms.IndexOf(a.GetType());
  if (idx == InvalidIndex) {
    for (size_t i=0; i < elms.Count(); i++) {
      if (elms[i]->z > a.GetType().z) {
        if (i == 0) {
          return false;
        }
        if (i > 0 && elms[i - 1]->z == iHydrogenZ) {
          return false;
        }
        if (return_any || alg::check_connectivity(a, *elms[i])) {
          return true;
        }
      }
    }
    return false;
  }
  else {
    if (idx == 0) {
      return false;
    }
    for (size_t i=idx-1; i != InvalidIndex; i--) {
      if (elms[i]->z == iHydrogenZ) {
        return false;
      }
      if (return_any || alg::check_connectivity(a, *elms[i])) {
        return true;
      }
    }
    return false;
  }
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
  if (_peaks.IsEmpty()) {
    return ranges;
  }
  TCAtomPList peaks = _peaks;
  QuickSorter::SortSF(peaks, peak_sort);
  if (!peaks.IsEmpty()) {
    ranges.AddNew().peaks.Add(peaks.GetLast());
  }
  for (size_t i=peaks.Count()-2; i != InvalidIndex; i--) {
    TCAtomPList &range = ranges.GetLast().peaks;
    TCAtom *cmpr = range.GetLast();
    if (peaks[i]->GetQPeak() / cmpr->GetQPeak() < 1.25 &&
      peaks[i]->GetQPeak() / range[0]->GetQPeak() < 2)
    {
      range.Add(peaks[i]);
    }
    else {
      ranges.AddNew().peaks.Add(peaks[i]);
    }
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
    if (a.GetType() == iQPeakZ) {
      peaks.Add(a);
    }
    else {
      all_peaks = false;
    }
  }
  if (_all_peaks != 0) {
    *_all_peaks = all_peaks;
  }
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
  if (peaks.IsEmpty()) {
    return peaks;
  }
  const TAsymmUnit &au = *peaks[0]->GetParent();
  for (size_t i=0; i < peaks.Count(); i++) {
    if (peaks[i]->IsDeleted()) {
      continue;
    }
    for (size_t si=0; si < peaks[i]->AttachedSiteCount(); si++) {
      TCAtom::Site &st = peaks[i]->GetAttachedSite(si);
      double d = au.Orthogonalise(
        peaks[i]->ccrd()-st.matrix*st.atom->ccrd()).Length();
      if (d < 0.95) {
        if (st.atom->GetType() == iQPeakZ) {
          if (peaks[i]->GetQPeak() < st.atom->GetQPeak()) {
            peaks[i]->SetDeleted(true);
          }
          else {
            st.atom->SetDeleted(true);
          }
        }
        else {
          peaks[i]->SetDeleted(true);
        }
      }
      if (peaks[i]->IsDeleted()) {
        break;
      }
    }
  }
  return peaks.Pack(TCAtom::FlagsAnalyser(catom_flag_Deleted));
}
//.............................................................................
//.............................................................................
void fragments::tree_node::reverse() {
  olx_reverse(trunk);
  trunk.ForEach(ACollectionItem::IndexTagSetter());
  sort();
}
//.............................................................................
//.............................................................................
vec3d fragments::cart_ring::calc_center() const {
  vec3d cent;
  for (size_t i = 0; i < atoms.Count(); i++) {
    cent += atoms[i].xyz;
  }
  return (cent /= atoms.Count());
}
//.............................................................................
fragments::cart_plane fragments::cart_ring::calc_plane() const {
  cart_plane cp;
  cp.center = calc_center();
  mat3d m;
  for (size_t i = 0; i < atoms.Count(); i++) {
    const vec3d t = (atoms[i].xyz - cp.center);
    m[0][0] += (t[0] * t[0]);
    m[0][1] += (t[0] * t[1]);
    m[0][2] += (t[0] * t[2]);
    m[1][1] += (t[1] * t[1]);
    m[1][2] += (t[1] * t[2]);
    m[2][2] += (t[2] * t[2]);
  }
  m[1][0] = m[0][1];
  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  mat3d normals;
  mat3d::EigenValues(m, normals.I());
  int mi = 0;
  if (m[1][1] < m[mi][mi]) {
    mi = 1;
  }
  if (m[2][2] < m[mi][mi]) {
    mi = 2;
  }
  cp.normal = normals[mi];
  cp.rmsd = sqrt(olx_max(0, m[mi][mi]));
  return cp;
}
//.............................................................................
bool fragments::cart_ring::is_regular() const {
  vec3d cent = calc_center();
  double avAng = 2 * M_PI / atoms.Count(),
    avDis = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    avDis += cent.DistanceTo(atoms[i].xyz);
  }
  avDis /= atoms.Count();
  for (size_t i = 0; i < atoms.Count(); i++) {
    double d = cent.DistanceTo(atoms[i].xyz);
    if (olx_abs(d - avDis) > 0.2) {
      return false;
    }
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    vec3d a = atoms[i].xyz, b;
    if ((i + 1) == atoms.Count()) {
      b = atoms[0].xyz;
    }
    else {
      b = atoms[i + 1].xyz;
    }
    a -= cent;
    b -= cent;
    double ca = a.CAngle(b);
    if (ca < -1) {
      ca = -1;
    }
    if (ca > 1) {
      ca = 1;
    }
    ca = acos(ca);
    if (olx_abs(ca - avAng) > 5. / M_PI) {
      return false;
    }
  }
  return true;
}
//.............................................................................
//.............................................................................
bool fragments::ring::is_leq(const ring &r) const {
  atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, true));
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (!atoms[i]->IsProcessed()) {
      return false;
    }
  }
  return true;
}
//.............................................................................
bool fragments::ring::is_fused_with(const ring &r) const {
  atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, true));
  size_t prev_ind=InvalidIndex;
  for (size_t i=0; i < atoms.Count(); i++) {
    if (atoms[i]->IsProcessed()) {
      if (prev_ind != InvalidIndex) {
        if (i == prev_ind + 1 || (prev_ind == 0 && i == atoms.Count() - 1)) {
          return true;
        }
      }
      prev_ind = i;
    }
  }
  return false;
}
//.............................................................................
bool fragments::ring::merge(ring &r) {
  atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, true));
  TSizeList tii, tai;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->IsProcessed()) {
      tii << i;
    }
  }
  if (tii.Count() != 2) {
    return false;
  }

  atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, false));
  for (size_t i = 0; i < r.atoms.Count(); i++) {
    if (!r.atoms[i]->IsProcessed()) {
      tai << i;
    }
  }
  if (tii[0] == 0 && tii[1] == atoms.Count() - 1) {
    ;//atoms.ShiftR(1);
  }
  else if (tii[0] + 1 == tii[1]) {
    atoms.ShiftL(tii[0] + 1);
  }
  else {
    return false;
  }

  if (tai[0] == 0 && tai[1] == r.atoms.Count() - 1) {
    r.atoms.ShiftR(1);
  }
  else if (tai[0] + 1 == tai[1]) {
    r.atoms.ShiftL(tai[0]);
  }
  else {
    return false;
  }
  if (atoms[0] != r.atoms[1]) {
    atoms.AddList(r.atoms.SubListFrom(2));
  }
  else
    atoms.AddList(olx_list_reverse::MakeConst(r.atoms.SubListFrom(2)));
  for (size_t i=0; i < r.substituents.Count(); i++) {
    if (!r.substituents[i].atoms[0]->IsProcessed())
      substituents.AddCopy(r.substituents[i]);
  }
  fused_count++;
  return true;
}
//.............................................................................
int fragments::ring::Compare(const ring &r) const {
  int d = olx_cmp(atoms.Count(), r.atoms.Count());
  if (d != 0) {
    return d;
  }
  d = olx_cmp(substituents.Count(), r.substituents.Count());
  if (d != 0) {
    return d;
  }
  if (substituents.IsEmpty()) {
    return 0;
  }
  return substituents[0].Compare(r.substituents[0]);
}
//.............................................................................
fragments::cart_ring fragments::ring::to_cart() const {
  cart_ring r;
  if (atoms.IsEmpty()) {
    return r;
  }
  TAsymmUnit &au = *atoms[0]->GetParent();
  smatd m = smatd::Identity();
  r.atoms.Add(new cart_atom(atoms[0], au.Orthogonalise(atoms[0]->ccrd()), m));
  for (size_t i = 1; i < atoms.Count(); i++) {
    TCAtom &pa = *atoms[i - 1];
    bool found = false;
    for (size_t j = 0; j < pa.AttachedSiteCount(); j++) {
      TCAtom::Site &s = pa.GetAttachedSite(j);
      if (s.atom == atoms[i]) {
        m *= s.matrix;
        found = true;
        break;
      }
    }
    if (!found) {
      TBasicApp::NewLogEntry() << alg::label(atoms);
      throw TFunctionFailedException(__OlxSourceInfo, "disconnected ring");
    }
    else {
      r.atoms.Add(new cart_atom(atoms[i], au.Orthogonalise(m*atoms[i]->ccrd()), m));
#ifdef _DEBUG
      double d = r[i].xyz.DistanceTo(r[i - 1].xyz);
      if (d > 3) {
        TBasicApp::NewLogEntry() << d << ": " << r[i].atom.GetLabel() <<
          '-' << r[i-1].atom.GetLabel();
      }
#endif
    }
  }
  return r;
}
//.............................................................................
//.............................................................................
int fragments::ring::substituent::Compare(
  const fragments::ring::substituent &s) const
{
  int r = olx_cmp(ring_count, s.ring_count);
  if (r != 0) return r;
  if (ring_count == 1) {
    r = -olx_cmp(atoms.Count(), s.atoms.Count());
    if (r != 0) {
      return r;
    }
  }
  r = olx_cmp(alg::find_heaviest(atoms).z, alg::find_heaviest(s.atoms).z);
  if (r != 0) {
    return r;
  }
  return olx_cmp(atoms.Count(), s.atoms.Count());
}
//.............................................................................
//.............................................................................
//.............................................................................
void fragments::fragment::mask_neighbours(const TCAtomPList &atoms, index_t at,
  index_t nt)
{
  for (size_t i = 0; i < atoms.Count(); i++) {
    TCAtom &a = *atoms[i];
    for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
      a.GetAttachedAtom(j).SetTag(nt);
    }
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->SetTag(at);
  }
}
//.............................................................................
size_t fragments::fragment::get_neighbour_count(const TCAtom &a, index_t t) {
  size_t rv = 0;
  for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.IsDeleted() || aa.GetType().z < 0 || aa.GetTag() != t) {
      continue;
    }
    rv++;
  }
  return rv;
}
//.............................................................................
void fragments::fragment::order_list(TCAtomPList &inp) {
  sorted::PrimitiveAssociation<short, TCAtomPList> zo;
  for (size_t i = 0; i < inp.Count(); i++) {
    size_t idx = zo.IndexOf(inp[i]->GetType().z);
    if (idx == InvalidIndex) {
      zo.Add(inp[i]->GetType().z, TCAtomPList()).Value.Add(inp[i]);
    }
    else {
      zo.GetValue(idx).Add(inp[i]);
    }
  }
  mask_neighbours(inp, 1, 0);
  for (size_t i = 0; i < zo.Count(); i++) {
    TCAtomPList &l = zo.GetValue(i);
    TSizeList bc(l.Count());
    for (size_t j = 0; j < l.Count(); j++) {
      bc[j] = get_neighbour_count(*l[j], 1);
    }
    QuickSorter::Sort(bc, TPrimitiveComparator(), SyncSortListener::MakeSingle(l));
  }
  size_t idx = 0;
  for (size_t i = 0; i < zo.Count(); i++) {
    TCAtomPList &l = zo.GetValue(zo.Count()-i-1);
    // reverse here - guessing that more bonds is more unique
    for (size_t j = 0; j < l.Count(); j++) {
      inp[idx++] = l[l.Count()-j-1];
    }
  }
}
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
    if (st.atom->GetTag() < 0) {
      continue;
    }
    smatd m = uc.MulMatrix(m_, st.matrix);
    build_coordinate(*st.atom, m, res);
  }
}
//.............................................................................
ConstTypeList<vec3d> fragments::fragment::build_coordinates() const {
  vec3d_list crds(atoms_.Count(), true);
  if (atoms_.IsEmpty()) {
    return crds;
  }
  atoms_.ForEach(ACollectionItem::IndexTagSetter());
  build_coordinate(*atoms_[0],
    atoms_[0]->GetParent()->GetLattice().GetUnitCell().GetMatrix(0),
    crds);
  if (generators.Count() > 1) { // grow missing coordinates
    const TAsymmUnit &au = *atoms_[0]->GetParent();
    crds.SetCapacity(crds.Count()*(generators.Count()));
    const size_t crds_cnt = crds.Count();
    for (size_t i=1; i < generators.Count(); i++) {
      for (size_t j=0; j < crds_cnt; j++) {
        vec3d v = au.Orthogonalise(
          generators[i]*au.Fractionalise(crds[j]));
        bool uniq = true;
        for (size_t k=0; k < crds.Count(); k++) {
          if (crds[k].Equals(v, 1e-3)) {
            uniq = false;
            break;
          }
        }
        if (uniq) {
          crds.AddNew(v);
        }
      }
    }
  }
  return crds;
}
//.............................................................................
void fragments::fragment::init_generators() {
  if (atoms_.IsEmpty()) {
    return;
  }
  if (atoms_[0]->GetParent()->HasLattice()) {
    generators.AddList(
      atoms_[0]->GetParent()->GetLattice().GetFragmentGrowMatrices(atoms_,
      true));
  }
}
//.............................................................................
bool fragments::fragment::is_disjoint() const {
  depth_first_tags();
  for (size_t i = 0; i < atoms_.Count(); i++) {
    if (atoms_[i]->GetTag() == -1) {
      return true;
    }
  }
  return false;
}
//.............................................................................
bool fragments::fragment::is_polymeric() const {
  if (generators.Count() < 3) {
    return false;
  }
  smatd_list set(generators);
  for (size_t i=1; i < set.Count(); i++) {
    for (size_t j=i; j < set.Count(); j++) {
      smatd m = set[i]*set[j];
      bool uniq = true;
      for (size_t k=0; k < set.Count(); k++) {
        if (m.r == set[k].r) {
          if (m.t.Equals(set[k].t, 1e-3)) {
            uniq = false;
            break;
          }
          else {
            return true;
          }
        }
      }
      if (uniq) {
        set.Add(m);
      }
    }
  }
  return false;
}
//.............................................................................
bool fragments::fragment::is_regular() const {
  size_t ci = find_central_index();
  if (ci == InvalidIndex) {
    return false;
  }
  vec3d_list crds = build_coordinates();
  if (crds.Count() < 3) {
    return true;
  }
  if (crds.Count() == 3) { // check if 180 +- 10 degrees, linear arrangement
    TSizeList indices(3, olx_list_init::index());
    indices.Remove(ci);
    double ca = (crds[indices[0]]-crds[ci]).CAngle(crds[indices[1]]-crds[ci]);
    return (ca+1) < 0.03;
  }
  else if (crds.Count() == 4) { // check flat or tetrahedral arrangement
    TSizeList indices(4, olx_list_init::index());
    indices.Remove(ci);
    vec3d cnt = (crds[indices[0]]+crds[indices[1]]+crds[indices[2]])/3;
    vec3d rc = crds[ci]-cnt;
    // check rc to perepheral atoms is 90 +- 10 (if not flat)
    if (rc.Length() > 0.1) {
      for (size_t i=0; i < 3; i++) {
        double ca = (crds[indices[i]]-cnt).CAngle(rc);
        if (olx_abs(ca) > 0.17) {
          return false;
        }
      }
    }
    // finally - check the angles...
    vec3d v1 = crds[indices[0]]-cnt;
    // cos 120 = -0.5, +- 10
    if ((v1.CAngle(crds[indices[1]] - cnt) + 0.5) > 0.16) {
      return false;
    }
    if ((v1.CAngle(crds[indices[2]] - cnt) + 0.5) > 0.16) {
      return false;
    }
    return true;
  }
  // some spherical arrangement test
  else if (crds.Count() == 5 || crds.Count() == 7) {
    TSizeList indices(crds.Count(), olx_list_init::index());
    indices.Remove(ci);
    vec3d cnt = olx_mean(indices, IndexAccessor::MakeConst(crds));
    // check central atom is nearby the geometrical center
    if (cnt.DistanceTo(crds[ci]) > 0.1) {
      return false;
    }
    if (crds.Count() == 5) {
      for (size_t i=0; i < indices.Count(); i++) {
        vec3d pv = (crds[indices[i]]-crds[ci]);
        for (size_t j=i+1; j < indices.Count(); j++) {
          double ca = pv.CAngle((crds[indices[j]]-crds[ci]));
          if (olx_abs(ca + 1. / 3) > 0.16) { // cos(THA) = -1./3, +- 10
            return false;
          }
        }
      }
      return true;
    }
    else {
      size_t a90_cnt=0, a180_cnt=0;
      for (size_t i=0; i < indices.Count(); i++) {
        vec3d pv = (crds[indices[i]]-crds[ci]);
        for (size_t j=i+1; j < indices.Count(); j++) {
          double ca = pv.CAngle((crds[indices[j]]-crds[ci]));
          if (olx_abs(ca) < 0.17) { // 90 +- 10
            a90_cnt++;
          }
          else if ((ca + 1) < 0.03) { // 180 +- 10
            a180_cnt++;
          }
          else {
            return false;
          }
        }
      }
      return a180_cnt == 3;
    }
  }
  return false;
}
//.............................................................................
size_t fragments::fragment::find_central_index() const {
  size_t idx = InvalidIndex;
  for (size_t i=0; i < atoms_.Count(); i++) {
    size_t sc = atoms_[i]->AttachedSiteCount();
    if (sc >= atoms_.Count() - 1 && sc != 1) {
      idx = i;
    }
    else if (sc != 1) {
      return InvalidIndex;
    }
  }
  return idx;
}
//.............................................................................
bool fragments::fragment::is_flat() const {
  if (atoms_.IsEmpty()) {
    return true;
  }
  vec3d n;
  vec3d_list crds = build_coordinates();
  double rmsd = 0;
  if (crds.Count() <= 3) {
    return true;
  }
  else {
    TArrayList<olx_pair_t<vec3d, double> > points(crds.Count());
    for (size_t i=0; i < crds.Count(); i++) {
      points[i].a = crds[i];
      points[i].b = 1;
    }
    vec3d center;
    rmsd = TSPlane::CalcPlane(points, n, center, plane_best);
    return rmsd < 0.05;
  }
}
//.............................................................................
void fragments::fragment::depth_first_tag(TCAtom &a, index_t v) {
  a.SetTag(v);
  for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.GetTag() != -1) {
      continue;
    }
    depth_first_tag(aa, v + 1);
  }
}
//.............................................................................
void fragments::fragment::depth_first_tags(const TCAtomPList &atoms) {
  if (atoms.IsEmpty()) {
    return;
  }
  atoms.ForEach(ACollectionItem::TagSetter(-1));
  depth_first_tag(*atoms[0], 0);
}
//.............................................................................
void fragments::fragment::breadth_first_tags(const TCAtomPList &atoms,
  size_t start, TCAtomPList *ring_atoms)
{
  if (atoms.IsEmpty()) {
    return;
  }
  atoms.ForEach(ACollectionItem::TagSetter(-1));
  TQueue<TCAtom*> queue;
  if (start >= atoms.Count()) {
    start = 0;
  }
  queue.Push(atoms[start]);
  queue.Push(NULL);
  index_t tv = 0;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    if (a == NULL) {
      if (queue.IsEmpty()) {
        break;
      }
      tv++;
      queue.Push(NULL);
      continue;
    }
    if (a->GetTag() != -1) {
      if (a->GetTag() >= tv) {
        a->SetRingAtom(true);
        if (ring_atoms != NULL) {
          ring_atoms->AddUnique(a);
        }
      }
      continue;
    }
    a->SetTag(tv);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (!st.atom->IsAvailable()) {
        continue;
      }
      if (st.atom->GetTag() == -1) {
        queue.Push(st.atom);
      }
      else if (st.atom->GetTag() >= tv) {
        st.atom->SetRingAtom(true);
        if (ring_atoms != NULL) {
          ring_atoms->AddUnique(st.atom);
        }
      }
    }
  }
}
//.............................................................................
void fragments::fragment::init_rings(TTypeList<fragments::ring> &rings) {
  for (size_t i=0; i < rings.Count(); i++) {
    init_ring(i, rings);
    QuickSorter::Sort(rings[i].substituents);
    TArrayList<size_t> hi = alg::find_hetero_indices(rings[i].atoms);
    if (hi.Count() == 1 ) {
      rings[i].atoms.ShiftL(hi[0]);
      // minimise the subs indices by changing the direction if neeeded
      if (!rings[i].substituents.IsEmpty()) {
        rings[i].atoms.ForEach(ACollectionItem::IndexTagSetter());
        bool last_smallest=true;
        for (size_t j=0; j < rings[i].substituents.Count()-1; j++) {
          if (rings[i].substituents.GetLast().atoms[0]->GetTag() >
              rings[i].substituents[j].atoms[0]->GetTag())
          {
            last_smallest = false;
            break;
          }
        }
        if (!last_smallest ||
          ((size_t)rings[i].substituents.GetLast().atoms[0]->GetTag() >
            rings[i].atoms.Count()/2))
        {
          rings[i].reverse();
        }
      }
    }
    else {
      if (!rings[i].substituents.IsEmpty()) {
        QuickSorter::Sort(rings[i].substituents);
        // align atoms to start from the 'heaviest' substituent
        rings[i].atoms.ForEach(ACollectionItem::IndexTagSetter());
        rings[i].atoms.ShiftL(rings[i].substituents.GetLast().atoms[0]->GetTag());
      }
    }
  }
  QuickSorter::Sort(rings);
}
//.............................................................................
void fragments::fragment::init_ring(size_t i, TTypeList<ring> &rings) {
  if (atoms_.IsEmpty()) {
    return;
  }
  atoms_.ForEach(ACollectionItem::TagSetter(-1));
  ring &r = rings[i];
  TQueue<TCAtom*> queue;
  for (size_t i = 0; i < rings.Count(); i++) {
    rings[i].atoms.ForEach(ACollectionItem::TagSetter(-2));
  }
  for (size_t i = 0; i < r.atoms.Count(); i++) {
    queue.Push(r.atoms[i])->SetTag(-1);
  }
  queue.Push(NULL);
  index_t tv = 0;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    if (a == NULL) {
      if (queue.IsEmpty()) {
        break;
      }
      tv++;
      queue.Push(NULL);
      continue;
    }
    if (a->GetTag() != -1) {
      continue;
    }
    a->SetTag(tv);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (!st.atom->IsAvailable()) {
        continue;
      }
      if (st.atom->GetTag() == -1) {
        queue.Push(st.atom);
      }
    }
  }
  for (size_t i=0; i < r.atoms.Count(); i++) {
    for (size_t j=0; j < r.atoms[i]->AttachedSiteCount(); j++) {
      TCAtom &a = r.atoms[i]->GetAttachedAtom(j);
      if (!a.IsAvailable()) continue;
      if (a.GetTag() == 1 && a.GetType().z > 1) {
        ring::substituent &s = r.substituents.Add(
          new ring::substituent(r, *r.atoms[i]));
        s.atoms.Add(a);
        trace_substituent(s);
      }
      else if (a.GetTag() == -2) { // straight to the ring
        r.substituents.Add(
          new ring::substituent(r, *r.atoms[i])).ring_count++;
      }
    }
  }
  // build the substituent trees
  for (size_t i=0; i < r.substituents.Count(); i++) {
    ring::substituent &s = r.substituents[i];
    trace_tree(s.atoms, s.tree);
  }
}
//.............................................................................
fragments::tree_node &fragments::fragment::trace_tree(TCAtomPList &atoms,
  tree_node &tree)
{
  if (atoms.Count() == 1) {
    tree.trunk.Add(atoms[0]);
    return tree;
  }
  QuickSorter::Sort(atoms, ring::substituent::atom_cmp());
  trace_branch(atoms.GetLast(), tree);
  tree.trunk.ForEach(ACollectionItem::TagSetter(0));
  TPtrList<tree_node> branches;
  branches.Add(tree);
  for (size_t i=atoms.Count()-2; i != InvalidIndex; i--) {
    if (!atoms[i]->IsProcessed()) {
      tree_node &b = *(new tree_node);
      TCAtom *ta = trace_branch(atoms[i], b);
      const index_t t = ta->GetTag();
      b.trunk.ForEach(ACollectionItem::TagSetter(branches.Count()));
      branches[t]->branches.Add(b);
      branches.Add(b);
      ta->SetTag(t); //restore the tag to the original branch
    }
  }
  tree.sort();
  return tree;
}
//.............................................................................
ConstPtrList<TCAtom> fragments::fragment::trace_ring_d(TCAtom &ra) {
  TCAtomPList ring;
  TCAtom *a = &ra;
  ring.Add(a);
  index_t trim_tag = -1;
  while (a != 0) {
    bool added = false;
    TCAtom *b = a;
    size_t ring_c = ring.Count();
    for (size_t i = 0; i < b->AttachedSiteCount(); i++) {
      TCAtom &aa = b->GetAttachedAtom(i);
      if (aa.GetTag() == b->GetTag() - 1) {
        a = ring.Add(aa);
        added = true;
      }
      if (ring_c > 2 && &aa == ring[0]) {
        trim_tag = b->GetTag() - 1;
        a = 0;
        break;
      }
    }
    if (!added && trim_tag == -1) {
      ring.Clear();
      break;
      //throw TFunctionFailedException(__OlxSourceInfo, "assert");
    }
  }
  // trim
  while (!ring.IsEmpty() &&
    ring.GetLast()->GetTag() == trim_tag)
  {
    ring.SetCount(ring.Count() - 1);
  }
  return ring;
}
//.............................................................................
TTypeList<TCAtomPList>::const_list_type fragments::fragment::trace_ring_b(
  TQueue<TCAtom*> &queue, TCAtomPList &ring, bool flag)
{
  TTypeList<TCAtomPList> rv;
  typedef TQueue<TCAtom *> queue_t;
  atoms_.ForEach(
    TCAtom::FlagSetter(catom_flag_Processed, false));
  ring.ForEach(
    TCAtom::FlagSetter(catom_flag_Processed, true));

  TCAtom *la = NULL;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    TCAtomPList to_queue;
    bool stop = false;
    for (size_t i = 0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (!st.atom->IsAvailable()) continue;
      if (st.atom->GetTag() < a->GetTag()) {
        if (st.atom->IsProcessed()) {
          la = st.atom;
        }
        else {
          to_queue << st.atom;
        }
      }
      else if (st.atom->GetTag() == a->GetTag()) {
        if (st.atom->IsProcessed() && ring.Count() > 3) {
          stop = true;
        }
      }
    }
    if (stop) {
      break;
    }
    if (to_queue.Count() > 2) {
      for (size_t i = 0; i < to_queue.Count(); i++) {
        for (size_t j = i+1; j < to_queue.Count(); j++) {
          queue_t q(queue);
          TCAtomPList &r = rv.AddCopy(ring);
          q.Push(r.Add(to_queue[i]));
          q.Push(r.Add(to_queue[j]));
          TTypeList<TCAtomPList> nrings = trace_ring_b(q, r);
          if (r.IsEmpty()) {
            rv.Delete(rv.Count()-1);
          }
          for (size_t k = 0; k < nrings.Count(); k++) {
            rv.Add(nrings[k]);
          }
          nrings.ReleaseAll();
        }
      }
      ring.Clear();
      return rv;
    }
    else {
      if (!flag && to_queue.Count() > 1 && la == 0) {
        for (size_t i = 0; i < to_queue.Count(); i++) {
          queue_t q(queue);
          TCAtomPList &r = rv.AddCopy(ring);
          q.Push(r.Add(to_queue[i]));
          TTypeList<TCAtomPList> nrings = trace_ring_b(q, r);
          if (r.IsEmpty()) {
            rv.Delete(rv.Count()-1);
          }
          for (size_t k = 0; k < nrings.Count(); k++) {
            rv.Add(nrings[k]);
          }
          nrings.ReleaseAll();
        }
        ring.Clear();
        return rv;
      }
      for (size_t i = 0; i < to_queue.Count(); i++) {
        queue.Push(ring.Add(to_queue[i]))->SetProcessed(true);
      }
    }
    if (la != 0) {
      break;
    }
  }
  // trim
  if (la != NULL) {
    while (ring.Count() > 0 &&
      ring[ring.Count()-1]->GetTag() <= la->GetTag())
    {
      ring[ring.Count()-1]->SetProcessed(false);
      ring.SetCount(ring.Count()-1);
    }
    ring.Add(la)->SetProcessed(true);
  }
  return rv;
}
//.............................................................................
TTypeList<TCAtomPList>::const_list_type fragments::fragment::trace_ring_b(
  TCAtom &a)
{
  TTypeList<TCAtomPList> rings;
  TCAtomPList &ring = rings.AddNew();
  TQueue<TCAtom*> queue;
  queue.Push(ring.Add(a));
  for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
    TCAtom::Site &st = a.GetAttachedSite(i);
    if (!st.atom->IsAvailable()) {
      continue;
    }
    if (st.atom->GetTag() == a.GetTag()) {
      queue.Push(ring.Add(st.atom));
    }
  }
  TTypeList<TCAtomPList> nrings = trace_ring_b(queue, ring, true);
  if (ring.IsEmpty()) {
    rings.Delete(rings.Count()-1);
  }
  for (size_t i = 0; i < nrings.Count(); i++) {
    rings.Add(nrings[i]);
  }
  nrings.ReleaseAll();
  return rings;
}
//.............................................................................
void fragments::fragment::trace_substituent(ring::substituent &s) {
  atoms_.ForEach(
    TCAtom::FlagSetter(catom_flag_Processed, false));
  s.parent.atoms.ForEach(
    TCAtom::FlagSetter(catom_flag_Processed, true));
  for (size_t i=1; i < s.atoms.Count(); i++) { // hidden recursion
    if (s.atoms[i]->IsProcessed()) {
      continue;
    }
    for (size_t j=0; j < s.atoms[i]->AttachedSiteCount(); j++) {
      TCAtom &a = s.atoms[i]->GetAttachedAtom(j);
      if (a.IsProcessed() || !a.IsAvailable()) {
        continue;
      }
      if (a.GetTag() <= s.atoms[i]->GetTag()) {
        if (a.GetTag() == -2) {
          s.ring_count++;
        }
        continue;
      }
      s.atoms.Add(a);
    }
  }
}
//.............................................................................
TCAtom *fragments::fragment::trace_branch(TCAtom *a, tree_node &b) {
  while (true) {
    if (b.trunk.Count() > 1 && b.trunk.GetLast() == a) {
      throw TFunctionFailedException(__OlxSourceInfo, "internal error");
    }
    b.trunk.Add(a);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom &st = a->GetAttachedAtom(i);
      if (!st.IsAvailable()) {
        continue;
      }
      if (st.IsProcessed() || st.GetTag() == 0) {
        b.trunk.Add(st);
        b.trunk.ForEach(TCAtom::FlagSetter(catom_flag_Processed, true));
        return &st;
      }
      if (st.GetTag() < a->GetTag() && st.GetTag() > 0) {
        a = &st;
        break;
      }
    }
  }
}
//.............................................................................
fragments::tree_node fragments::fragment::build_tree() {
  tree_node b;
  return trace_tree(atoms_, b);
}
//.............................................................................
uint64_t fragments::fragment::calc_node_hash(
  TEGraphNode<uint64_t, TCAtom *> &nd) const
{
  mask_neighbours(-1, -2);
  TQueue<TCAtom *> aqueue;
  aqueue.Push(nd.GetObject());
  aqueue.Push(NULL);
  index_t v = 0;
  TArrayList<size_t> counts(16, olx_list_init::zero());
  while (!aqueue.IsEmpty()) {
    TCAtom *n = aqueue.Pop();
    if (n == NULL) {
      if (aqueue.IsEmpty()) {
        break;
      }
      v++;
      aqueue.Push(NULL);
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
    for (size_t i = 0; i < n->AttachedSiteCount(); i++) {
      TCAtom &aa = n->GetAttachedAtom(i);
      if (aa.GetTag() == -1) {
        aqueue.Push(&aa);
      }
    }
  }
  uint64_t rv = ((uint64_t)nd.GetObject()->GetType().z) << 56;
  rv |= ((uint64_t)v) << 48;
  int64_t h = 0;
  for (size_t i = counts.Count(); i != 0; i--) {
    h = (h << 4) | counts[i - 1];
  }
  rv |= (h & 0x0000ffffffffffffULL);
  return rv;
}
//.............................................................................
uint64_t fragments::fragment::mix_node_hash(
  TEGraphNode<uint64_t, TCAtom *>& node,
  const olxdict<TCAtom*, TEGraphNode<uint64_t, TCAtom *>*,
  TPointerComparator>& map)
{
  TArrayList<uint64_t> ids(node.Count());
  for (size_t i = 0; i < node.Count(); i++) {
    ids[i] = map.Get(node[i].GetObject())->GetData();
  }
  BubbleSorter::Sort(ids, TPrimitiveComparator());
  uint64_t id = 0;
  for (size_t i = 0; i < ids.Count(); i++) {
    if ((id & 1) == 1) {
      id |= (ids[i] << 16);
    }
    else {
      id ^= (ids[i] >> 16);
    }
  }
  uint64_t rv = (node.GetData() & 0xffff000000000000ULL);
  rv |= (id & 0x0000ffffffffffffULL);
  return rv;
}
//.............................................................................
olx_object_ptr<TEGraphNode<uint64_t, TCAtom *> >
fragments::fragment::build_graph() const
{
  typedef TEGraphNode<uint64_t, TCAtom *> node_t;
  // mask out atoms of this fragment
  mask_neighbours(atoms_, -1, -2);
  olx_object_ptr<node_t> root(new node_t(0, atoms_[0]));
  TQueue<TCAtom *> aqueue;
  aqueue.Push(root().GetObject());
  aqueue.Push(NULL);
  index_t v = 0;
  while (!aqueue.IsEmpty()) {
    TCAtom *n = aqueue.Pop();
    if (n == NULL) {
      if (aqueue.IsEmpty()) {
        break;
      }
      v++;
      aqueue.Push(NULL);
      continue;
    }
    if (n->GetTag() != -1) {
      continue;
    }
    n->SetTag(v);
    for (size_t i = 0; i < n->AttachedSiteCount(); i++) {
      TCAtom &aa = n->GetAttachedAtom(i);
      if (aa.GetTag() == -1) {
        aqueue.Push(&aa);
      }
    }
  }
  TQueue<node_t*> queue;
  queue.Push(&root());
  olxdict<TCAtom*, node_t*, TPointerComparator> map;
  map.Add(root().GetObject(), &root());
  while (!queue.IsEmpty()) {
    node_t *n = queue.Pop();
    for (size_t i = 0; i < n->GetObject()->AttachedSiteCount(); i++) {
      TCAtom &aa = n->GetObject()->GetAttachedAtom(i);
      if (aa.GetTag() < 0) {
        continue;
      }
      if (aa.GetTag() > n->GetObject()->GetTag()) {
        map.Add(&aa, queue.Push(&n->NewNode(0, &aa)));
      }
    }
  }
  calc_hashes(root());
  olxdict<node_t *, uint64_t, TPointerComparator> h2s;
  mix_hashes(root(), map, h2s);
  assign_hashes(root(), h2s);
  root().SetRoot(true);
  return root;
}
//.............................................................................
bool fragments::fragment::does_match(TCAtom &a, TCAtom &b, TCAtomPList &p) {
  if (a.GetType() != b.GetType() || a.IsRingAtom() != b.IsRingAtom()) {
    return false;
  }
  a.SetTag(4);
  b.SetTag(3);
  TEBitArray used(b.AttachedSiteCount());
  for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.GetTag() != 2) continue;
    bool matched = false;
    for (size_t j = 0; j < b.AttachedSiteCount(); j++) {
      TCAtom &ba = b.GetAttachedAtom(j);
      if (ba.GetTag() > 1 || ba.GetType() != aa.GetType() || used[j]) {
        continue;
      }
      if (does_match(aa, ba, p)) {
        matched = true;
        used.SetTrue(j);
        break;
      }
    }
    if (!matched) {
      a.SetTag(2);
      b.SetTag(0);
      return false;
    }
  }
  p.Add(b);
  return true;
}
//.............................................................................
TCAtomPList::const_list_type fragments::fragment::get_matching_set(
  TCAtom &root, const fragments::fragment &f)
{
  TCAtomPList rv;
  does_match(f[0], root, rv);
  if (rv.Count() != f.count()) {
    rv.Clear();
  }
  return rv;
}
//.............................................................................
ConstTypeList<fragments::ring> fragments::fragment::get_rings(
  const TCAtomPList &r_atoms)
{
  TTypeList<fragments::ring> rv;
  if (r_atoms.IsEmpty()) return rv;
  for (size_t i = 0; i < r_atoms.Count(); i++) {
    TTypeList<TCAtomPList> nrings = trace_ring_b(*r_atoms[i]);
    for (size_t j = 0; j < nrings.Count(); j++) {
      if (nrings[j].Count() > 2) {
        rv.AddNew(nrings[j]);
      }
    }
  }
  atoms_.ForEach(TCAtom::FlagSetter(catom_flag_Processed, false));
  // sort the ring according to the connectivity
  for (size_t i=0; i < rv.Count(); i++) {
    if (!rv[i].atoms.IsEmpty()) {
      // need for the case of overlapping rings
      rv[i].atoms.ForEach(TCAtom::FlagSetter(catom_flag_Processed, true));
      try {
        rv[i] = ring_sorter(rv[i].atoms);
      }
      catch (const TExceptionBase &e) {
        rv.NullItem(i);
        TBasicApp::NewLogEntry(logInfo) << e.GetException()->GetFullMessage();
      }
    }
  }
  rv.Pack();
  return rv;
}
//.............................................................................
ConstPtrList<TCAtom> fragments::fragment::ring_sorter(const TCAtomPList &r) {
  if (r.Count() < 3) {
    return new TCAtomPList(r);
  }
  TCAtomPList res(r.Count());
  res.Set(0, r[0])->SetProcessed(false);
  for (size_t i=0; i < r.Count()-1; i++) {
    bool set=false;
    for (size_t j=0; j < res[i]->AttachedSiteCount(); j++) {
      TCAtom &a = res[i]->GetAttachedAtom(j);
      if (a.IsProcessed()) {
        res.Set(i+1, a)->SetProcessed(false);
        set = true;
        break;
      }
    }
    if (!set) {
      TBasicApp::NewLogEntry() << alg::label(r, '-');
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("ring ") << alg::label(r));
    }
  }
  return res;
}
//.............................................................................
//.............................................................................
void fragments::expand_node(TCAtom &a, TCAtomPList &atoms) {
  atoms.Add(a)->SetTag(1);
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (!aa.IsAvailable() || aa.GetTag() != 0) {
      continue;
    }
    expand_node(aa, atoms);
  }
}
//.............................................................................
ConstTypeList<fragments::fragment> fragments::extract(TAsymmUnit &au) {
  TTypeList<fragments::fragment> rv;
  TCAtomPList &atoms = au.GetAtoms();
  atoms.ForEach(ACollectionItem::TagSetter(0));
  size_t cnt = 0;
  for (size_t i=0; i < atoms.Count(); i++) {
    if (!atoms[i]->IsAvailable()) continue;
    if (atoms[i]->GetTag() == 0) {
      TCAtomPList catoms;
      expand_node(*atoms[i], catoms);
      rv.AddNew().set_atoms(catoms);
      if ((cnt+=catoms.Count()) == atoms.Count())
        break;
    }
  }
  return rv;
}
//.............................................................................
struct CAtomGraphAnalyser  {
  TEGraphNode<uint64_t, TCAtom *> &RootA, &RootB;
  size_t CallsCount;
  bool Invert;
  double permutations;
  time_t start_time;
  struct TagSetter {
    size_t calls;
    TagSetter() : calls(0) {}
    bool OnItem(const TEGraphNode<uint64_t, TCAtom *> &v) {
      calls++;
      v.GetObject()->SetTag(1);
      return true;
    }
  };
  CAtomGraphAnalyser(TEGraphNode<uint64_t, TCAtom *> &rootA,
    TEGraphNode<uint64_t, TCAtom *> &rootB)
    : RootA(rootA), RootB(rootB), CallsCount(0), Invert(false)
  {}

  static TArrayList<align::pair>& AtomsToPairs(
    const TTypeList<olx_pair_t<TCAtom*, TCAtom*> >& atoms,
    TArrayList<align::pair>& pairs,
    bool invert)
  {
    if (atoms.IsEmpty()) return pairs;
    pairs.SetCount(atoms.Count());
    const TAsymmUnit& au1 = *atoms[0].GetA()->GetParent();
    const TAsymmUnit& au2 = *atoms[0].GetB()->GetParent();
    for (size_t i = 0; i < atoms.Count(); i++) {
      vec3d v1 = atoms[i].GetA()->ccrd(),
        v2 = atoms[i].GetB()->ccrd();
      if (invert)  v2 *= -1;
      pairs[i].a.value = au1.CellToCartesian(v1);
      pairs[i].a.weight = 1;
      pairs[i].b.value = au2.CellToCartesian(v2);
      pairs[i].b.weight = 1;
    }
    return pairs;
  }
  //..............................................................................
  static align::out GetAlignmentInfo(
    const TTypeList<olx_pair_t<TCAtom*, TCAtom*> > &atoms,
    bool invert)
  {
    TArrayList<align::pair> pairs;
    return align::FindAlignmentQuaternions(AtomsToPairs(atoms,
      pairs, invert));
  }

  static TTypeList< olx_pair_t<TCAtom *, TCAtom *> >::const_list_type
    CollectResult(TEGraphNode<uint64_t, TCAtom*> &a,
    TEGraphNode<uint64_t, TCAtom*> &b)
  {
    TTypeList< olx_pair_t<TCAtom *, TCAtom *> > rv;
    TagSetter tag_s;
    a.Traverser.Traverse(a, tag_s);
    rv.SetCapacity(tag_s.calls);
    b.Traverser.Traverse(b, tag_s);
    CollectResult(a, b, rv);
     return rv;
  }

  static void CollectResult(
    TEGraphNode<uint64_t, TCAtom*> &subRoot,
    TEGraphNode<uint64_t, TCAtom*> &Root,
    TTypeList< olx_pair_t<TCAtom *, TCAtom *> > &res)
  {
    if (!subRoot.ShallowEquals(Root))  return;
    res.AddNew(subRoot.GetObject(), Root.GetObject());
    subRoot.GetObject()->SetTag(0);
    Root.GetObject()->SetTag(0);
    for (size_t i = 0; i < subRoot.Count(); i++) {
      if (subRoot[i].GetObject()->GetTag() != 0 &&
        Root[i].GetObject()->GetTag() != 0)
      {
        CollectResult(subRoot[i], Root[i], res);
      }
    }
  }

  void OnStart(double perms) {
    permutations = perms;
    start_time = TETime::msNow();
  }

  double CalcRMS() {
    TTypeList< olx_pair_t<TCAtom*, TCAtom*> > matchedAtoms;
    TagSetter tag_s;
    RootA.Traverser.Traverse(RootA, tag_s);
    matchedAtoms.SetCapacity(tag_s.calls);
    RootB.Traverser.Traverse(RootB, tag_s);
    CollectResult(RootA, RootB, matchedAtoms);
    matchedAtoms.Pack();
    CallsCount++;
    align::out ao = GetAlignmentInfo(matchedAtoms, Invert);
    return ao.rmsd[0];
  }

  double CalcRMS(const TEGraphNode<uint64_t, TCAtom *> &src,
    const TEGraphNode<uint64_t, TCAtom *> &dest)
  {
    if ((TETime::msNow() - start_time) >= 60000) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("the procedure was terminated. Number of permutations: ") <<
        permutations << ", number of calls: " << CallsCount
        );
    }
    return CalcRMS();
  }

  void OnFinish() {}
};

ConstTypeList<fragments::fragment> fragments::extract(const TCAtomPList &aua,
  const fragment &f_)
{
  if (f_.is_disjoint()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "set of atoms is disconnected");
  }

  TEBitArray arr(aua.Count());
  // mark ring atoms
  {
    fragment x(aua);
    for (size_t i = 0; i < aua.Count(); i++) {
      TCAtom &a = *aua[i];
      if (a.GetType().z < 0 && a.IsAvailable()) {
        a.SetDeleted(true);
        arr.SetTrue(i);
      }
      a.SetTag(-1);
    }
    
    for (size_t i=0; i < aua.Count(); i++) {
      if (aua[i]->IsAvailable() && aua[i]->GetTag() == -1) {
        TCAtomPList r_atoms;
        x.breadth_first_tags(i, &r_atoms);
        for (size_t j = 0; j < r_atoms.Count(); j++) {
          TTypeList<TCAtomPList> rings = x.trace_ring_b(*r_atoms[j]);
          for (size_t k = 0; k < rings.Count(); k++) {
            rings[k].ForEach(TCAtom::FlagSetter(catom_flag_RingAtom, true));
          }
        }
      }
    }
    //for (size_t i = 0; i < aua.Count(); i++) {
    //  aua[i]->SetLabel(aua[i]->GetTag(), false);
    //  if (aua[i]->IsRingAtom()) {
    //    aua[i]->SetLabel(aua[i]->GetLabel() + 'r', false);
    //  }
    //}
  }
  fragment f = f_;
  fragment::order_list(f.atoms());
  f.mask_neighbours(2, 1);
  const size_t bc = fragment::get_neighbour_count(f[0], 2);
  olx_object_ptr<fragment::node_t> fr = f.build_graph();
  aua.ForEach(ACollectionItem::TagSetter(0));
  TCAtomPList of_interest;
  for (size_t i = 0; i < aua.Count(); i++) {
    TCAtom &a = *aua[i];
    if (a.IsDeleted() || a.GetType() != f[0].GetType() ||
      fragment::get_neighbour_count(a, 0) < bc)
    {
      continue;
    }
    of_interest.Add(a);
  }
  TTypeList<TCAtomPList> matching;
  for (size_t i = 0; i < of_interest.Count(); i++) {
    aua.ForEach(ACollectionItem::TagSetter(0));
    f.mask_neighbours(2, 1);
    for (size_t j = 0; j < matching.Count(); j++) {
      matching[j].ForEach(ACollectionItem::TagSetter(4));
    }
    if (of_interest[i]->GetTag() != 0) {
      continue;
    }
    TCAtomPList &matching_set = matching.Add(
      fragment::get_matching_set(*of_interest[i], f).Release());
    if (!matching_set.IsEmpty()) {
      olx_reverse(matching_set);
      fragment f1(matching_set);
      olx_object_ptr<fragment::node_t> fr1 = f1.build_graph();
      CAtomGraphAnalyser ga(fr(), fr1());
      ga.Invert = false;
      if (!fr1().FullMatchEx(fr, ga)) { // should not happen
        matching.Delete(matching.Count() - 1);
      }
      else {
        TTypeList<olx_pair_t<TCAtom *, TCAtom *> > m =
          ga.CollectResult(fr(), fr1());
        if (m.Count() != f_.count()) {
          throw TFunctionFailedException(__OlxSourceInfo, "assert");
        }
        f_.atoms().ForEach(ACollectionItem::IndexTagSetter());
        
        for (size_t j = 0; j < m.Count(); j++) {
          matching_set[m[j].a->GetTag()] = m[j].b;
        }
      }
    }
    else {
      matching.Delete(matching.Count() - 1);
    }
  }
  TTypeList<fragments::fragment> rv;
  for (size_t i = 0; i < matching.Count(); i++) {
    rv.Add(new fragment(matching[i]));
  }

  for (size_t i = 0; i < arr.Count(); i++) {
    if (arr[i]) {
      aua[i]->SetDeleted(false);
    }
  }
  return rv;
}
//.............................................................................
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
      if (peak_ranges[i].get_mean() < 1.5 && atoms.Count() > 0.75*mn ) break;
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
const cm_Element &Analysis::check_proposed_element(
    TCAtom &a, const cm_Element &e, ElementPList *set)
{
  if ( !alg::check_connectivity(a, e) || !alg::check_geometry(a, e) )
    return a.GetType();
  if (XElementLib::IsGroup8(e)) {
    cm_Element *pe = XElementLib::FindByZ(e.z-1);
    if (pe != NULL)
      return *pe;
  }
  return e;
}
//.............................................................................
const cm_Element &Analysis::check_atom_type(TSAtom &a) {
  static sorted::ObjectPrimitive<short> types;
  if (types.IsEmpty()) {
    types.Add(iCarbonZ);
    types.Add(iOxygenZ);
    types.Add(iFluorineZ);
    types.Add(iChlorineZ);
    types.Add(iSulphurZ);
    types.Add(33); // arsenic
    types.Add(iBromineZ);
  }
  if (types.IndexOf(a.GetType().z) == InvalidIndex)
    return a.GetType();

  TUnitCell &uc = a.CAtom().GetParent()->GetLattice().GetUnitCell();
  TAtomEnvi ae;
  uc.GetAtomEnviList(a, ae);
  size_t metal_c = 0;
  for (size_t i=0; i < ae.Count(); i++) {
    if (XElementLib::IsMetal(ae.GetCAtom(i).GetType()))
      metal_c++;
  }
  size_t non_metal_c = ae.Count() - metal_c;
  if (a.GetType() == iOxygenZ) {
    if (non_metal_c > 2)
      return XElementLib::GetByIndex(iNitrogenIndex);
  }
  else if (a.GetType() == iFluorineZ) {
    if (non_metal_c > 2)
      return XElementLib::GetByIndex(iNitrogenIndex);
    if (non_metal_c > 1)
      return XElementLib::GetByIndex(iOxygenIndex);
  }
  else if (a.GetType() == iChlorineZ) {
    if (non_metal_c <= 1) return a.GetType();
    if (metal_c != 0) {
      if (ae.Count() == 4) // PR3->M
        return XElementLib::GetByIndex(iPhosphorusIndex);
      if (ae.Count() > 2) //SR2->M
        return XElementLib::GetByIndex(iSulphurIndex);
    }
    if (ae.Count() == 3) // PR3
      return XElementLib::GetByIndex(iPhosphorusIndex);
    return XElementLib::GetByIndex(iSulphurIndex);
  }
  else if (a.GetType() == 33) { // arsenic
    if (non_metal_c <= 1)
      return *XElementLib::FindByZ(iBromineZ);
  }
  else if (a.GetType() == iSulphurZ) {
    if (non_metal_c <= 1)
      return XElementLib::GetByIndex(iChlorineIndex);
    else if (ae.Count() > 6)  // reset - bad conenctivity
      return XElementLib::GetByIndex(iCarbonIndex);
  }
  else if (a.GetType() == iBromineZ) {
    if (non_metal_c > 1)
      return *XElementLib::FindByZ(33);
  }
  else if (a.GetType() == iCarbonZ) {
    if (ae.Count() == 0)
      return XElementLib::GetByIndex(iOxygenIndex);
  }
  return a.GetType();
}
//.............................................................................
void Analysis::funFindScale(const TStrObjList& Params, TMacroData& E) {
  bool apply = Params.IsEmpty() ? false : Params[0].ToBool();
  TLattice &latt = TXApp::GetInstance().XFile().GetLattice();
  double scale = find_scale(latt);
  if (scale > 0) {
    for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
      TSAtom &a = latt.GetObjects().atoms[i];
      if (a.GetType() == iQPeakZ && apply) {
        int z = olx_round(a.CAtom().GetQPeak()*scale);
        cm_Element *tp = XElementLib::FindByZ(z),
          *tp1 = NULL;
        // find previous halogen vs noble gas or alkaline metal
        if (tp != NULL) {
          if (XElementLib::IsGroup8(*tp) ||
            XElementLib::IsGroup1(*tp) ||
            XElementLib::IsGroup2(*tp))
          {
            tp1 = XElementLib::PrevGroup(7, tp);
          }
          a.CAtom().SetType(tp1 == NULL ? *tp : *tp1);
        }
      }
    }
  }
  E.SetRetVal(scale);
}
//.............................................................................

TLibrary *Analysis::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name);
  lib->Register(
    new TStaticFunction(&Analysis::funTrim, "Trim", fpNone | fpOne,
    "Trims the size of the assymetric unit according to the 18 A^3 rule."
    "Returns true if any atoms were deleted")
    );
  lib->Register(
    new TStaticFunction(&Analysis::funFindScale, "Scale", fpNone | fpOne,
    "Scales the Q-peaks according to found fragments."
    "Returns the scale or 0")
    );
  lib->Register(
    new TStaticFunction(&Analysis::funAnaluseUeq, "AnalyseUeq", fpNone | fpOne,
    ""
    "")
    );
  return lib;
}
