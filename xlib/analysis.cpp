#include "analysis.h"
#include "equeue.h"

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
olxstr alg::formula(const TCAtomPList &atoms, double mult) {
  ElementPList elements;
  ContentList cl;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const cm_Element& elm = atoms[i]->GetType();
    if (atoms[i]->IsDeleted() || elm == iQPeakZ) continue;
    const size_t ind = elements.IndexOf(elm);
    if (ind == InvalidIndex)  {
      cl.AddNew(elm, atoms[i]->GetOccu()*mult);
      elements.Add(elm);
    }
    else
      cl[ind] += atoms[i]->GetOccu()*mult;
  }
  XElementLib::SortContentList(cl);
  olxstr rv;
  for (size_t i=0; i < cl.Count(); i++)  {
    rv << cl[i].element.symbol;
    if( olx_abs(cl[i].count-1.0) > 1e-3 )
      rv << olxstr::FormatFloat(3, cl[i].count).TrimFloat();
    if( (i+1) < cl.Count() )
      rv << ' ';
  }
  return rv;
}
//.............................................................................
olxstr alg::label(const TCAtomPList &atoms) {
  olxstr rv;
  for( size_t i=0; i < atoms.Count(); i++ )
    rv << atoms[i]->GetLabel();
  return rv;
}
//.............................................................................
const cm_Element &alg::find_heaviest(const TCAtomPList &atoms) {
  if (atoms.IsEmpty())
    return XElementLib::GetByIndex(iQPeakIndex);
  size_t ind=0;
  for (size_t i=1; i < atoms.Count(); i++) {
    if (atoms[i]->GetType().z > atoms[ind]->GetType().z)
      ind = i;
  }
  return atoms[ind]->GetType();
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
  if (_peaks.IsEmpty()) return ranges;
  TCAtomPList peaks = _peaks;
  QuickSorter::SortSF(peaks, peak_sort);
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
void fragments::tree_node::reverse() {
  olx_reverse(trunk);
  trunk.ForEach(ACollectionItem::IndexTagSetter<>());
  sort();
}
//.............................................................................
//.............................................................................
bool fragments::ring::is_leq(const ring &r) const {
  atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, true));
  for (size_t i=0; i < atoms.Count(); i++)
    if (!atoms[i]->IsProcessed())
      return false;
  return true;
}
//.............................................................................
bool fragments::ring::is_fused_with(const ring &r) const {
  atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, true));
  size_t prev_ind=InvalidIndex;
  for (size_t i=0; i < atoms.Count(); i++) {
    if (atoms[i]->IsProcessed()) {
      if (prev_ind != InvalidIndex) {
        if (i == prev_ind+1 || (prev_ind==0 && i==atoms.Count()-1))
          return true;
      }
      prev_ind = i;
    }
  }
  return false;
}
//.............................................................................
bool fragments::ring::merge(ring &r) {
  atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, false));
  r.atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, true));
  TSizeList tii, tai;
  for (size_t i=0; i < atoms.Count(); i++)
    if (atoms[i]->IsProcessed()) tii << i;
  if (tii.Count() != 2) return false;

  atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, false));
  for (size_t i=0; i < r.atoms.Count(); i++)
    if (!r.atoms[i]->IsProcessed()) tai << i;

  if (tii[0] == 0 && tii[1] == atoms.Count()-1)
    ;//atoms.ShiftR(1);
  else if (tii[0]+1 == tii[1])
    atoms.ShiftL(tii[0]+1);
  else
    return false;

  if (tai[0] == 0 && tai[1] == r.atoms.Count()-1)
    r.atoms.ShiftR(1);
  else if (tai[0]+1 == tai[1])
    r.atoms.ShiftL(tai[0]);
  else
    return false;
  if (atoms[0] != r.atoms[1])
    atoms.AddList(r.atoms.SubListFrom(2));
  else
    atoms.AddList(ReverseList::MakeConst(r.atoms.SubListFrom(2)));
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
  if (d != 0) return d;
  d = olx_cmp(substituents.Count(), r.substituents.Count());
  if (d != 0) return d;
  if (substituents.IsEmpty()) return 0;
  return substituents[0].Compare(r.substituents[0]);
}
//.............................................................................
//.............................................................................
int fragments::ring::substituent::Compare(
  const fragments::ring::substituent &s) const
{
  int r = olx_cmp(ring_count, s.ring_count);
  if (r != 0) return r;
  r = olx_cmp(alg::find_heaviest(atoms).z, alg::find_heaviest(s.atoms).z);
  if (r != 0) return r;
  return olx_cmp(atoms.Count(), s.atoms.Count());
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
  vec3d_list crds(atoms_.Count(), true);
  if (atoms_.IsEmpty()) return crds;
  atoms_.ForEach(ACollectionItem::IndexTagSetter<>());
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
        if (uniq)
          crds.AddNew(v);
      }
    }
  }
  return crds;
}
//.............................................................................
void fragments::fragment::init_generators() {
  if (atoms_.IsEmpty()) return;
  generators.AddList(
    atoms_[0]->GetParent()->GetLattice().GetFragmentGrowMatrices(atoms_,
      true));
}
//.............................................................................
bool fragments::fragment::is_polymeric() const {
  if (generators.Count() < 3) return false;
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
          else
            return true;
        }
      }
      if (uniq)
        set.Add(m);
    }
  }
  return false;
}
//.............................................................................
bool fragments::fragment::is_regular() const {
  size_t ci = find_central_index();
  if (ci == InvalidIndex) return false;
  vec3d_list crds = build_coordinates();
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
  // some spherical arrangement test
  else if (crds.Count() == 5 || crds.Count() == 7) {
    TSizeList indices(crds.Count(), list_init::index());
    indices.Remove(ci);
    vec3d cnt = olx_mean(indices, ConstIndexAccessor<vec3d, vec3d_list>(crds));
    // check central atom is nearby the geometrical center
    if (cnt.DistanceTo(crds[ci]) > 0.1) return false;
    if (crds.Count() == 5) {
      for (size_t i=0; i < indices.Count(); i++) {
        vec3d pv = (crds[indices[i]]-crds[ci]);
        for (size_t j=i+1; j < indices.Count(); j++) {
          double ca = pv.CAngle((crds[indices[j]]-crds[ci]));
          if ( olx_abs(ca+1./3) > 0.16) // cos(THA) = -1./3, +- 10
            return false;
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
          if ( olx_abs(ca) < 0.17) // 90 +- 10
            a90_cnt++;
          else if ((ca+1) < 0.03) // 180 +- 10
            a180_cnt++;
          else
            return false;
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
    if (sc >= atoms_.Count()-1 && sc != 1)
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
  if (crds.Count() <= 3) { 
    return true;
  }
  else { 
    TArrayList<AnAssociation2<vec3d, double> > points(crds.Count());
    for (size_t i=0; i < crds.Count(); i++) {
      points[i].A() = crds[i];
      points[i].B() = 1;
    }
    vec3d center;
    rmsd = TSPlane::CalcPlane(points, n, center, plane_best);
    return rmsd < 0.05;
  }
}
//.............................................................................
void fragments::fragment::breadth_first_tags(size_t start,
  TCAtomPList *ring_atoms)
{
  if (atoms_.IsEmpty()) return;
  atoms_.ForEach(ACollectionItem::TagSetter<>(-1));
  TQueue<TCAtom*> queue;
  if (start >= atoms_.Count())
    start = 0;
  queue.Push(atoms_[start]);
  queue.Push(NULL);
  index_t tv = 0;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    if (a == NULL) {
      if (queue.IsEmpty()) break;
      tv++;
      queue.Push(NULL);
      continue;
    }
    if (a->GetTag() != -1 ) {
      if (a->GetTag() >= tv && ring_atoms != NULL)
        ring_atoms->Add(a);
      continue;
    }
    a->SetTag(tv);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (st.atom->GetTag() == -1)
        queue.Push(st.atom);
      else if (st.atom->GetTag() >= tv && ring_atoms != NULL)
        ring_atoms->Add(st.atom);
    }
  }
}
//.............................................................................
void fragments::fragment::init_rings(TTypeList<fragments::ring> &rings) {
  for (size_t i=0; i < rings.Count(); i++) {
    init_ring(i, rings);
    if (!rings[i].substituents.IsEmpty()) {
      QuickSorter::Sort(rings[i].substituents);
      // align atoms to start from the 'heaviest' substituent
      rings[i].atoms.ForEach(ACollectionItem::IndexTagSetter<>());
      rings[i].atoms.ShiftL(rings[i].substituents.GetLast().atoms[0]->GetTag());
    }
  }
  QuickSorter::Sort(rings);
}
//.............................................................................
void fragments::fragment::init_ring(size_t i, TTypeList<ring> &rings) {
  if (atoms_.IsEmpty()) return;
  atoms_.ForEach(ACollectionItem::TagSetter<>(-1));
  ring &r = rings[i];
  TQueue<TCAtom*> queue;
  for (size_t i=0; i < rings.Count(); i++)
    rings[i].atoms.ForEach(ACollectionItem::TagSetter<>(-2));
  for (size_t i=0; i < r.atoms.Count(); i++)
    queue.Push(r.atoms[i])->SetTag(-1);
  queue.Push(NULL);
  index_t tv = 0;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    if (a == NULL) {
      if (queue.IsEmpty()) break;
      tv++;
      queue.Push(NULL);
      continue;
    }
    if (a->GetTag() != -1 ) continue;
    a->SetTag(tv);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (st.atom->GetTag() == -1)
        queue.Push(st.atom);
    }
  }
  for (size_t i=0; i < r.atoms.Count(); i++) {
    for (size_t j=0; j < r.atoms[i]->AttachedSiteCount(); j++) {
      TCAtom &a = r.atoms[i]->GetAttachedAtom(j);
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
  QuickSorter::Sort(atoms, ring::substituent::atom_cmp());
  if (atoms.Count() == 1) {
    tree.trunk.Add(atoms[0]);
    return tree;
  }
  trace_branch(atoms.GetLast(), tree);
  tree.trunk.ForEach(ACollectionItem::TagSetter<>(0));
  TPtrList<tree_node> branches;
  branches.Add(tree);
  for (size_t i=atoms.Count()-2; i != InvalidIndex; i--) {
    if (!atoms[i]->IsProcessed()) {
      tree_node &b = *(new tree_node);
      TCAtom *ta = trace_branch(atoms[i], b);
      const index_t t = ta->GetTag();
      b.trunk.ForEach(ACollectionItem::TagSetter<>(branches.Count()));
      branches[t]->branches.Add(b);
      branches.Add(b);
      ta->SetTag(t); //restore the tag to the original branch
    }
  }
  tree.sort();
  return tree;
}
//.............................................................................
ConstPtrList<TCAtom> fragments::fragment::trace_ring(TCAtom &a) {
  TCAtomPList ring;
  TQueue<TCAtom*> queue;
  atoms_.ForEach(
    TCAtom::FlagSetter<>(catom_flag_Processed, false));
  queue.Push(ring.Add(a))->SetProcessed(true);
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom::Site &st = a.GetAttachedSite(i);
    if (st.atom->GetTag() == a.GetTag())
      queue.Push(ring.Add(st.atom))->SetProcessed(true);
  }
  TCAtom *la = NULL;
  while (!queue.IsEmpty()) {
    TCAtom *a = queue.Pop();
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom::Site &st = a->GetAttachedSite(i);
      if (st.atom->GetTag() < a->GetTag()) {
        if (st.atom->IsProcessed())
          la = st.atom;
        else
          queue.Push(ring.Add(st.atom))->SetProcessed(true);
      }
    }
    if (la != NULL) break;
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
  return ring;
}
//.............................................................................
void fragments::fragment::trace_substituent(ring::substituent &s) {
  atoms_.ForEach(
    TCAtom::FlagSetter<>(catom_flag_Processed, false));
  s.parent.atoms.ForEach(
    TCAtom::FlagSetter<>(catom_flag_Processed, true));
  for (size_t i=1; i < s.atoms.Count(); i++) { // hidden recursion
    if (s.atoms[i]->IsProcessed()) continue;
    for (size_t j=0; j < s.atoms[i]->AttachedSiteCount(); j++) {
      TCAtom &a = s.atoms[i]->GetAttachedAtom(j);
      if (a.IsProcessed()) continue;
      if (a.GetTag() <= s.atoms[i]->GetTag()) {
        if (a.GetTag() == -2)
          s.ring_count++;
        continue;
      }
      s.atoms.Add(a);
    }
  }
}
//.............................................................................
TCAtom *fragments::fragment::trace_branch(TCAtom *a, tree_node &b) {
  while (true) {
    if (b.trunk.Count() > 1 && b.trunk.GetLast() == a)
      throw TFunctionFailedException(__OlxSourceInfo, "internal error");
    b.trunk.Add(a);
    for (size_t i=0; i < a->AttachedSiteCount(); i++) {
      TCAtom &st = a->GetAttachedAtom(i);
      if (st.IsProcessed() || st.GetTag() == 0) {
        b.trunk.Add(st);
        b.trunk.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, true));
        return &st;
      }
      if (st.GetTag() < a->GetTag()) {
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
ConstTypeList<fragments::ring> fragments::fragment::get_rings(
  const TCAtomPList &r_atoms)
{
  TTypeList<fragments::ring> rv;
  if (r_atoms.IsEmpty()) return rv;
  for (size_t i=0; i < r_atoms.Count(); i++)
    rv.AddNew(trace_ring(*r_atoms[i]));
  atoms_.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, false));
  // sort the ring according to the connectivity
  for (size_t i=0; i < rv.Count(); i++) {
    if (!rv[i].atoms.IsEmpty()) {
      // need for the case of overlapping rings
      rv[i].atoms.ForEach(TCAtom::FlagSetter<>(catom_flag_Processed, true));
      try {
        rv[i] = ring_sorter(rv[i].atoms);
      }
      catch (const TExceptionBase &e) {
        throw TFunctionFailedException(__OlxSourceInfo, e);
      }
    }
  }
  return rv;
}
//.............................................................................
ConstPtrList<TCAtom> fragments::fragment::ring_sorter(const TCAtomPList &r) {
  if (r.Count() < 3) return new TCAtomPList(r);
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
    if (!aa.IsAvailable() || aa.GetTag() != 0) continue;
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
    if (!atoms[i]->IsAvailable()) continue;
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
const cm_Element &Analysis::check_proposed_element(
    TCAtom &a, const cm_Element &e)
{
  TTypeList<AnAssociation2<const TCAtom*, vec3d> > res;
  TUnitCell &uc = a.GetParent()->GetLattice().GetUnitCell();
  uc.FindInRangeAC(a.ccrd(), e.r_vdw, res); 
  vec3d center = a.GetParent()->Orthogonalise(a.ccrd());
  QuickSorter::Sort(res, TUnitCell::AC_Sort(center));
  size_t cnt = 0;
  for (size_t i=1; i < res.Count(); i++) {
    double d = center.DistanceTo(res[i].GetB());
    if (d < e.r_vdw)
      cnt++;
  }
  if (XElementLib::IsGroup8(e)) {
    cm_Element *pe = XElementLib::FindByZ(e.z-1);
    if (pe != NULL)
      return *pe;
  }
  return e;
}
//.............................................................................
//.............................................................................
