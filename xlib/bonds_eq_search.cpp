#include "bonds_eq_search.h"
#include "unitcell.h"
#include "estopwatch.h"

TBondsSymmEqTaskDirect::TBondsSymmEqTaskDirect(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : Atoms(atoms), Matrices(matrices)
{
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TBondsSymmEqTaskDirect::Run(size_t ind) const {
  if (Atoms[ind]->IsDeleted()) {
    return;
  }
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for (size_t i = ind + 1; i < ac; i++) {
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    if (Atoms[i]->GetExyzGroup() != 0 &&
      Atoms[i]->GetExyzGroup() == Atoms[ind]->GetExyzGroup())
    {
      continue;
    }
    for (size_t j = 0; j < mc; j++) {
      vec3d v = Atoms[ind]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      // collect asymmetric unit bonds
      if (j == 0 && shift.IsNull()) {  // I
        AU->CellToCartesian(v);
        const double qd = v.QLength();
        if (qd < 1e-4) {
          if (Atoms[i]->GetPart() != Atoms[ind]->GetPart()) {
            continue;
          }
          if (Atoms[ind]->GetType() == iQPeakZ) {
            Atoms[ind]->SetDeleted(true);
            break;
          }
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
        else {
          if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
            Latt->GetDelta()))  // covalent bond
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSite(Atoms[i], Matrices[j]);
            Atoms[i]->AttachSite(Atoms[ind], Matrices[j]);
          }
          else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
            Latt->GetDeltaI()))  // interaction
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSiteI(Atoms[i], Matrices[j]);
            Atoms[i]->AttachSiteI(Atoms[ind], Matrices[j]);
          }
        }
        continue;
      }
      AU->CellToCartesian(v -= shift);
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
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
      }
      else {
        smatd matr = Matrices[j];
        matr.t += shift;
        matr.SetId((uint8_t)j, shift);
        if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDelta()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSite(Atoms[i], matr);
          Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
        else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDeltaI()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSiteI(Atoms[i], matr);
          Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
      }
    }
    for (int ii = -1; ii <= 1; ii++) {
      for (int ij = -1; ij <= 1; ij++) {
        for (int ik = -1; ik <= 1; ik++) {
          const vec3i shift(ii, ij, ik);
          const double qd = AU->Orthogonalise(
            Atoms[ind]->ccrd() - shift - Atoms[i]->ccrd()).QLength();
          smatd matr = Matrices[0];
          matr.t += shift;
          matr.SetId(0, shift);
          if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDelta()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSite(Atoms[i], matr);
            if (i != ind) {
              Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
            }
          }
          else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDeltaI()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSiteI(Atoms[i], matr);
            if (i != ind) {
              Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
            }
          }
        }
      }
    }
  }
}
//..............................................................................
void TBondsSymmEqTaskDirect::InitEquiv() const {
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for (size_t i = 0; i < ac; i++) {
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    for (size_t j = 1; j < mc; j++) {
      vec3d v = Atoms[i]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      AU->CellToCartesian(v -= shift);
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
        matr.SetId((uint8_t)j, shift);
        if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDelta()))
        {
          Atoms[i]->AttachSite(Atoms[i], matr);
        }
        else if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDeltaI()))
        {
          Atoms[i]->AttachSiteI(Atoms[i], matr);
        }
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TBondsSymmEqTaskShells::TBondsSymmEqTaskShells(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : atoms(atoms), min_d(1000)
{
  if (atoms.IsEmpty()) {
    return;
  }
  const TAsymmUnit& au = *atoms[0]->GetParent();
  TTypeList<AtomInfo>& data = *(data_ = new data_t());
  vec3d max_d(-1000);
  double shell_thickness = 3,
    max_b_ql = 3 * 3;
  for (size_t ai = 0; ai < atoms.Count(); ai++) {
    if (atoms[ai]->IsDeleted()) {
      continue;
    }
    vec3d v = au.Orthogonalise(atoms[ai]->ccrd());
    data.AddNew(v, atoms[ai]);
    vec3d::UpdateMinMax(v, min_d, max_d);
  }
  // allow some space at the min_d
  min_d = min_d - vec3d(max_b_ql);
  // sort from the min_d so that can build stacks
  for (size_t di = 0; di < data.Count(); di++) {
    data[di].ql = (data[di].center - min_d).QLength();
  }
  QuickSorter::Sort(data);
  vec3i dim = (max_d - min_d).Floor<int>() + vec3i(1);
  size_t shell_n = olx_round(dim.QLength() / shell_thickness);
  TTypeList<Shell>& shells = *(shells_ = new shell_data_t(shell_n));
  for (size_t di = 0; di < data.Count(); di++) {
    int shell = olx_round(data[di].ql);
    shells[shell].data.Add(&data[di]);
    shells[shell].center += data[di].center;
    data[di].parent = &shells[shell];
  }
  for (size_t i = 0; i < shells.Count(); i++) {
    if (shells[i].data.IsEmpty()) {
      continue;
    }
    shells[i].center /= shells[i].data.Count();
  }
  for (size_t i = 0; i < shells.Count(); i++) {
    QuickSorter::SortSF(shells[i].data, &Shell::Compare);
  }
  double max_ql = data.GetLast().ql;
  for (size_t ai = 0; ai < atoms.Count(); ai++) {
    if (atoms[ai]->IsDeleted()) {
      continue;
    }
    for (size_t mi = 1; mi < matrices.Count(); mi++) {
      vec3d v = au.Orthogonalise(matrices[mi] * atoms[ai]->ccrd());
      double ql = (v - min_d).QLength();
      if (max_ql - ql > max_b_ql) {
        continue;
      }
      int shell = olx_round(ql);
      if (ql > max_ql) {
        max_ql = ql;
        if (shell >= shells.Count()) {
          size_t shell_n = olx_round(ql / shell_thickness);
          shells.SetCount(shell_n);
        }
      }
      TPtrList<AtomInfo>& s_data = shells[shell].data;
      olx_object_ptr<AtomInfo> i = new AtomInfo(v, atoms[ai], matrices[mi].GetId());
      if (ql < data[0].ql) {
        data.Insert(0, *i);
      }
      else if (ql > data.GetLast().ql) {
        data.Add(*i);
      }
      else {
        size_t ii = sorted::FindInsertIndex(data, TComparableComparator(), *i);
        data.Insert(ii, *i);
      }
      i->parent = &shells[shell];
      if (s_data.IsEmpty()) {
        s_data.Add(&i);
        shells[shell].center = i->center;
      }
      else {
        vec3d sc_n = s_data[0]->center * s_data.Count();
        if (Shell::Compare(*s_data[0], i) < 0) {
          s_data.Insert(0, &i);
        }
        else if (Shell::Compare(*s_data.GetLast(), i) < 0) {
          s_data.Add(&i);
          double l_ql = (shells[shell].data.GetLast()->center - shells[shell].center).QLength();
          double n_ql = (v - shells[shell].center).QLength();
          if (l_ql - n_ql > max_b_ql) {
            continue;
          }
        }
        else {
          size_t ii = sorted::FindInsertIndex(s_data,
            FunctionComparator::Make(&Shell::Compare), i);
          s_data.Insert(ii, &i);
        }
        s_data[0]->center = (sc_n + v) / s_data.Count();
      }
      shells[shell].data.Add();
      i.release();
    }
  }
}
//..............................................................................
// Checks a single shell for atoms within max_b_ql (squared) of v.
// self_atom is excluded from matches. Found candidates are appended to `matches`.
void TBondsSymmEqTaskShells::ProcessMatch(TCAtom *atom, AtomInfo *match, double qd) {
  //if (qd < 1e-4) {
  //  if (atom->GetPart() != match->atom->GetPart()) {
  //    return;
  //  }
  //  if (atom->GetType() == iQPeakZ) {
  //    atom->SetDeleted(true);
  //    break;
  //  }
  //  volatile olx_scope_cs cs_(GetCriticalSection());
  //  match->->SetDeleted(true);
  //}
  //else {
  //  if (TNetwork::BondExistsQ(*atom, *match->atom, qd,
  //    Latt->GetDelta()))  // covalent bond
  //  {
  //    volatile olx_scope_cs cs_(GetCriticalSection());
  //    atom->AttachSite(match->atom, match->mat_id);
  //    match->atom->AttachSite(Atoms[ind], Matrices[j]);
  //  }
  //  else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
  //    Latt->GetDeltaI()))  // interaction
  //  {
  //    volatile olx_scope_cs cs_(GetCriticalSection());
  //    Atoms[ind]->AttachSiteI(Atoms[i], Matrices[j]);
  //    Atoms[i]->AttachSiteI(Atoms[ind], Matrices[j]);
  //  }
  //}

}
void TBondsSymmEqTaskShells::CheckShell(const Shell& shell, const vec3d& v, TCAtom* self_atom,
  double max_b_ql, TLinkedList<AtomInfo*>& matches) const
{
  const TPtrList<AtomInfo>& s_data = shell.data;
  if (s_data.IsEmpty()) {
    return;
  }
  const double max_b_l = sqrt(max_b_ql);
  const double query_to_center = (v - shell.center).Length();

  // temporary probe so FindInsertIndex can use Shell::Compare correctly
  AtomInfo probe(v, self_atom);
  probe.parent = const_cast<Shell*>(&shell);
  size_t idx = sorted::FindInsertIndex(s_data,
    FunctionComparator::Make(&Shell::Compare), &probe);

  // right sweep: idx, idx+1, ...
  for (size_t i = idx; i < s_data.Count(); i++) {
    double d_to_center = (s_data[i]->center - shell.center).Length();
    if (olx_abs(query_to_center - d_to_center) > max_b_l) {
      break;
    }
    if (s_data[i]->atom != self_atom &&
      (v - s_data[i]->center).QLength() <= max_b_ql)
    {
      matches.Add(s_data[i]);
    }
  }
  // left sweep: idx-1, idx-2, ...
  if (idx > 0) {
    for (size_t i = idx - 1; ; i--) {
      double d_to_center = (s_data[i]->center - shell.center).Length();
      if (olx_abs(query_to_center - d_to_center) > max_b_l) {
        break;
      }
      if (s_data[i]->atom != self_atom &&
        (v - s_data[i]->center).QLength() <= max_b_ql)
      {
        matches.Add(s_data[i]);
      }
      if (i == 0) {
        break;
      }
    }
  }
}
//..............................................................................
void TBondsSymmEqTaskShells::Run(size_t ind) const {
  if (atoms[ind]->IsDeleted()) {
    return;
  }
  const data_t& data = *data_;
  const shell_data_t& shells = *shells_;
  const TAsymmUnit& au = *atoms[ind]->GetParent();
  vec3d v = au.Orthogonalise(atoms[ind]->ccrd());
  double ql = (v - min_d).QLength();
  int shell = olx_round(ql);
  if (shell > shells.Count()) {
    return;
  }
  double max_b_ql = 3 * 3;
  typedef TLinkedList<AtomInfo*> matches_list_t;
  matches_list_t matches;
  CheckShell(shells[shell], v, atoms[ind], max_b_ql, matches);
  if (shell > 0) {
    CheckShell(shells[shell - 1], v, atoms[ind], max_b_ql, matches);
  }
  if (shell + 1 < (int)shells.Count()) {
    CheckShell(shells[shell + 1], v, atoms[ind], max_b_ql, matches);
  }
  matches_list_t::Iterator itr = matches.GetIterator();
  while (itr.HasNext()) {
    AtomInfo* ai = itr.Next();

  }

}
//..............................................................................
void TBondsSymmEqTaskShells::InitEquiv() const {
}
//..............................................................................
//..............................................................................
//..............................................................................
namespace {
  /* beyond this the search can find nothing: every outcome needs
  d < r(a1)+r(a2)+delta, so this bounds them all. Not a tolerance
  */
  double symm_eq_cutoff(const TCAtomPList& atoms, double delta, double deltaI) {
    double r = 0;
    for (size_t i = 0; i < atoms.Count(); i++) {
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
    TSizeList seen(ac);
    for (size_t i = 0; i < ac; i++) {
      seen[i] = InvalidIndex;
    }
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
              if ((v * c2c).QLength() <= qcut) {
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
TBondsSymmEqTaskCellList::TBondsSymmEqTaskCellList(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : Atoms(atoms), Matrices(matrices)
{
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();

  const size_t cell_list_min_atoms = 500;
  const bool use_cell_list = Atoms.Count() >= cell_list_min_atoms;
  const double cutoff = symm_eq_cutoff(Atoms, Latt->GetDelta(),
    Latt->GetDeltaI());
  if (use_cell_list) {
    TStopWatch sw1("Neighbour lists");
    Neighbours_ = new TArrayList<TSizeList>();
    build_symm_eq_neighbours(*AU, Atoms, Matrices,
      cutoff, *Neighbours_);
  }
  /* if the closest a shift could come is beyond the cutoff the -1..1 loop
  cannot find anything and goes whole. The identity test matters: both the
  loop and min_shift_distance read Matrices[0] as I with no translation
  */
  SkipTranslations = Matrices[0].r.IsI() &&
    Matrices[0].t.IsNull() &&
    min_shift_distance(Latt->GetAsymmUnit()) > cutoff;
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
  const size_t mc = Matrices.Count();
  /* ascending and only i > ind, so the pairs come in the original order -
  the body deletes atoms and later iterations test IsDeleted
  */
  const TSizeList* nb = (!Neighbours_.ok()) ? 0 : &(*Neighbours_)[ind];
  const size_t n = (nb == 0) ? (Atoms.Count() - ind - 1) : nb->Count();
  for (size_t ni = 0; ni < n; ni++) {
    const size_t i = (nb == 0) ? (ind + 1 + ni) : (*nb)[ni];
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    if (Atoms[i]->GetExyzGroup() != 0 &&
      Atoms[i]->GetExyzGroup() == Atoms[ind]->GetExyzGroup())
    {
      continue;
    }
    vec3i done_shift(0, 0, 0);
    for (size_t j = 0; j < mc; j++) {
      vec3d v = Atoms[ind]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      if (j == 0) {
        done_shift = shift;
      }
      // collect asymmetric unit bonds
      if (j == 0 && shift.IsNull()) {  // I
        AU->CellToCartesian(v);
        const double qd = v.QLength();
        if (qd < 1e-4) {
          if (Atoms[i]->GetPart() != Atoms[ind]->GetPart()) {
            continue;
          }
          if (Atoms[ind]->GetType() == iQPeakZ) {
            Atoms[ind]->SetDeleted(true);
            break;
          }
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
        else {
          if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
            Latt->GetDelta()))  // covalent bond
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSite(Atoms[i], Matrices[j]);
            Atoms[i]->AttachSite(Atoms[ind], Matrices[j]);
          }
          else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
            Latt->GetDeltaI()))  // interaction
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSiteI(Atoms[i], Matrices[j]);
            Atoms[i]->AttachSiteI(Atoms[ind], Matrices[j]);
          }
        }
        continue;
      }
      AU->CellToCartesian(v -= shift);
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
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
      }
      else {
        smatd matr = Matrices[j];
        matr.t += shift;
        matr.SetId((uint8_t)j, shift);
        if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDelta()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSite(Atoms[i], matr);
          Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
        else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDeltaI()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSiteI(Atoms[i], matr);
          Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
      }
    }
    /* done_shift was already tested above, so skipping it drops a duplicate.
    The others cannot go in general - Round() minimises the fractional
    difference, not the cartesian one, so the image it picks need not be the
    nearest (ZZULI2). SkipTranslations is the provable case, see FindSymmEq
    */
    if (SkipTranslations) {
      continue;
    }
    for (int ii = -1; ii <= 1; ii++) {
      for (int ij = -1; ij <= 1; ij++) {
        for (int ik = -1; ik <= 1; ik++) {
          const vec3i shift(ii, ij, ik);
          if (shift == done_shift) {
            continue;
          }
          const double qd = AU->Orthogonalise(
            Atoms[ind]->ccrd() - shift - Atoms[i]->ccrd()).QLength();
          smatd matr = Matrices[0];
          matr.t += shift;
          matr.SetId(0, shift);
          if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDelta()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSite(Atoms[i], matr);
            if (i != ind) {
              Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
            }
          }
          else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDeltaI()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSiteI(Atoms[i], matr);
            if (i != ind) {
              Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
            }
          }
        }
      }
    }
  }
}
//..............................................................................
void TBondsSymmEqTaskCellList::InitEquiv() const {
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for (size_t i = 0; i < ac; i++) {
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    for (size_t j = 1; j < mc; j++) {
      vec3d v = Atoms[i]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      AU->CellToCartesian(v -= shift);
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
        matr.SetId((uint8_t)j, shift);
        if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDelta()))
        {
          Atoms[i]->AttachSite(Atoms[i], matr);
        }
        else if (TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDeltaI()))
        {
          Atoms[i]->AttachSiteI(Atoms[i], matr);
        }
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
olx_object_ptr<IBondsSymmEqTask>
  BondsSymmEqTaskFctory::build(TPtrList<TCAtom>& atoms, const smatd_list& matrices,
  int type)
{
  if (type == BondsSymmEqTaskDefault) {
    type = default_type();
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
