#include "bonds_eq_search.h"
#include "unitcell.h"
#include "estopwatch.h"

namespace {
  /* beyond this the search can find nothing: every outcome needs
  d < r(a1)+r(a2)+delta, so this bounds them all. Not a tolerance
  */
  double symm_eq_cutoff(const TCAtomPList& atoms, double delta, double deltaI) {
    double r = 0;
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->IsDeleted()) {
        continue;
      }
      r = olx_max(r, atoms[i]->GetConnInfo().r);
    }
    return 2 * r + olx_max(delta, deltaI);
  }
  /* the shortest any -1..1 shift but the tested one can reach. The reduced
  difference is within [-0.5, 0.5], so another shift is at least half a cell
  wide along some axis. Perpendicular width V/|bxc|, not the axis length,
  which over-estimates in a skewed cell
  */
  double min_shift_distance(const TAsymmUnit& au) {
    const mat3d& c2c = au.GetCellToCartesian();
    const vec3d ca(c2c[0]), cb(c2c[1]), cc(c2c[2]);
    const double vol = olx_abs(ca.DotProd(cb.XProdVec(cc)));
    const double bc = cb.XProdVec(cc).Length(),
      ac = ca.XProdVec(cc).Length(),
      ab = ca.XProdVec(cb).Length();
    if (vol < 1e-6 || bc < 1e-6 || ac < 1e-6 || ab < 1e-6) {
      return 0;
    }
    return 0.5 * olx_min(vol / bc, olx_min(vol / ac, vol / ab));
  }
  /* fills out[i], ascending, with the atoms after i within cutoff under any
  matrix. Bins are at least cutoff wide perpendicular to each face - the axis
  length bins too coarsely in a skewed cell - so a close pair is always within
  one bin and the 27 around it are the whole search. Made unique, since an atom
  can be a neighbour under several matrices
  */
  void build_symm_eq_neighbours(const TAsymmUnit& au,
    const TCAtomPList& atoms, const smatd_list& matrices, double cutoff,
    bool all_shifts,
    TArrayList<TSizeList>& out)
  {
    const size_t ac = atoms.Count(), mc = matrices.Count();
    out.SetCount(ac);
    const mat3d& c2c = au.GetCellToCartesian();
    const vec3d ca(c2c[0]), cb(c2c[1]), cc(c2c[2]);
    const double vol = olx_abs(ca.DotProd(cb.XProdVec(cc)));
    size_t na = 1, nb = 1, nc = 1;
    if (vol > 1e-6 && cutoff > 1e-3) {
      na = olx_max((size_t)1, (size_t)(vol / cb.XProdVec(cc).Length() / cutoff));
      nb = olx_max((size_t)1, (size_t)(vol / ca.XProdVec(cc).Length() / cutoff));
      nc = olx_max((size_t)1, (size_t)(vol / ca.XProdVec(cb).Length() / cutoff));
      // fewer bins costs speed, never correctness
      while (na * nb * nc > 4000000) {
        na = olx_max((size_t)1, na / 2);
        nb = olx_max((size_t)1, nb / 2);
        nc = olx_max((size_t)1, nc / 2);
        if (na == 1 && nb == 1 && nc == 1) {
          break;
        }
      }
    }
    TArrayList<TSizeList> bins(na * nb * nc);
    TArrayList<vec3d> img_crd(ac * mc);
    TSizeList img_atom(ac * mc);
    size_t ic = 0;
    for (size_t i = 0; i < ac; i++) {
      for (size_t m = 0; m < mc; m++) {
        vec3d f = matrices[m] * atoms[i]->ccrd();
        f -= f.Floor<int>();
        img_crd[ic] = f;
        img_atom[ic] = i;
        const size_t ba = olx_min(na - 1, (size_t)(f[0] * na)),
          bb = olx_min(nb - 1, (size_t)(f[1] * nb)),
          bc = olx_min(nc - 1, (size_t)(f[2] * nc));
        bins[(ba * nb + bb) * nc + bc].Add(ic++);
      }
    }
    const double qcut = cutoff * cutoff;
    TSizeList seen(ac, olx_list_init::value(InvalidIndex));
    for (size_t i = 0; i < ac; i++) {
      vec3d f = atoms[i]->ccrd();
      f -= f.Floor<int>();
      const size_t ba = olx_min(na - 1, (size_t)(f[0] * na)),
        bb = olx_min(nb - 1, (size_t)(f[1] * nb)),
        bc = olx_min(nc - 1, (size_t)(f[2] * nc));
      for (int da = -1; da <= 1; da++) {
        for (int db = -1; db <= 1; db++) {
          for (int dc = -1; dc <= 1; dc++) {
            /* signed: bin 0's neighbour is the last one, and in size_t that
            underflows and loses the pairs across a cell face
            */
            const size_t ja = (size_t)(((int)ba + da + (int)na) % (int)na),
              jb = (size_t)(((int)bb + db + (int)nb) % (int)nb),
              jc = (size_t)(((int)bc + dc + (int)nc) % (int)nc);
            const TSizeList& bin = bins[(ja * nb + jb) * nc + jc];
            for (size_t k = 0; k < bin.Count(); k++) {
              const size_t im = bin[k], j = img_atom[im];
              // the caller only looks forward, and once per atom
              if (j <= i || seen[j] == i) {
                continue;
              }
              vec3d v = atoms[i]->ccrd() - img_crd[im];
              v -= v.Round<int>();
              bool close = (v * c2c).QLength() <= qcut;
              // small cell: a neighbouring translation of this image can be the close one
              for (int s = 0; !close && all_shifts && s < 27; s++) {
                if (s == 13) {
                  continue;  // (0,0,0), tested above
                }
                const vec3d t(s / 9 - 1, (s / 3) % 3 - 1, s % 3 - 1);
                close = ((v + t) * c2c).QLength() <= qcut;
              }
              if (close) {
                seen[j] = i;
                out[i].Add(j);
              }
            }
          }
        }
      }
      // ascending, to preserve the original pair order
      QuickSorter::Sort(out[i], TPrimitiveComparator());
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void IBondsSymmEqTask::init_equivs(TPtrList<TCAtom>& Atoms,
  const smatd_list& Matrices, bool check_translations)
{
  const size_t ac = Atoms.Count();
  if (ac == 0) {
    return;
  }
  const TAsymmUnit &AU = *Atoms[0]->GetParent();
  const TLattice &Latt = AU.GetLattice();
  const double delta = Latt.GetDelta(),
    deltaI = Latt.GetDeltaI();
  const size_t mc = Matrices.Count();
  for (size_t i = 0; i < ac; i++) {
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    for (size_t j = 1; j < mc; j++) {
      vec3d v = Atoms[i]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      AU.CellToCartesian(v -= shift);
      const double qd = v.QLength();
      if (qd < 1e-4) {
        smatd eqm(Matrices[j]);
        eqm.t += shift;
        eqm.SetId((uint8_t)j, shift);
        Atoms[i]->AddEquiv(eqm);
      }
      else {
        smatd matr = Matrices[j];
        matr.t += shift;
        matr.SetId(matr.GetContainerId(), shift);
        if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd, delta)) {
          Atoms[i]->AttachSite(Atoms[i], matr);
        }
        else if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd, deltaI)) {
          Atoms[i]->AttachSiteI(Atoms[i], matr);
        }
      }
      if (check_translations) {
        process_t(*Atoms[i], *Atoms[i], Matrices[j], shift, 0);
      }
    }
    if (check_translations) {
      process_t(*Atoms[i], *Atoms[i], Matrices[0], vec3i(0), 0);
    }
  }
}
//..............................................................................
void IBondsSymmEqTask::process_1(TPtrList<TCAtom>& Atoms, size_t ind, size_t i,
  const smatd_list& Matrices,  bool check_t,
  olx_critical_section* cs)
{
  if (Atoms[i]->IsDeleted()) {
    return;
  }
  if (Atoms[i]->GetExyzGroup() != 0 &&
    Atoms[i]->GetExyzGroup() == Atoms[ind]->GetExyzGroup())
  {
    return;
  }
  const TAsymmUnit& AU = *Atoms[i]->GetParent();
  const TLattice& Latt = AU.GetLattice();
  const TUnitCell& UC = Latt.GetUnitCell();
  const double delta = Latt.GetDelta(),
    deltaI = Latt.GetDeltaI();
  const size_t mc = Matrices.Count();
  for (size_t j = 0; j < mc; j++) {
    vec3d v = Atoms[ind]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
    const vec3i shift = v.Round<int>();
    // collect asymmetric unit bonds
    if (j == 0 && shift.IsNull()) {  // I
      AU.CellToCartesian(v);
      const double qd = v.QLength();
      if (qd < 1e-4) {
        if (Atoms[i]->GetPart() != Atoms[ind]->GetPart()) {
          continue;
        }
        if (Atoms[ind]->GetType() == iQPeakZ) {
          Atoms[ind]->SetDeleted(true);
          break;
        }
        olx_scope_cs cs_(cs);
        Atoms[i]->SetDeleted(true);
        continue;
      }
      else {
        if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd, delta)) { // covalent bond
          olx_scope_cs cs_(cs);
          Atoms[ind]->AttachSite(Atoms[i], Matrices[j]);
          Atoms[i]->AttachSite(Atoms[ind], Matrices[j]);
        }
        else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd, deltaI)) {  // interaction
          olx_scope_cs cs_(cs);
          Atoms[ind]->AttachSiteI(Atoms[i], Matrices[j]);
          Atoms[i]->AttachSiteI(Atoms[ind], Matrices[j]);
        }
      }
      if (check_t) {
        process_t(*Atoms[ind], *Atoms[i], Matrices[j], shift, cs);
      }
      continue;
    }
    AU.CellToCartesian(v -= shift);
    const double qd = v.QLength();
    if (qd < 1e-4) {
      if (Atoms[ind]->GetType() == iQPeakZ) {
        Atoms[ind]->SetDeleted(true);
        break;
      }
      if (Atoms[i]->GetPart() != Atoms[ind]->GetPart() ||
        Atoms[i]->GetPart() < 0)
      {
        continue;
      }
      if (Atoms[i]->GetParentAfixGroup() == 0) {
        olx_scope_cs cs_(cs);
        Atoms[i]->SetDeleted(true);
        continue;
      }
    }
    else {
      smatd matr = Matrices[j];
      matr.t += shift;
      matr.SetId((uint8_t)j, shift);
      if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd, delta)) {
        olx_scope_cs cs_(cs);
        Atoms[ind]->AttachSite(Atoms[i], matr);
        Atoms[i]->AttachSite(Atoms[ind], UC.InvMatrix(matr));
      }
      else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd, deltaI)) {
        olx_scope_cs cs_(cs);
        Atoms[ind]->AttachSiteI(Atoms[i], matr);
        Atoms[i]->AttachSiteI(Atoms[ind], UC.InvMatrix(matr));
      }
    }
    if (check_t) {
      process_t(*Atoms[ind], *Atoms[i], Matrices[j], shift, cs);
    }
  }
}
//..............................................................................
void IBondsSymmEqTask::process_t(TCAtom& from, TCAtom& to,
  const smatd& M, const vec3i& done_shift, olx_critical_section *cs)
{
  const TAsymmUnit& AU = *from.GetParent();
  const TLattice& Latt = AU.GetLattice();
  const TUnitCell& UC = Latt.GetUnitCell();
  const double delta = Latt.GetDelta(),
    deltaI = Latt.GetDeltaI();
  vec3d t_ccrd = M.IsFirst() ? to.ccrd() : M * to.ccrd();
  for (int ii = -1; ii <= 1; ii++) {
    for (int ij = -1; ij <= 1; ij++) {
      for (int ik = -1; ik <= 1; ik++) {
        if ((ii|ij|ik) == 0) {
          continue;
        }
        const vec3i shift = done_shift + vec3i(ii, ij, ik);
        const double qd = AU.Orthogonalise(
          from.ccrd() - shift - t_ccrd).QLength();
        smatd matr = M;
        matr.t += shift;
        matr.SetId(M.GetContainerId(), shift);
        if (TNetwork::BondExistsQ(from, to, matr, qd, delta)) {
          olx_scope_cs cs_(cs);
          from.AttachSite(&to, matr);
          if (from.GetId() != to.GetId()) {
            to.AttachSite(&from, UC.InvMatrix(matr));
          }
        }
        else if (TNetwork::BondExistsQ(from, to, matr, qd, deltaI)) {
          olx_scope_cs cs_(cs);
          from.AttachSiteI(&to, matr);
          if (from.GetId() != to.GetId()) {
            to.AttachSiteI(&from, UC.InvMatrix(matr));
          }
        }
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TBondsSymmEqTaskDirect::TBondsSymmEqTaskDirect(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : Atoms(atoms), Matrices(matrices)
{
  if (!atoms.IsEmpty()) {
    AU = atoms[0]->GetParent();
    Latt = &AU->GetLattice();
    const double cutoff = symm_eq_cutoff(Atoms, Latt->GetDelta(),
      Latt->GetDeltaI());
    SkipTranslations = min_shift_distance(*AU) > cutoff;
  }
}
//..............................................................................
void TBondsSymmEqTaskDirect::Run(size_t ind) const {
  if (Atoms[ind]->IsDeleted()) {
    return;
  }
  const size_t ac = Atoms.Count();
  for (size_t i = ind + 1; i < ac; i++) {
    IBondsSymmEqTask::process_1(Atoms, ind, i, Matrices,
      !SkipTranslations, GetCriticalSection());
  }
}
//..............................................................................
void TBondsSymmEqTaskDirect::InitEquiv() const {
  IBondsSymmEqTask::init_equivs(Atoms, Matrices, !SkipTranslations);
}
//..............................................................................
//..............................................................................
//..............................................................................
TBondsSymmEqTaskShells::TBondsSymmEqTaskShells(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : atoms(atoms), matrices(matrices), shell_thickness(3), max_r(0)
{
  if (atoms.IsEmpty()) {
    return;
  }
  const TAsymmUnit& au = *atoms[0]->GetParent();
  delta = au.GetLattice().GetDelta();
  deltaI = au.GetLattice().GetDeltaI();
  UC = &au.GetLattice().GetUnitCell();
  TTypeList<AtomInfo>& data = *(data_ = new data_t());
  TPtrList<AtomInfo>& au_atoms = *(au_atoms_ = new TPtrList<AtomInfo>(atoms.Count()));
  vec3d min_v(1000), max_v(-1000),
    min_c(1000), max_c(-1000),
    cell_c;
  size_t atom_count = 0;
  au_atoms.SetCount(atoms.Count());
  for (size_t ai = 0; ai < atoms.Count(); ai++) {
    if (atoms[ai]->IsDeleted()) {
      continue;
    }
    vec3d::UpdateMinMax(atoms[ai]->ccrd(), min_c, max_c);
    vec3d v = au.Orthogonalise(atoms[ai]->ccrd());
    AtomInfo& atom_i = data.AddNew(v, atoms[ai]);
    vec3d::UpdateMinMax(v, min_v, max_v);
    au_atoms[ai] = &atom_i;
    if (atoms[ai]->GetConnInfo().r > max_r) {
      max_r = atoms[ai]->GetConnInfo().r;
    }
    cell_c += v;
    atom_count++;
  }
  cell_c /= atom_count;

  vec3d buf;
  {
    double max_b_l = max_r * 2 + olx_max(delta, deltaI);
    const mat3d& m = au.GetCartesianToCell();
    for (int k = 0; k < 3; k++) {
      buf[k] = max_b_l * sqrt(olx_sqr(m[0][k]) + olx_sqr(m[1][k]) + olx_sqr(m[2][k]));
    }
  }
  min_c -= buf;
  max_c += buf;

  for (size_t ai = 0; ai < atoms.Count(); ai++) {
    if (atoms[ai]->IsDeleted()) {
      continue;
    }
    au_atoms[ai]->len = (au_atoms[ai]->center - cell_c).Length();
    for (size_t mi = 0; mi < matrices.Count(); mi++) {
      vec3d c = matrices[mi] * atoms[ai]->ccrd();
      vec3i t0 = (min_c - c).Ceil<int>(),
        t1 = (max_c - c).Floor<int>();
      for (int x = t0[0]; x <= t1[0]; x++) {
        for (int y = t0[1]; y <= t1[1]; y++) {
          for (int z = t0[2]; z <= t1[2]; z++) {
            // the AU atom itself is already in data
            if ((mi|x|y|z) == 0) {
              continue;
            }
            const vec3d f = c + vec3d(x, y, z);
            const vec3d v = au.Orthogonalise(f);
            // optional: a Cartesian box test here trims the corners
            // the parallelepiped over-includes in a skewed cell
            smatd m(matrices[mi].r, matrices[mi].t + vec3d(x, y, z));
            UC->InitMatrixId(m);
            AtomInfo& ai_ = data.AddNew(v, atoms[ai], m.GetId());
            ai_.len = (v - cell_c).Length();
          }
        }
      }
    }
  }
  QuickSorter::Sort(data);
  double max_span = data.GetLast().len;// -data[0].len;
  size_t shell_n = olx_round(max_span / shell_thickness) + 1;
  TTypeList<Shell>& shells = *(shells_ = new shell_data_t(shell_n));
  for (size_t di = 0; di < data.Count(); di++) {
    int shell = olx_round(data[di].len / shell_thickness);
    shells[shell].data.Add(&data[di]);
    data[di].parent = &shells[shell];
  }
  for (size_t i = 0; i < shells.Count(); i++) {
    if (shells[i].data.IsEmpty()) {
      continue;
    }
    vec3d cnt = shells[i].data[0]->center;
    shells[i].center = cnt;
    for (size_t j = 0; j < shells[i].data.Count(); j++) {
      shells[i].data[j]->slen = (shells[i].data[j]->center - cnt).Length();
    }
  }
  for (size_t i = 0; i < shells.Count(); i++) {
    QuickSorter::SortSF(shells[i].data, &Shell::Compare);
  }
}
//..............................................................................
void TBondsSymmEqTaskShells::ProcessMatch(TCAtom *atom, AtomInfo *match, double qd)
  const 
{
  if (qd < 1e-4) {
    if (atom->GetPart() != match->atom->GetPart()) {
      return;
    }
    if (atom->GetType() == iQPeakZ) {
      atom->SetDeleted(true);
    }
    olx_scope_cs cs_(GetCriticalSection());
    match->atom->SetDeleted(true);
  }
  else {
    if (TNetwork::BondExistsQ(*atom, *match->atom, qd, delta)) { // covalent bond
      atom->AttachSite(match->atom, UC->GetMatrixById(match->mat_id));
    }
    else if (TNetwork::BondExistsQ(*atom, *match->atom, qd, deltaI)) { // interaction
      atom->AttachSiteI(match->atom, UC->GetMatrixById(match->mat_id));
    }
  }
}
//..............................................................................
void TBondsSymmEqTaskShells::CheckShell(const Shell& shell, const vec3d& v,
  TCAtom* self_atom, double max_ql) const
{
  const TPtrList<AtomInfo>& s_data = shell.data;
  if (s_data.IsEmpty()) {
    return;
  }
  if (s_data.Count() == 1) {
    if (s_data[0]->atom != self_atom) {
      double ql = (v - s_data[0]->center).QLength();
      if (ql < max_ql) {
        ProcessMatch(self_atom, s_data[0], ql);
      }
    }
    return;
  }
  const double max_b_l = sqrt(max_ql);
  const double query_to_center = (v - shell.center).Length();
  AtomInfo probe(v, self_atom);
  probe.parent = const_cast<Shell*>(&shell);
  probe.slen = query_to_center;
  size_t idx = sorted::FindInsertIndex(s_data,
    FunctionComparator::Make(&Shell::Compare), &probe);

  // go up
  for (size_t i = idx; i < s_data.Count(); i++) {
    if (olx_abs(query_to_center - s_data[i]->slen) > max_b_l) {
      break;
    }
    if (s_data[i]->atom != self_atom) {
      double ql = (v - s_data[i]->center).QLength();
      if (ql < max_ql) {
        ProcessMatch(self_atom, s_data[i], ql);
      }
    }
  }
  // go down
  if (idx > 0) {
    for (size_t i = idx - 1; i != InvalidIndex; i--) {
      if (olx_abs(query_to_center - s_data[i]->slen) > max_b_l) {
        break;
      }
      if (s_data[i]->atom != self_atom) {
        double ql = (v - s_data[i]->center).QLength();
        if (ql < max_ql) {
          ProcessMatch(self_atom, s_data[i], ql);
        }
      }
    }
  }
}
//..............................................................................
void TBondsSymmEqTaskShells::Run(size_t ind) const {
  if (atoms[ind]->IsDeleted()) {
    return;
  }
  const shell_data_t& shells = *shells_;
  const AtomInfo& q = *(*au_atoms_)[ind];
  const double c = atoms[ind]->GetConnInfo().r + max_r + olx_max(delta, deltaI);
  // shell k covers [(k - 0.5) t, (k + 0.5) t) because of the rounding
  const size_t s0 = (size_t)olx_max(0, olx_round((q.len - c) / shell_thickness));
  const size_t s1 = olx_min(shells.Count() - 1,
    (size_t)olx_round((q.len + c) / shell_thickness));
  for (size_t s = s0; s <= s1; s++) {
    CheckShell(shells[s], q.center, atoms[ind], c * c);
  }
}
//..............................................................................
void TBondsSymmEqTaskShells::InitEquiv() const {
  if (atoms.IsEmpty()) {
    return;
  }
  const TAsymmUnit &AU = *atoms[0]->GetParent();
  const TLattice& Latt = AU.GetLattice();
  const double cutoff = symm_eq_cutoff(atoms, Latt.GetDelta(), Latt.GetDeltaI());
  bool SkipTranslations = min_shift_distance(AU) > cutoff;
  IBondsSymmEqTask::init_equivs(atoms, matrices, !SkipTranslations);
}
//..............................................................................
//..............................................................................
//..............................................................................
TBondsSymmEqTaskCellList::TBondsSymmEqTaskCellList(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : Atoms(atoms), Matrices(matrices)
{
  if (atoms.IsEmpty()) {
    return;
  }
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
  const double cutoff = symm_eq_cutoff(Atoms, Latt->GetDelta(),
    Latt->GetDeltaI());
  SkipTranslations = min_shift_distance(*AU) > cutoff;
  Neighbours_ = new TArrayList<TSizeList>();
  build_symm_eq_neighbours(*AU, Atoms, Matrices,
    cutoff, !SkipTranslations, *Neighbours_);
}
//..............................................................................
TBondsSymmEqTaskCellList::TBondsSymmEqTaskCellList(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices,
  olx_object_ptr<TArrayList<TSizeList> > Neighbours, bool skip_translations)
  : Atoms(atoms), Matrices(matrices), Neighbours_(Neighbours),
  SkipTranslations(skip_translations)
{
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TBondsSymmEqTaskCellList::Run(size_t ind) const {
  if (Atoms[ind]->IsDeleted()) {
    return;
  }
  /* ascending and only i > ind, so the pairs come in the original order -
  the body deletes atoms and later iterations test IsDeleted
  */
  const TSizeList* nb = (!Neighbours_.ok()) ? 0 : &(*Neighbours_)[ind];
  const size_t n = (nb == 0) ? (Atoms.Count() - ind - 1) : nb->Count();
  for (size_t ni = 0; ni < n; ni++) {
    const size_t i = (nb == 0) ? (ind + 1 + ni) : (*nb)[ni];
    IBondsSymmEqTask::process_1(Atoms, ind, i, Matrices,
      !SkipTranslations, GetCriticalSection());
  }
}
//..............................................................................
void TBondsSymmEqTaskCellList::InitEquiv() const {
  IBondsSymmEqTask::init_equivs(Atoms, Matrices, !SkipTranslations);
}
//..............................................................................
//..............................................................................
//..............................................................................
olx_object_ptr<IBondsSymmEqTask>
  BondsSymmEqTaskFactory::build(TPtrList<TCAtom>& atoms, const smatd_list& matrices,
  int type)
{
  TStopWatch sw("Building BondsSymmEqTask");
  if (type == BondsSymmEqTaskDefault) {
    type = default_type();
  }
  if (type == BondsSymmEqTaskDefault && !atoms.IsEmpty()) {
    const TAsymmUnit& au = *atoms[0]->GetParent();
    const TLattice& latt = au.GetLattice();
    if (symm_eq_cutoff(atoms, latt.GetDelta(), latt.GetDeltaI()) >
      min_shift_distance(au))
    {
      type = BondsSymmEqTaskDirect;
    }
    else {
      type = BondsSymmEqTaskCellList;
    }
  }
  switch (type) {
  case BondsSymmEqTaskDefault:
    return new TBondsSymmEqTaskDirect(atoms, matrices);
  case BondsSymmEqTaskCellList:
    return new TBondsSymmEqTaskCellList(atoms, matrices);
  case BondsSymmEqTaskShells:
    return new TBondsSymmEqTaskShells(atoms, matrices);
  }
  return new TBondsSymmEqTaskDirect(atoms, matrices);
}
