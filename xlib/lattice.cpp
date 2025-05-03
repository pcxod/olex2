/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "lattice.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "network.h"
#include "sbond.h"
#include "splane.h"
#include "ellipsoid.h"
#include "bapp.h"
#include "log.h"
#include "emath.h"
#include "congen.h"
#include "estlist.h"
#include "library.h"
#include "olxmps.h"
#include "estrbuffer.h"
#include "symmparser.h"
#include "equeue.h"
#include "analysis.h"
#include "estopwatch.h"

#undef GetObject

// sorts largest -> smallest
int TLattice_SortAtomsById(const TSAtom* a1, const TSAtom* a2) {
  return olx_cmp(a1->CAtom().GetId(), a2->CAtom().GetId());
}
int TLattice_AtomsSortByDistance(const TSAtom* A1, const TSAtom* A2) {
  const double d = A1->crd().QLength() - A2->crd().QLength();
  return (d < 0 ? -1 : ((d > 0) ? 1 : 0));
}

TLattice::TLattice(ASObjectProvider& ObjectProvider) :
  Objects(ObjectProvider),
  OnStructureGrow(Actions.New("STRGEN")),
  OnStructureUniq(Actions.New("STRUNIQ")),
  OnDisassemble(Actions.New("DISASSEBLE")),
  OnAtomsDeleted(Actions.New("ATOMSDELETE"))
{
  AsymmUnit = new TAsymmUnit(this);
  UnitCell = new TUnitCell(this);
  Network = new TNetwork(this, 0);
  Delta = 0.5f;
  DeltaI = 1.2f;
  _GrowInfo = 0;
}
//..............................................................................
TLattice::~TLattice() {
  Clear(true);
  delete UnitCell;
  delete AsymmUnit;
  delete Network;
  olx_del_obj(_GrowInfo);
  delete &Objects;
}
//..............................................................................
void TLattice::ClearAtoms() {
  if (!Objects.atoms.IsEmpty()) {
    OnAtomsDeleted.Enter(this);
    Objects.atoms.Clear();
    OnAtomsDeleted.Exit(this);
  }
}
//..............................................................................
void TLattice::ClearFragments() {
  Fragments.DeleteItems();
  Fragments.Clear();
}
//..............................................................................
void TLattice::ClearMatrices()  {
  Matrices.DeleteItems();
  Matrices.Clear();
}
//..............................................................................
void TLattice::Clear(bool ClearUnitCell) {
  ClearAtoms();
  ClearBonds();
  ClearFragments();
  ClearMatrices();
  ClearPlanes();
  if (ClearUnitCell) {
    GetUnitCell().Clear();
    GetAsymmUnit().Clear();
  }
}
//..............................................................................
ConstPtrList<smatd> TLattice::GenerateMatrices(
  const vec3d& VFrom, const vec3d& VTo)
{
  olx_pdict<uint32_t, smatd*> matrices;
  const vec3i ts = (VFrom - vec3d(0)).Round<int>(),
    tt = (VTo - vec3d(0)).Round<int>();
  const TUnitCell &uc = GetUnitCell();
  TAsymmUnit &au = GetAsymmUnit();
  for (size_t i = 0; i < uc.MatrixCount(); i++) {
    const smatd& m = uc.GetMatrix(i);
    for (size_t j = 0; j < au.AtomCount(); j++) {
      TCAtom& ca = au.GetAtom(j);
      if (!ca.IsAvailable())  continue;
      vec3d c = m*ca.ccrd();
      vec3i t = -c.Floor<int>();
      c += t;
      for (int tx = ts[0]; tx <= tt[0]; tx++) {
        for (int ty = ts[1]; ty <= tt[1]; ty++) {
          for (int tz = ts[2]; tz <= tt[2]; tz++) {
            vec3i t_(tx, ty, tz);
            if (!vec3d::IsInRangeInc(c + t_, VFrom, VTo)) continue;
            t_ += t;
            const uint32_t m_id = smatd::GenerateId((uint8_t)i, t_);
            if (!matrices.HasKey(m_id)) {
              smatd *m_ = new smatd(m);
              m_->SetRawId(m_id);
              matrices.Add(m_id, m_)->t += t_;
            }
          }
        }
      }
    }
  }
  TPtrList<smatd> result(matrices.Count());
  for (size_t i = 0; i < matrices.Count(); i++) {
    result[i] = matrices.GetValue(i);
  }
  if (result.IsEmpty()) {
    result.Add(new smatd)->I().SetId(0);
  }
  return result;
}
//..............................................................................
ConstPtrList<smatd> TLattice::GenerateMatrices(
  const vec3d& center_, double rad)
{
  const TAsymmUnit& au = GetAsymmUnit();
  const TUnitCell &uc = GetUnitCell();
  const double qrad = rad*rad;
  olx_pdict<uint32_t, smatd*> matrices;
  const vec3d center = au.Fractionalise(center_);
  for (size_t i = 0; i < uc.MatrixCount(); i++) {
    const smatd& m = uc.GetMatrix(i);
    for (size_t j = 0; j < au.AtomCount(); j++) {
      TCAtom& ca = au.GetAtom(j);
      if (!ca.IsAvailable()) {
        continue;
      }
      vec3d c = m*ca.ccrd();
      vec3i t = -c.Floor<int>();
      c += t;
      c -= center;
      for (int tx = -4; tx <= 4; tx++) {
        for (int ty = -4; ty <= 4; ty++) {
          for (int tz = -4; tz <= 4; tz++) {
            vec3i t_(tx, ty, tz);
            if (au.Orthogonalise(c + t_).QLength() > qrad) {
              continue;
            }
            t_ += t;
            const uint32_t m_id = smatd::GenerateId((uint8_t)i, t_);
            if (!matrices.HasKey(m_id)) {
              smatd *m_ = new smatd(m);
              m_->SetRawId(m_id);
              matrices.Add(m_id, m_)->t += t_;
            }
          }
        }
      }
    }
  }
  TPtrList<smatd> result(matrices.Count());
  for (size_t i = 0; i < matrices.Count(); i++) {
    result[i] = matrices.GetValue(i);
  }
  if (result.IsEmpty()) {
    result.Add(new smatd)->I().SetId(0);
  }
  return result;
}
//..............................................................................
void TLattice::GenerateBondsAndFragments(TArrayList<vec3d> *ocrd) {
  volatile TStopWatch sw(__FUNC__);
  // treat detached and the rest of atoms separately
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (ocrd != 0) {
      (*ocrd)[i] = sa.crd();
      sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
    }
    if (!sa.IsAvailable()) {
      sa.SetNetwork(*Network);
    }
  }
  BuildAtomRegistry();
  Network->Disassemble(Objects, Fragments);
  size_t dac = 0;
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.IsDeleted()) {
      dac++;
    }
    else {
      if (ocrd != 0) {
        sa.crd() = (*ocrd)[i];
      }
    }
  }
  if (dac != 0) {
    OnAtomsDeleted.Enter(this);
    for (size_t i = 0; i < ac; i++) {
      if (Objects.atoms[i].IsDeleted()) {
        Objects.atoms.Null(i);
      }
    }
    Objects.atoms.Pack();
    OnAtomsDeleted.Exit(this);
  }
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    GetAsymmUnit().GetAtom(i).SetFragmentId(-1);
  }
  BubbleSorter::SortSF(Fragments, CompareFragmentsBySize);
  for (size_t i = 0; i < Fragments.Count(); i++) {
    Fragments[i]->SetOwnerId(i);
    for (size_t j = 0; j < Fragments[i]->NodeCount(); j++) {
      Fragments[i]->Node(j).CAtom().SetFragmentId((uint32_t)i);
    }
  }
}
//..............................................................................
void TLattice::BuildPlanes() {
  ClearPlanes();
  TXApp& app = TXApp::GetInstance();
  for (size_t i = 0; i < PlaneDefs.Count(); i++) {
    TSPlane::Def& pd = PlaneDefs[i];
    for (size_t j = 0; j < Matrices.Count(); j++) {
      TSPlane* p = pd.FromAtomRegistry(app, Objects, i, Network, *Matrices[j]);
      if (p != 0) {
        bool uniq = true;
        for (size_t k = 0; k < Objects.planes.Count() - 1; k++) {
          if (Objects.planes[k].GetCenter().QDistanceTo(p->GetCenter()) < 1e-6) {
            uniq = false;
            break;
          }
        }
        if (!uniq) {
          Objects.planes.DeleteLast();
        }
      }
    }
  }
}
//..............................................................................
void TLattice::InitBody() {
  volatile TStopWatch sw(__FUNC__);
  OnDisassemble.Enter(this);
  if (!ApplyGrowInfo()) {
    // create identity matrix
    Matrices.Add(new smatd(GetUnitCell().GetMatrix(0)))->SetId(0);
    ClearPlanes();
    Objects.atoms.IncCapacity(GetAsymmUnit().AtomCount());
    for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
      TCAtom& CA = GetAsymmUnit().GetAtom(i);
      if (CA.IsDeleted()) {
        continue;
      }
      GenerateAtom(CA, *Matrices[0]);
    }
  }
  GenerateBondsAndFragments(0);
  BuildPlanes();
  OnDisassemble.Exit(this);
}
//..............................................................................
//..............................................................................
void TLattice::DefaultConnectivityGenerator::Generate() const {
  UnitCell.FindSymmEq();
};
//..............................................................................
//..............................................................................
void TLattice::Init(const IConnectivityGenerator &cg) {
  volatile TStopWatch sw(__FUNC__);
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  GetAsymmUnit().GetRefMod()->UpdateUsedSymm(GetUnitCell());
  try {
    cg.Generate();
    InitBody();
  }
  catch (const TExceptionBase &e) {
    // must clear RM as the AU is cleared!
    if (GetAsymmUnit().GetRefMod() != 0) {
      GetAsymmUnit().GetRefMod()->Clear(rm_clear_ALL);
    }
    Clear(true);
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//..............................................................................
void TLattice::Uniq() {
  OnStructureUniq.Enter(this);
  Clear(false);
  ClearMatrices();
  GetUnitCell().UpdateEllipsoids();  // if new atoms are created...
  GetUnitCell().FindSymmEq();
  InitBody();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template) {
  OnStructureGrow.Enter(this);
  Generate(Template, false);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(TCAtomPList* Template, bool ClearCont) {
  if (ClearCont && Template != 0) {
    ClearAtoms();
  }
  else {
    const size_t ac = Objects.atoms.Count();
    size_t da = 0;
    for (size_t i = 0; i < ac; i++) {  // restore atom coordinates
      TSAtom& sa = Objects.atoms[i];
      if (sa.IsDeleted()) {
        da++;
      }
      else {
        sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
      }
    }
    if (da != 0) {
      const size_t ac = Objects.atoms.Count();
      OnAtomsDeleted.Enter(this);
      for (size_t i = 0; i < ac; i++) {  // restore atom coordinates
        if (Objects.atoms[i].IsDeleted()) {
          Objects.atoms.Null(i);
        }
      }
      Objects.atoms.Pack();
      OnAtomsDeleted.Exit(this);
    }
  }
  const TCAtomPList &al = (Template != 0 && !Template->IsEmpty())
    ? *Template : GetAsymmUnit().GetAtoms();
  Objects.atoms.IncCapacity(Matrices.Count()*al.Count());
  for (size_t i = 0; i < Matrices.Count(); i++) {
    for (size_t j = 0; j < al.Count(); j++) {
      if (!al[j]->IsAvailable()) {
        continue;
      }
      GenerateAtom(*al[j], *Matrices[i]);
    }
  }
  Disassemble();
}
//..............................................................................
void TLattice::GenerateCell() {
  ClearAtoms();
  ClearMatrices();
  OnStructureGrow.Enter(this);
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  olx_pdict<uint32_t, smatd*> matrices;
  for (size_t i = 0; i < uc.MatrixCount(); i++) {
    const smatd& m = uc.GetMatrix(i);
    for (size_t j = 0; j < au.AtomCount(); j++) {
      TCAtom& ca = au.GetAtom(j);
      if (!ca.IsAvailable()) {
        continue;
      }
      TSAtom& sa = Objects.atoms.New(Network);
      sa.CAtom(ca);
      sa.ccrd() = m*ca.ccrd();
      vec3i t = -sa.ccrd().Floor<int>();
      sa.ccrd() += t;
      // 2019-02-11, H13A 2 0.499999!! 0.230127 0.251084
      for (int k = 0; k < 3; k++) {
        if (olx_abs(sa.ccrd()[k] - 1) < 1e-3) {
          t[k] -= 1;
          sa.ccrd()[k] = 0;
        }
      }
      const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
      smatd* lm = matrices.Find(m_id, 0);
      if (lm == 0) {
        lm = matrices.Add(m_id, new smatd(m));
        lm->t += t;
        lm->SetRawId(m_id);
      }
      sa.crd() = au.Orthogonalise(sa.ccrd());
      sa.SetEllipsoid(&GetUnitCell().GetEllipsoid(m.GetContainerId(),
        ca.GetId()));
      sa._SetMatrix(*lm);
    }
  }
  Matrices.SetCapacity(matrices.Count());
  for (size_t i = 0; i < matrices.Count(); i++) {
    Matrices.Add(matrices.GetValue(i));
  }
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const IVolumeValidator& v, TCAtomPList *Template,
  bool clear_content, bool cart_validator)
{
  OnStructureGrow.Enter(this);
  if (clear_content) {
    ClearAtoms();
    ClearMatrices();
  }
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  olx_pdict<uint32_t, smatd*> matrices;
  const TCAtomPList& al = (Template != 0 && !Template->IsEmpty())
    ? *Template : GetAsymmUnit().GetAtoms();
  for (size_t i = 0; i < uc.MatrixCount(); i++) {
    const smatd& m = uc.GetMatrix(i);
    for (int di = -3; di <= 3; di++) {
      for (int dj = -3; dj <= 3; dj++) {
        for (int dk = -3; dk <= 3; dk++) {
          const vec3d t(di, dj, dk);
          const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
          smatd* lm = matrices.Find(m_id, 0);
          bool matrix_created = false;
          if (lm == 0) {
            lm = new smatd(m);
            lm->t += t;
            lm->SetRawId(m_id);
            matrix_created = true;
          }
          for (size_t j = 0; j < al.Count(); j++) {
            TCAtom& ca = *al[j];
            if (!ca.IsAvailable()) {
              continue;
            }
            vec3d p = m * ca.ccrd() + t;
            if (cart_validator) {
              au.CellToCartesian(p);
            }
            if (!v.IsInside(p)) {
              continue;
            }
            GenerateAtom(ca, *lm);
            if (matrix_created) {
              matrices.Add(m_id, lm);
              matrix_created = false;
            }
          }
          if (matrix_created) {
            delete lm;
          }
        }
      }
    }
  }
  Matrices.SetCapacity(matrices.Count());
  for (size_t i = 0; i < matrices.Count(); i++) {
    Matrices.Add(matrices.GetValue(i));
  }
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& MFrom, const vec3d& MTo,
  TCAtomPList* Template, bool ClearCont, bool atoms)
{
  OnStructureGrow.Enter(this);
  if (ClearCont) {
    ClearAtoms();
    ClearMatrices();
  }
  if (!atoms) {
    Matrices.AddAll(GenerateMatrices(MFrom, MTo));
    Generate(Template, ClearCont);
  }
  else {
    Generate(BoxValidator(MFrom, MTo, true, true),
      Template, ClearCont, false);
  }
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& center, double rad, TCAtomPList* Template,
  bool ClearCont, bool atoms)
{
  OnStructureGrow.Enter(this);
  if (ClearCont) {
    ClearAtoms();
    ClearMatrices();
  }
  if (!atoms) {
    Matrices.AddAll(GenerateMatrices(center, rad));
    Generate(Template, ClearCont);
  }
  else {
    Generate(SphereVolumeValidator(center, rad, true), Template, ClearCont, true);
  }
  OnStructureGrow.Exit(this);
}
//..............................................................................
SortedObjectList<smatd, smatd::ContainerIdComparator>
TLattice::GetFragmentGrowMatrices(const TCAtomPList& l, bool use_q_peaks,
  bool *polymeric) const
{
  SortedObjectList<smatd, smatd::ContainerIdComparator> res;
  const TUnitCell& uc = GetUnitCell();
  res.Add(uc.GetMatrix(0));
  TQueue<const smatd*> q;
  q.Push(&res[0]);
  while (!q.IsEmpty()) {
    const smatd &ref_m = *q.Pop();
    for (size_t j = 0; j < l.Count(); j++) {
      TCAtom& a = *l[j];
      for (size_t k = 0; k < a.AttachedSiteCount(); k++) {
        TCAtom::Site &as = a.GetAttachedSite(k);
        if (as.atom->IsDeleted() ||
          (!use_q_peaks && as.atom->GetType() == iQPeakZ))
        {
          continue;
        }
        smatd m = uc.MulMatrix(as.matrix, ref_m);
        olx_pair_t<size_t, bool> idx = res.AddUnique(m);
        if (idx.b) {
          q.Push(&res[idx.a]);
        }
        else {
          if (polymeric != 0 && !*polymeric && m.t != res[idx.a].t) {
            *polymeric = true;
          }
        }
      }
    }
  }
  return res;
}
//..............................................................................
SortedObjectList<smatd, smatd::IdComparator>
TLattice::GetFragmentGrowMatrices_1(const TCAtomPList& l, bool use_q_peaks) const
{
  SortedObjectList<smatd, smatd::IdComparator> res;
  const TUnitCell& uc = GetUnitCell();
  res.Add(uc.GetMatrix(0));
  TQueue<const smatd*> q;
  q.Push(&res[0]);
  while (!q.IsEmpty()) {
    const smatd &ref_m = *q.Pop();
    for (size_t j = 0; j < l.Count(); j++) {
      TCAtom& a = *l[j];
      for (size_t k = 0; k < a.AttachedSiteCount(); k++) {
        TCAtom::Site &as = a.GetAttachedSite(k);
        if (as.atom->IsDeleted() ||
          (!use_q_peaks && as.atom->GetType() == iQPeakZ))
        {
          continue;
        }
        smatd m = uc.MulMatrix(as.matrix, ref_m);
        // remap translation to [0-1]
        for (int l = 0; l < 3; l++) {
          if (m.t[l] <= -1) {
            m.t[l] -= olx_floor(m.t[l]);
          }
          else if (m.t[l] > 1) {
            m.t[l] -= olx_floor(m.t[l]);
            if (m.t[l] == 0) {
              m.t[l] = 1;
            }
          }
        }
        m.SetId(m.GetContainerId(),
          (m.t - uc.GetMatrix(m.GetContainerId()).t).Round<int>());
        olx_pair_t<size_t, bool> idx = res.AddUnique(m);
        if (idx.b) {
          q.Push(&res[idx.a]);
        }
      }
    }
  }
  return res;
}
//..............................................................................
void TLattice::GetGrowMatrices(smatd_list& res) const {
  const TUnitCell& uc = GetUnitCell();
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.IsGrown() || !sa.IsAvailable() || !sa.CAtom().IsAvailable()) {
      continue;
    }
    const TCAtom& ca = sa.CAtom();
    for (size_t j = 0; j < ca.AttachedSiteCount(); j++) {
      const TCAtom::Site& site = ca.GetAttachedSite(j);
      if (!site.atom->IsAvailable()) {
        continue;
      }
      const smatd m = uc.MulMatrix(site.matrix, sa.GetMatrix());
      bool found = false;
      for (size_t l = 0; l < MatrixCount(); l++) {
        if (Matrices[l]->GetId() == m.GetId()) {
          found = true;
          break;
        }
      }
      if (!found && res.IndexOf(m) == InvalidIndex) {
        res.AddCopy(m);
      }
    }
  }
}
//..............................................................................
void TLattice::DoGrow(const TSAtomPList& atoms, bool GrowShell,
  TCAtomPList* Template)
{
  RestoreCoordinates();
  const TUnitCell& uc = GetUnitCell();
  SortedPointerList<smatd, smatd::IdComparator> matrices;
  matrices.SetCapacity(Matrices.Count());
  for (size_t i = 0; i < Matrices.Count(); i++) {
    matrices.AddUnique(Matrices[i]);
  }
  OnStructureGrow.Enter(this);
  if (GrowShell) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for (size_t j = 0; j < CA.AttachedSiteCount(); j++) {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if (!site.atom->IsAvailable()) {
          continue;
        }
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix());
        if (Objects.atomRegistry.Find(site.atom->GetId(), m.GetId()) != 0) {
          continue;
        }
        size_t mi = matrices.IndexOf(&m);
        smatd *mp;
        if (mi == InvalidIndex) {
          matrices.Add(mp = Matrices.Add(new smatd(m)));
        }
        else {
          mp = matrices[mi];
        }
        GenerateAtom(*site.atom, *mp);
      }
    }
  }
  else {
    // the fragmens to grow by a particular matrix
    olxdict<smatd*, TIntList, TPointerComparator> Fragments2Grow;
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for (size_t j = 0; j < CA.AttachedSiteCount(); j++) {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if (!site.atom->IsAvailable()) {
          continue;
        }
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix());
        size_t mi = matrices.IndexOf(&m);
        smatd *mp;
        if (mi == InvalidIndex) {
          matrices.Add(mp = Matrices.Add(new smatd(m)));
        }
        else {
          mp = matrices[mi];
        }
        Fragments2Grow.Add(mp).Add(site.atom->GetFragmentId());
      }
    }
    for (size_t i = 0; i < Fragments2Grow.Count(); i++) {
      TIntList& ToGrow = Fragments2Grow.GetValue(i);
      for (size_t j = 0; j < GetAsymmUnit().AtomCount(); j++) {
        TCAtom& ca = GetAsymmUnit().GetAtom(j);
        if (ca.IsAvailable() &&
          ToGrow.IndexOf(ca.GetFragmentId()) != InvalidIndex)
        {
          GenerateAtom(ca, *const_cast<smatd *>(Fragments2Grow.GetKey(i)));
        }
      }
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowFragments(bool GrowShells, TCAtomPList* Template) {
  TSAtomPList TmpAtoms;
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& A = Objects.atoms[i];
    if (A.IsDeleted() || !A.CAtom().IsAvailable()) {
      continue;
    }
    for (size_t j = 0; j < A.NodeCount(); j++) {
      if (A.Node(j).IsDeleted()) {
        A.NullNode(j);
      }
    }
    A.PackNodes();
    if (!A.IsGrown()) {
      TmpAtoms.Add(A);
    }
  }
  if (!TmpAtoms.IsEmpty()) {
    GrowAtoms(TmpAtoms, GrowShells, Template);
  }
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, bool GrowShells,
  TCAtomPList* Template)
{
  if (atoms.IsEmpty()) {
    return;
  }
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtom(TSAtom& Atom, bool GrowShells, TCAtomPList* Template) {
  if (Atom.IsGrown()) {
    return;
  }
  DoGrow(TSAtomPList() << Atom, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowFragment(uint32_t FragId, const smatd_list& transforms) {
  olx_pdict<uint32_t, smatd_list> j;
  j.Add(FragId, transforms);
  GrowFragments(j);
}
//..............................................................................
void TLattice::GrowFragments(const olx_pdict<uint32_t, smatd_list> &job) {
  olx_pdict<uint32_t, smatd*> matrix_map;
  // check if the matix is unique
  matrix_map.SetCapacity(Matrices.Count());
  for (size_t i = 0; i < Matrices.Count(); i++)
    matrix_map.Add(Matrices[i]->GetId(), Matrices[i]);
  for (size_t i = 0; i < job.Count(); i++) {
    const smatd_list &l = job.GetValue(i);
    for (size_t j = 0; j < l.Count(); j++) {
      if (!matrix_map.HasKey(l[j].GetId())) {
        matrix_map.Add(l[j].GetId(), Matrices.Add(new smatd(l[j])));
      }
    }
  }
  OnStructureGrow.Enter(this);
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    if (!ca.IsAvailable()) {
      continue;
    }
    size_t fi = job.IndexOf(ca.GetFragmentId());
    if (fi != InvalidIndex) {
      const smatd_list &l = job.GetValue(fi);
      for (size_t j = 0; j < l.Count(); j++) {
        GenerateAtom(ca, *matrix_map.Get(l[j].GetId()));
      }
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TCAtomPList& atoms, const smatd_list& matrices) {
  if (atoms.IsEmpty()) {
    return;
  }
  smatd_plist addedMatrices;
  // check if the matices is unique
  for (size_t i = 0; i < matrices.Count(); i++) {
    bool found = false;
    for (size_t j = 0; j < Matrices.Count(); j++) {
      if (Matrices[j]->GetId() == matrices[i].GetId()) {
        found = true;
        addedMatrices.Add(Matrices[j]);
        break;
      }
    }
    if (!found) {
      addedMatrices.Add(Matrices.Add(new smatd(matrices[i])));
    }
  }
  if (addedMatrices.IsEmpty()) {
    return;
  }
  OnStructureGrow.Enter(this);
  Objects.atoms.IncCapacity(atoms.Count()*addedMatrices.Count());
  for (size_t i = 0; i < addedMatrices.Count(); i++) {
    for (size_t j = 0; j < atoms.Count(); j++) {
      if (atoms[j]->IsAvailable()) {
        GenerateAtom(*atoms[j], *addedMatrices[i]);
      }
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
TSAtom *TLattice::GrowAtom(TCAtom& atom, const smatd& matrix) {
  // check if unique
  TSAtom *a = GetAtomRegistry().Find(atom.GetId(), matrix.GetId());
  if (a != 0 && !a->IsDeleted()) {
    return a;
  }
  smatd* m = 0;
  bool found = false;
  for (size_t i = 0; i < Matrices.Count(); i++) {
    if (Matrices[i]->GetId() == matrix.GetId()) {
      m = Matrices[i];
      found = true;
      break;
    }
  }
  if (!found) {
    m = Matrices.Add(new smatd(matrix));
  }
  OnStructureGrow.Enter(this);
  a = &GenerateAtom(atom, *m);
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
  return a;
}
//..............................................................................
TSAtom& TLattice::GenerateAtom(TCAtom& a, smatd& symop, TNetwork* net) {
  TSAtom& SA = Objects.atoms.New(net == NULL ? Network : net);
  SA.CAtom(a);
  SA._SetMatrix(symop);
  SA.crd() = GetAsymmUnit().Orthogonalise(SA.ccrd() = symop * SA.ccrd());
  SA.SetEllipsoid(&GetUnitCell().GetEllipsoid(
    symop.GetContainerId(), SA.CAtom().GetId()));
  return SA;
}
//..............................................................................
void TLattice::Grow(const smatd& transform) {
  smatd *M = 0;
  // check if the matix is unique
  bool found = false;
  for (size_t i = 0; i < Matrices.Count(); i++) {
    if (Matrices[i]->GetId() == transform.GetId()) {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if (!found) {
    M = Matrices.Add(new smatd(transform));
  }
  OnStructureGrow.Enter(this);
  TAsymmUnit& au = GetAsymmUnit();
  const size_t ac = au.AtomCount();
  Objects.atoms.IncCapacity(ac);
  for (size_t i = 0; i < ac; i++) {
    TCAtom& ca = au.GetAtom(i);
    if (ca.IsAvailable()) {
      GenerateAtom(ca, *M);
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::RestoreAtom(const TSAtom::Ref& id) {
  if (smatd::GetContainerId(id.matrix_id) >= GetUnitCell().MatrixCount()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix ID");
  }
  if (id.atom_id >= GetAsymmUnit().AtomCount()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "catom ID");
  }
  smatd* matr = 0;
  for (size_t i = 0; i < Matrices.Count(); i++) {
    if (Matrices[i]->GetId() == id.matrix_id) {
      matr = Matrices[i];
      break;
    }
  }
  if (matr == 0) {
    matr = Matrices.Add(new smatd(smatd::FromId(id.matrix_id,
      GetUnitCell().GetMatrix(smatd::GetContainerId(id.matrix_id)))));
  }
  vec3d origin_shift;
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    TSAtom & a = Objects.atoms[i];
    if (a.IsAvailable()) {
      origin_shift = a.crd() - GetAsymmUnit().Orthogonalise(a.ccrd());
      break;
    }
  }
  TSAtom& sa = GenerateAtom(GetAsymmUnit().GetAtom(id.atom_id), *matr);
  sa.CAtom().SetDeleted(false);
  sa.crd() += origin_shift;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr& Label) const {
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    if (Label.Equalsi(Objects.atoms[i].GetLabel())) {
      return &Objects.atoms[i];
    }
  }
  return 0;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TCAtom& ca) const {
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    if (ca.GetId() == Objects.atoms[i].CAtom().GetId()) {
      return &Objects.atoms[i];
    }
  }
  return 0;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TSAtom::Ref& id) const {
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    if (Objects.atoms[i] == id) {
      return &Objects.atoms[i];
    }
  }
  return 0;
}
//..............................................................................
TSAtomPList TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  TSAtomPList rv;
  if (Atoms.IsEmpty()) {
    return rv;
  }
  vec3d cc, ce;
  const smatd itm = UnitCell->InvMatrix(Atoms[0]->GetMatrix());
  for (size_t i=0; i < Atoms.Count(); i++) {
    cc += itm*Atoms[i]->ccrd();
    ce += olx_sqr(Atoms[i]->CAtom().ccrdEsd());
  }
  ce.Sqrt();
  ce /= Atoms.Count();
  cc /= Atoms.Count();
  try {
    for (size_t i = 0; i < AsymmUnit->AtomCount(); i++) {
      TCAtom &a = AsymmUnit->GetAtom(i);
      if (a.IsDeleted()) {
        continue;
      }
      if (cc.Equals(a.ccrd(), 1e-3)) {
        return rv;
      }
    }
    TCAtom& CCent = AsymmUnit->NewCentroid(cc);
    GetUnitCell().AddEllipsoid();
    rv.SetCapacity(Matrices.Count());
    vec3d_list crds;
    crds.AddCopy(cc);
    CCent.SetFragmentId((uint32_t)Fragments.Count());
    for (size_t i = 0; i < Matrices.Count(); i++) {
      if (i > 0) {
        cc = (*Matrices[i]) * CCent.ccrd();
        bool found = false;
        for (size_t j = 0; j < crds.Count(); j++) {
          if (cc.Equals(crds[j], 1e-3)) {
            found = true;
            break;
          }
        }
        if (found) {
          continue;
        }
        crds.AddCopy(cc);
      }
      TNetwork *net = new TNetwork(this, Network);
      Fragments.Add(net);
      TSAtom& c = GenerateAtom(CCent, *Matrices[i], net);
      CCent.ccrdEsd() = ce;
      GetUnitCell().AddEllipsoid();
      rv << c;
      net->AddNode(c);
    }
    return rv;
  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//..............................................................................
TSAtom& TLattice::NewAtom(const vec3d& center) {
  TCAtom& ca = AsymmUnit->NewAtom();
  ca.ccrd() = center;
  GetUnitCell().AddEllipsoid();
  return GenerateAtom(ca, *Matrices[0]);
}
//..............................................................................
TSPlanePList TLattice::NewPlane(const TSAtomPList& Atoms, double weightExtent) {
  TXApp& app = TXApp::GetInstance();
  TSPlane* Plane = TmpPlane(Atoms, weightExtent);
  TSPlanePList rv;
  if (Plane != 0) {
    TSPlane::Def pd = Plane->GetDef();
    bool found = false;
    for (size_t i = 0; i < PlaneDefs.Count(); i++) {
      if (PlaneDefs[i] == pd) {
        Plane->_SetDefId(i);
        found = true;
        break;
      }
    }
    if (!found) {
      PlaneDefs.AddCopy(pd);
      if (IsGenerated()) {
        delete Plane;
        for (size_t i = 0; i < Matrices.Count(); i++) {
          TSPlane* p = pd.FromAtomRegistry(app, Objects, PlaneDefs.Count() - 1,
            Network, *Matrices[i]);
          if (p != 0) {
            bool uniq = true;
            for (size_t j = 0; j < Objects.planes.Count() - 1; j++) {
              if (!Objects.planes[j].IsDeleted() &&
                Objects.planes[j].GetCenter().QDistanceTo(
                  p->GetCenter()) < 1e-6 &&
                Objects.planes[j].GetNormal().IsParallel(p->GetNormal()))
              {
                rv << Objects.planes[j];
                uniq = false;
                break;
              }
            }
            if (!uniq) {
              Objects.planes.DeleteLast();
            }
            else {
              rv.Add(p);
            }
          }
        }
      }
      else {
        Objects.planes.Attach(*Plane);
        rv.Add(Plane);
        Plane->_SetDefId(PlaneDefs.Count() - 1);
      }
    }
    else {
      bool uniq = true;
      for (size_t i = 0; i < Objects.planes.Count(); i++) {
        if (!Objects.planes[i].IsDeleted() &&
          Objects.planes[i].GetCenter().QDistanceTo(
            Plane->GetCenter()) < 1e-6 &&
          Objects.planes[i].GetNormal().IsParallel(Plane->GetNormal()))
        {
          rv << Objects.planes[i];
          uniq = false;
          break;
        }
      }
      if (!uniq) {
        delete Plane;
      }
      else {
        rv << Plane;
      }
    }
  }
  return rv;
}
//..............................................................................
TSPlane* TLattice::TmpPlane(const TSAtomPList& atoms, double weightExtent) {
  if (atoms.Count() < 3) {
    return 0;
  }
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList<olx_pair_t<TSAtom*, double> > Points;
  Points.SetCapacity(atoms.Count());
  if (weightExtent != 0) {
    double swg = 0;
    for (size_t i = 0; i < atoms.Count(); i++) {
      const double wght = pow(atoms[i]->GetType().z, weightExtent);
      Points.AddNew(atoms[i], wght);
      swg += wght;
    }
    // normalise the sum of weights to atoms.Count()...
    const double m = atoms.Count() / swg;
    for (size_t i = 0; i < Points.Count(); i++) {
      Points[i].b *= m;
    }
  }
  else {
    for (size_t i = 0; i < atoms.Count(); i++) {
      Points.AddNew(atoms[i], 1);
    }
  }

  Objects.planes.New(Network).Init(Points);
  return &Objects.planes.Detach(Objects.planes.Count() - 1);
}
//..............................................................................
void TLattice::UpdatePlaneDefinitions() {
  PlaneDefs.ForEach(ACollectionItem::TagSetter(0));
  for (size_t i = 0; i < Objects.planes.Count(); i++) {
    TSPlane& sp = Objects.planes[i];
    if (sp.IsDeleted()) {
      continue;
    }
    //check consistency
    if (sp.GetDefId() >= PlaneDefs.Count()) {
      sp.SetDeleted(true);
      continue;
    }
    PlaneDefs[sp.GetDefId()].IncTag();
  }
  TSizeList ids(PlaneDefs.Count());
  size_t id = 0;
  for (size_t i = 0; i < PlaneDefs.Count(); i++) {
    if (PlaneDefs[i].GetTag() != 0) {
      ids[i] = id++;
    }
    else {
      PlaneDefs.NullItem(i);
    }
  }
  for (size_t i = 0; i < Objects.planes.Count(); i++) {
    TSPlane& sp = Objects.planes[i];
    if (sp.IsDeleted()) {
      continue;
    }
    sp._SetDefId(ids[sp.GetDefId()]);
  }
  PlaneDefs.Pack();
}
//..............................................................................
void TLattice::_OnAUChange() {
  for (size_t i = 0; i < PlaneDefs.Count(); i++) {
    PlaneDefs[i].Sort();
  }
}
//..............................................................................
void TLattice::UpdateAsymmUnit() {
  if (Objects.atoms.IsEmpty()) {
    return;
  }
  const size_t ac = GetAsymmUnit().AtomCount();
  TArrayList<TSAtomPList> AUAtoms(ac);
  TSizeList del_cnt(ac);
  for (size_t i = 0; i < ac; i++) {
    del_cnt[i] = 0;
  }
  const size_t lat_ac = Objects.atoms.Count();
  for (size_t i = 0; i < lat_ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.IsDeleted()) {
      del_cnt[sa.CAtom().GetId()]++;
      continue;
    }
    AUAtoms[sa.CAtom().GetId()].Add(sa);
  }
  for (size_t i = 0; i < ac; i++) {  // create lists to store atom groups
    TSAtomPList& l = AUAtoms[i];
    if (del_cnt[i] == 0 && (l.Count() > 1)) {
      continue;  // nothing to do
    }
    TCAtom& ca = AsymmUnit->GetAtom(i);
    if (l.IsEmpty()) {  // all atoms are deleted or none generated
      if (!ca.IsDeleted() && ca.IsAvailable()) {
        ca.SetDeleted(del_cnt[i] != 0);
      }
      continue;
    }
    else if (l.Count() == 1) {  // special case...
      if (l[0]->IsAUAtom()) {
        continue;
      }
      if (l[0]->GetEllipsoid() != 0) {
        ca.UpdateEllp(*l[0]->GetEllipsoid());
      }
      ca.ccrd() = l[0]->ccrd();
      continue;
    }
    // find the original atom, or symmetry equivalent if removed
    // !2011.07.01
    // this bit bites for grown structures - the basis must be changed for all
    // atoms and symetry operators
    //TSAtom* OA = NULL;
    //const size_t lst_c = l.Count();
    //for( size_t j=0; j < lst_c; j++ )  {
    //  TSAtom* A = l[j];
    //  const size_t am_c = A->MatrixCount();
    //  for( size_t k=0; k < am_c; k++ )  {
    //    const smatd& m = A->GetMatrix(k);
    //    if( m.IsFirst() )  {  // the original atom
    //      OA = A;
    //      break;
    //    }
    //  }
    //  if( OA != NULL )  break;
    //}
    //if( OA == NULL )
    //  OA = l[0];
    //ca.SetDeleted(false);
    //if( OA->GetEllipsoid() )
    //  ca.UpdateEllp(*OA->GetEllipsoid());
    //ca.ccrd() = OA->ccrd();
  }
}
//..............................................................................
void TLattice::MoveFragment(const vec3d& to, TSAtom& fragAtom) {
  if (IsGenerated()) {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to, fragAtom.ccrd(), true);
  if (m != 0) {
    for (size_t i = 0; i < fragAtom.GetNetwork().NodeCount(); i++) {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if (SA.CAtom().GetEllipsoid() != 0) {
        *SA.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else {
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
  }
}
//..............................................................................
void TLattice::MoveFragment(TSAtom& to, TSAtom& fragAtom) {
  if (IsGenerated()) {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to.CAtom(), fragAtom.CAtom(), true);
  if (m != 0) {
    if (to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId()) {
      fragAtom.CAtom().ccrd() = *m * fragAtom.CAtom().ccrd();
      if (fragAtom.CAtom().GetEllipsoid() != 0) {
        *fragAtom.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), fragAtom.CAtom().GetId());
      }
    }
    else {  // move whole fragment then
      uint32_t fragId = fragAtom.CAtom().GetFragmentId();
      for (size_t i = 0; i < Objects.atoms.Count(); i++) {
        TSAtom& sa = Objects.atoms[i];
        if (sa.CAtom().GetFragmentId() == fragId) {
          sa.CAtom().ccrd() = *m * sa.CAtom().ccrd();
          if (sa.CAtom().GetEllipsoid() != 0) {
            *sa.CAtom().GetEllipsoid() =
              GetUnitCell().GetEllipsoid(m->GetContainerId(), sa.CAtom().GetId());
          }
        }
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else {
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
  }
}
//..............................................................................
void TLattice::MoveFragmentG(const vec3d& to, TSAtom& fragAtom) {
  vec3d from;
  from = fragAtom.ccrd();
  smatd* m = GetUnitCell().GetClosest(to, from, true);
  vec3d offset;
  if (m != 0) {
    /* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    for (size_t i = 0; i < fragAtom.GetNetwork().NodeCount(); i++) {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if (SA.IsDeleted()) {
        continue;
      }
      GenerateAtom(SA.CAtom(), *m, &SA.GetNetwork());
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else {
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
  }
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom) {
  smatd* m = GetUnitCell().GetClosest(to.ccrd(), fragAtom.ccrd(), true);
  if (m != 0) {
    /* restore atom centres if were changed by some other procedure */
    GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
    RestoreCoordinates();
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    TSAtomPList atoms;
    if (to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId()) {
      atoms.Add(&fragAtom);
    }
    else  // copy whole fragment then
      for (size_t i = 0; i < fragAtom.GetNetwork().NodeCount(); i++) {
        atoms.Add(&fragAtom.GetNetwork().Node(i));
      }

    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom* SA = atoms.GetItem(i);
      if (SA->IsDeleted()) {
        continue;
      }
      GenerateAtom(SA->CAtom(), *m);
    }
    Disassemble();
    OnStructureGrow.Exit(this);
    GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
  }
  else {
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
  }
}
//..............................................................................
void TLattice::MoveToCenter() {
  if (IsGenerated()) {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
  vec3d cnt(0.5),
    ocnt = GetAsymmUnit().Orthogonalise(cnt);
  for (size_t i = 0; i < Fragments.Count(); i++) {
    TNetwork* frag = Fragments[i];
    vec3d molCenter;
    size_t ac = 0;
    for (size_t j = 0; j < frag->NodeCount(); j++) {
      if (frag->Node(j).IsDeleted()) {
        continue;
      }
      molCenter += frag->Node(j).ccrd();
      ac++;
    }
    if (ac == 0) {
      continue;
    }
    molCenter /= ac;
    smatd* m = GetUnitCell().GetClosest(cnt, molCenter, true);
    if (m == 0) {
      vec3i nt = molCenter.Floor<int>();
      if (!nt.IsNull()) {
        m = new smatd(GetUnitCell().GetMatrix(0));
        m->t -= nt;
      }
      else {
        continue;
      }
    }
    else {
      vec3d nc = *m*molCenter;
      double d1 = GetAsymmUnit().Orthogonalise(molCenter).DistanceTo(ocnt);
      double d2 = GetAsymmUnit().Orthogonalise(nc).DistanceTo(ocnt);
      if (olx_abs(d1 - d2) < 1e-6) {
        delete m;
        continue;
      }
      m->t -= nc.Floor<int>();
    }
    for (size_t j = 0; j < frag->NodeCount(); j++) {
      TSAtom& SA = frag->Node(j);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if (SA.CAtom().GetEllipsoid() != 0) {
        *SA.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
    }
    delete m;
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
  GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
}
//..............................................................................
void TLattice::Compaq() {
  if (IsGenerated() || Fragments.Count() < 2) {
    return;
  }
  TNetwork* frag = Fragments[0];
  vec3d acenter;
  size_t ac = 0;
  for (size_t i = 0; i < frag->NodeCount(); i++) {
    if (frag->Node(i).IsDeleted()) {
      continue;
    }
    acenter += frag->Node(i).ccrd();
    ac++;
  }
  if (ac == 0) {
    return;
  }
  GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
  TUnitCell &uc = GetUnitCell();
  acenter /= ac;
  for (size_t i = 1; i < Fragments.Count(); i++) {
    frag = Fragments[i];
    smatd* m = 0;
    for (size_t j = 0; j < Fragments[0]->NodeCount(); j++) {
      TSAtom& fa = Fragments[0]->Node(j);
      for (size_t k = 0; k < frag->NodeCount(); k++) {
        if (frag->Node(k).CAtom().IsAttachedTo(fa.CAtom())) {
          m = uc.GetClosest(fa.ccrd(), frag->Node(k).ccrd(), true);
          if (m != 0) {
            break;
          }
        }
      }
      if (m != 0) {
        break;
      }
    }
    if (m == 0) {
      vec3d molCenter;
      ac = 0;
      for (size_t j = 0; j < frag->NodeCount(); j++) {
        if (frag->Node(j).IsDeleted()) {
          continue;
        }
        molCenter += frag->Node(j).ccrd();
        ac++;
      }
      if (ac == 0) {
        continue;
      }
      molCenter /= ac;
      m = uc.GetClosest(acenter, molCenter, true);
    }
    if (m != 0) {
      for (size_t j = 0; j < frag->NodeCount(); j++) {
        TSAtom& SA = frag->Node(j);
        if (SA.IsDeleted()) {
          continue;
        }
        TCAtom &ca = SA.CAtom();
        ca.ccrd() = *m * ca.ccrd();
        if (SA.GetEllipsoid() != 0) {
          *ca.GetEllipsoid() = uc.GetEllipsoid(m->GetContainerId(), ca.GetId());
        }
      }
      delete m;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
  GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
}
//..............................................................................
int TLattice_CompaqAll_SiteCmp(const TCAtom::Site &s1,
  const TCAtom::Site &s2)
{
  return olx_cmp(s2.atom->GetQPeak(), s1.atom->GetQPeak());
}

size_t TLattice_CompaqAll_Process(TUnitCell& uc, TCAtom& ca,
  const smatd& matr, bool q_peaks)
{
  if (!q_peaks && ca.GetType() == iQPeakZ) {
    return 0;
  }
  size_t cnt = 0;
  ca.SetTag(1);
  TPtrList<TCAtom::Site> sites;
  for (size_t i = 0; i < ca.AttachedSiteCount(); i++) {
    TCAtom::Site& site = ca.GetAttachedSite(i);
    if (site.atom->GetTag() != 0) {
      continue;
    }
    if (!matr.IsFirst()) {
      cnt++;
      site.matrix = uc.MulMatrix(site.matrix, matr);
    }
    else if (site.atom->GetFragmentId() == ca.GetFragmentId() &&
      !site.matrix.IsFirst() && ca.GetType() != iQPeakZ)
    {
      continue;
    }
    site.atom->SetTag(1);
    site.atom->SetFragmentId(ca.GetFragmentId());
    site.atom->ccrd() = site.matrix*site.atom->ccrd();
    if (site.atom->GetEllipsoid() != NULL) {
      *site.atom->GetEllipsoid() = uc.GetEllipsoid(
        site.matrix.GetContainerId(), site.atom->GetId());
    }
    if (site.atom->IsAvailable()) {
      sites << site;
    }
  }
  QuickSorter::SortSF(sites, &TLattice_CompaqAll_SiteCmp);
  for (size_t i = 0; i < sites.Count(); i++) {
    cnt += TLattice_CompaqAll_Process(uc,
      *sites[i]->atom, sites[i]->matrix, q_peaks);
  }
  return cnt;
}
void TLattice_CompaqAll_ProcessRest(TUnitCell& uc, TCAtom& ca)
{
  TPtrList<TCAtom::Site> sites;
  for (size_t i = 0; i < ca.AttachedSiteCount(); i++) {
    TCAtom::Site& site = ca.GetAttachedSite(i);
    if (site.atom->GetTag() != 1) {
      continue;
    }
    double d = 0;
    smatd *m = uc.GetClosest(*site.atom, ca, true, &d);
    if (m == 0) {
      return;
    }
    ca.ccrd() = *m*ca.ccrd();
    if (ca.GetEllipsoid() != NULL) {
      *ca.GetEllipsoid() = uc.GetEllipsoid(m->GetContainerId(), ca.GetId());
    }
    delete m;
    return;
  }
}
void TLattice::CompaqAll() {
  if (IsGenerated()) {
    return;
  }
  TUnitCell& uc = GetUnitCell();
  using namespace olx_analysis;
  TCAtomPList sqp;
  sqp.SetCapacity(Objects.atoms.Count());
  size_t ac = 0, cnt = 0;
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    TCAtom& ca = Objects.atoms[i].CAtom();
    if (ca.GetType() == iQPeakZ) {
      sqp.Add(ca);
    }
    else {
      ac++;
    }
    ca.SetTag(0);
  }
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.CAtom().GetTag() == 0 || sa.CAtom().IsAvailable()) {
      cnt += TLattice_CompaqAll_Process(uc, sa.CAtom(), uc.GetMatrix(0), false);
    }
  }
  if (!sqp.IsEmpty()) {
    TTypeList<peaks::range> peak_ranges = peaks::analyse(sqp);
    for (size_t i = peak_ranges.Count() - 1; i != 0; i--) {
      ac += peak_ranges[i].peaks.Count();
    }
    bool z_processed = true;
    if (uc.CalcVolume() / (ac*uc.MatrixCount()) > 14) {
      for (size_t i = 0; i < peak_ranges[0].peaks.Count(); i++) {
        if (peak_ranges[0].peaks[i]->GetTag() == 0) {
          peak_ranges[0].peaks[i]->SetTag(2);
        }
      }
      z_processed = false;
    }
    for (size_t i = 0; i < sqp.Count(); i++) {
      if (sqp[i]->GetTag() == 0 && sqp[i]->IsAvailable()) {
        TLattice_CompaqAll_Process(uc, *sqp[i], uc.GetMatrix(0), true);
      }
    }
    if (!z_processed) {
      for (size_t i = peak_ranges[0].peaks.Count() - 1; i != InvalidIndex; i--) {
        TCAtom &p = *peak_ranges[0].peaks[i];
        if (p.IsAvailable() && p.GetTag() == 2) {
          TLattice_CompaqAll_ProcessRest(uc, p);
        }
      }
    }
  }
  OnStructureUniq.Enter(this);
  TActionQueueLock __queuelock(&OnStructureUniq);
  Init();
  if (cnt != 0) {
    MoveToCenter();
  }
  __queuelock.Unlock();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqClosest() {
  if (IsGenerated() || Fragments.Count() < 2) {
    return;
  }
  GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
  const size_t fr_cnt = Fragments.Count();
  TDoubleList vminQD(fr_cnt, olx_list_init::value(1e5));
  for (size_t fi = 0; fi < fr_cnt; fi++) {
    TNetwork* neta = Fragments[fi];
    const size_t neta_cnt = neta->NodeCount();
    for (size_t i = fi + 1; i < fr_cnt; i++) {
      TNetwork* netb = Fragments[i];
      const size_t netb_cnt = netb->NodeCount();
      double minQD = 100000;
      smatd* transform = 0;
      for (size_t j = 0; j < neta_cnt; j++) {
        if (!neta->Node(j).CAtom().IsAvailable()) {
          continue;
        }
        const vec3d& crda = neta->Node(j).CAtom().ccrd();
        for (size_t k = 0; k < netb_cnt; k++) {
          if (!netb->Node(k).CAtom().IsAvailable()) {
            continue;
          }
          const vec3d& crdb = netb->Node(k).CAtom().ccrd();
          double qd = 0;
          smatd* m = GetUnitCell().GetClosest(crda, crdb, true, &qd);
          if (m == 0) {
            if (qd < minQD) { // still remember the minimal distance
              minQD = qd;
              if (transform != 0) {  // reset the transform as well
                delete transform;
                transform = 0;
              }
            }
            continue;
          }
          if (qd < minQD) {
            minQD = qd;
            olx_del_obj(transform);
            transform = m;
          }
          else {
            delete m;
          }
        }
      }
      if (vminQD[i] <= minQD) {
        olx_del_obj(transform);
        continue;
      }
      vminQD[i] = minQD;
      if (transform == 0) {
        continue;
      }
      for (size_t k = 0; k < netb_cnt; k++) {
        TSAtom& fb = netb->Node(k);
        if (fb.IsDeleted()) {
          continue;
        }
        fb.CAtom().ccrd() = *transform * fb.CAtom().ccrd();
        if (fb.CAtom().GetEllipsoid() != 0) {
          *fb.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(
            transform->GetContainerId(), fb.CAtom().GetId());
        }
      }
      // this needs to be done if any one fragment is transformed multiple times...
      GetUnitCell().UpdateEllipsoids();
      delete transform;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
  GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
}
//..............................................................................
void TLattice::CompaqType(short type) {
  GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
  const TAsymmUnit& au = GetAsymmUnit();
  const TUnitCell &uc = GetUnitCell();
  size_t ac = au.AtomCount();
  for (size_t i = 0; i < ac; i++) {
    TCAtom& ca = au.GetAtom(i);
    if (ca.GetType() != type) {
      continue;
    }
    smatd* transform = 0;
    double minQD = 1000;
    for (size_t j = 0; j < ac; j++) {
      TCAtom& cb = au.GetAtom(j);
      if (cb.GetType() == type || cb.GetType() == iQPeakZ) {
        continue;
      }
      double qd = 0;
      smatd* m = GetUnitCell().GetClosest(cb.ccrd(), ca.ccrd(), true, &qd);
      if (qd < minQD) {
        olx_del_obj(transform);
        transform = m;
        minQD = qd;
      }
      else if (m != 0) {
        delete m;
      }
    }
    if (transform == 0) {
      continue;
    }
    ca.ccrd() = (*transform * ca.ccrd());
    if (ca.GetEllipsoid() != 0) {
      *ca.GetEllipsoid() = uc.GetEllipsoid(transform->GetContainerId(),
        ca.GetId());
    }
    delete transform;
  }
  // update symm eqivs
  ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.GetType() != type) {
      continue;
    }
    sa.ccrd() = (sa.GetMatrix() * sa.CAtom().ccrd());
    sa.crd() = au.Orthogonalise(sa.ccrd());
  }
  UpdateConnectivityInfo();
  GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms,
  const smatd& transform)
{
  if (IsGenerated()) {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  bool update_equiv = true;
  try {
    smatd m = transform;
    GetUnitCell().InitMatrixId(m);
  }
  catch (const TExceptionBase &e) {
    update_equiv = false;
  }
  /* transform may come with random Tag, so need to process ADP's manually -
  cannot pick from UC
  */
  const mat3d abc2xyz(GetAsymmUnit().GetCellToCartesian().GetT()),
              xyz2abc(GetAsymmUnit().GetCartesianToCell().GetT());
  const mat3d etm = abc2xyz*transform.r*xyz2abc;
  ematd J = TEllipsoid::GetTransformationJ(etm),
    Jt = J.GetTranspose();
  if (update_equiv) {
    GetAsymmUnit().GetRefMod()->BeforeAUUpdate_();
  }
  for (size_t i = 0; i < fragAtoms.Count(); i++) {
    fragAtoms[i]->GetNetwork().SetTag(i);
    fragAtoms[i]->CAtom().ClearChiralFlag();
  }

  for (size_t i=0; i < fragAtoms.Count(); i++) {
    if ((size_t)fragAtoms[i]->GetNetwork().GetTag() == i) {
      for (size_t j=0; j < fragAtoms[i]->GetNetwork().NodeCount(); j++) {
        TSAtom& SA = fragAtoms[i]->GetNetwork().Node(j);
        SA.CAtom().ccrd() = transform * SA.CAtom().ccrd();
        if (SA.CAtom().GetEllipsoid() != 0) {
          SA.CAtom().GetEllipsoid()->Mult(etm, J, Jt);
        }
      }
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
  if (update_equiv) {
    GetAsymmUnit().GetRefMod()->AfterAUUpdate_();
  }
}
//..............................................................................
void TLattice::UpdateConnectivity()  {
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::UpdateConnectivityInfo()  {
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::Disassemble(bool create_planes) {
  if (Objects.atoms.IsEmpty()) {
    return;
  }
  OnDisassemble.Enter(this);
  // clear bonds & fragments
  ClearBonds();
  ClearFragments();
  {
    smatd *I = 0;
    for (size_t i = 0; i < Matrices.Count(); i++) {
      if (Matrices[i]->IsFirst()) {
        I = Matrices[i];
        break;
      }
    }
    if (I == 0) {
      I = Matrices.Add(new smatd());
      I->I().SetId(0);
    }
    for (size_t i = 0; i < Objects.atoms.Count(); i++) {
      TSAtom &a = Objects.atoms[i];
      if (a.CAtom().EquivCount() == 0) {
        continue;
      }
      if (!a.GetMatrix().IsFirst()) {
        for (size_t j = 0; j < a.CAtom().EquivCount(); j++) {
          uint32_t id = GetUnitCell().MulMatrixId(a.CAtom().GetEquiv(j), a.GetMatrix());
          if (smatd::IsFirst(id)) {
            a._SetMatrix(*I);
            break;
          }
        }
      }
    }
  }
  TArrayList<vec3d> ocrd(Objects.atoms.Count());
  GenerateBondsAndFragments(&ocrd);
  if (create_planes) {
    BuildPlanes();
  }
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::RestoreCoordinates() {
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
  }
}
//..............................................................................
size_t TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom,
  TSAtomPList& ProcessingAtoms, bool dry_run,
  int part, TCAtomPList* generated)
{
  if (ProcessingAtoms.IndexOf(atom) != InvalidIndex ||
    (atom.CAtom().IsHAttached() && part == DefNoPart))
  {
    return 0;
  }
  ProcessingAtoms.Add(atom);
  size_t count = 0;
  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  TAtomEnvi AE = UnitCell->GetAtomEnviList(atom, false, part, true);
  if (part == DefNoPart) {  // check for disorder
    TIntList parts;
    TDoubleList occu;
    RefinementModel* rm = GetAsymmUnit().GetRefMod();
    for (size_t i = 0; i < AE.Count(); i++) {
      if (AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() !=
        AE.GetBase().CAtom().GetPart())
      {
        if (parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex) {
          // check if fixed at 1
          double a_occu = rm->Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof);
          int vi = (int)olx_abs(a_occu / 10);
          if (vi == 1 && olx_abs(a_occu - 10) == 1) {
            continue;
          }
          parts.Add(AE.GetCAtom(i).GetPart());
          occu.Add(rm->Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof));
        }
      }
    }
    if (!parts.IsEmpty()) {  // here we go..
      TTypeList<TCAtomPList> gen_atoms;
      ProcessingAtoms.Remove(atom);
      if (parts.Count() > 1) {
        for (size_t i = 0; i < parts.Count(); i++) {
          count += _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, dry_run, parts[i],
            &gen_atoms.AddNew());
          TCAtomPList& gen = gen_atoms.GetLast();
          for (size_t j = 0; j < gen.Count(); j++) {
            gen[j]->SetPart(parts[i]);
            rm->Vars.SetParam(*gen[j], catom_var_name_Sof, occu[i]);
          }
        }
      }
      else if (parts[0] > 0) { // special case with just a single part
        /* if occu is fixed, it is > 5, then we 'invert' the variable like
        21 -> -21, otherwise, just set a value of 1-occu
        */
        int vi = (int)olx_abs(occu[0] / 10);
        double soccu;
        if (vi < 2) {
          soccu = (vi == 0 ? 1 : 11) - olx_abs(occu[0] - vi * 10);
        }
        else {
          soccu = -occu[0];
        }
        // do not generate 0-occupancy atoms
        if (soccu != 0 && soccu != 10) {
          count += _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, dry_run, 0,
            &gen_atoms.AddNew());
          TCAtomPList& gen = gen_atoms.GetLast();
          int spart = (parts[0] == 2 ? 1 : olx_abs(parts[0]) + 1);
          for (size_t j = 0; j < gen.Count(); j++) {
            gen[j]->SetPart(spart);
            rm->Vars.SetParam(*gen[j], catom_var_name_Sof, soccu);
          }
        }
      }
      cg.AnalyseMultipart(AE, gen_atoms);
      return count;
    }
  }
  if (atom.GetType() == iCarbonZ) {
    if (AE.Count() == 1) {
      // check acetylene
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      TSAtom* A = FindSAtom(AE.GetCAtom(0));
      if (A == 0) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Could not locate atom ").quote() << AE.GetLabel(0));
      }

      TAtomEnvi NAE = UnitCell->GetAtomEnviList(*A, false, part);
      if (A->GetType() == iCarbonZ && NAE.Count() == 2 && d < 1.2) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH";
          cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
        }
        count += 1;
      }
      else {
        if (d > 1.35) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH3";
            cg.FixAtom(AE, fgCH3, h_elm, NULL, generated);
          }
          count += 3;
        }
        else {
          if (d < 1.35) {
            if (NAE.Count() > 1) {
              bool done = false;
              if (NAE.Count() == 2 && d < 1.2) { // check acetylene again
                vec3d v = (NAE.GetCAtom(0) == atom.CAtom() ? NAE.GetCrd(1)
                  : NAE.GetCrd(0));
                double ang = (v - NAE.GetBase().crd())
                  .CAngle(atom.crd() - NAE.GetBase().crd());
                if ((ang + 1) < 0.03) {
                  if (!dry_run) {
                    TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                      ": XCH";
                    cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
                  }
                  count += 1;
                  done = true;
                }
              }
              if (!done) {
                if (!dry_run) {
                  TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                    ": X=CH2";
                  cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
                }
                count += 2;
              }
            }
            else {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                ": possibly X=CH2";
            }
          }
        }
      }
    }
    else  if (AE.Count() == 2) {
      const double db = 1.41;
      vec3d a = AE.GetCrd(0);
      a -= atom.crd();
      vec3d b = AE.GetCrd(1);
      b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v) * 180 / M_PI;
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if (d1 > db && d2 > db && v < 125) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYCH2";
          cg.FixAtom(AE, fgCH2, h_elm, 0, generated);
        }
        count += 2;
      }
      else {
        if ((d1 < db || d2 < db) && v < 160) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X(Y=C)H";
            cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
          }
          count += 1;
        }
      }
    }
    else if (AE.Count() == 3) {
      double v = 0;
      try {
        v = olx_tetrahedron_volume(
          atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2));
      }
      catch (const TDivException& e) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logWarning) << "Check environment parts/connectivity for "
            << atom.GetLabel();
        }
      }
      if (v > 0.3) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZCH";
          cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
        }
        count += 1;
      }
    }
    else if (AE.Count() == 5) {  // carboranes ...
      //check
      if (TSPlane::CalcRMSD(AE) < 0.1) {
        bool proceed = false;
        for (size_t j = 0; j < AE.Count(); j++) {
          if (AE.GetType(j) == iBoronZ) {
            proceed = true;
            break;
          }
        }
        if (proceed) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R5CH";
            cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
          }
          count += 1;
        }
      }
    }
  }
  else if (atom.GetType() == iNitrogenZ) {  // nitrogen
    if (AE.Count() == 1) {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if (d > 1.35) {
        if (d > 1.44) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH3";
            cg.FixAtom(AE, fgNH3, h_elm, NULL, generated);
          }
          count += 3;
        }
        else {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH2";
            cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
          }
          count += 2;
        }
      }
      else  if (d > 1.2) {  //else nitrile
        // have to check if double bond
        TSAtom* A = FindSAtom(AE.GetCAtom(0));
        if (A == 0) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("Could not locate atom ").quote() << AE.GetLabel(0));
        }
        TAtomEnvi NAE = UnitCell->GetAtomEnviList(*A, false, part);
        NAE.Exclude(atom.CAtom());

        if (A->GetType() == iCarbonZ && NAE.Count() > 1) {
          vec3d a = NAE.GetCrd(0);
          a -= NAE.GetBase().crd();
          vec3d b = AE.GetBase().crd();
          b -= NAE.GetBase().crd();

          d = a.CAngle(b);
          d = acos(d) * 180 / M_PI;
          if (d > 115 && d < 130) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH2";
              cg.FixAtom(AE, fgNH2, h_elm, &NAE, generated);
            }
            count += 2;
          }
        }
        else {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH";
            cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
          }
          count += 1;
        }
      }
    }
    else  if (AE.Count() == 2) {
      vec3d a = AE.GetCrd(0);
      a -= atom.crd();
      vec3d b = AE.GetCrd(1);
      b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v) * 180 / M_PI;
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if (d1 > 1.72 || d2 > 1.72) {  // coordination?
        if (v > 165) { // skip ..
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RN->M";
        }
        else if ((d1 < 1.5 && d1 > 1.35) || (d2 < 1.5 && d2 > 1.35)) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RNH(2)M";
            cg.FixAtom(AE, fgNH2, h_elm, 0, generated);
          }
          count += 2;
        }
        else if (d1 > 1.72 && d2 > 1.72) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XX'NH";
            cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
          }
          count += 1;
        }
      }
      else if (v < 121 && d1 > 1.45 && d2 > 1.45) {
        bool two = (AE.GetType(0) == iCarbonZ || AE.GetType(0) == iNitrogenZ) &&
          (AE.GetType(1) == iCarbonZ || AE.GetType(1) == iNitrogenZ);
        if (two) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYNH2+";
            cg.FixAtom(AE, fgNH2, h_elm, 0, generated);
          }
          count += 2;
        }
        else {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYNH";
            cg.FixAtom(AE, fgNH1, h_elm, 0, generated);
          }
          count += 1;
        }
      }
      else if (v > 118 && d1 > 1.33 && d2 > 1.33) {
        bool has_do = false;
        for (size_t ei = 0; ei < 2; ei++) {
          for (size_t si = 0; si < AE.GetCAtom(ei).AttachedSiteCount(); si++) {
            TCAtom::Site& s = AE.GetCAtom(ei).GetAttachedSite(si);
            if (s.atom->GetId() == AE.GetBase().CAtom().GetId() || s.atom->IsDeleted()) {
              continue;
            }
            if (s.atom->GetType() == iOxygenZ) {
              double d = GetAsymmUnit().Orthogonalise(
                s.matrix * s.atom->ccrd() - AE.GetCAtom(ei).ccrd()).Length();
              if (d < 1.3) {
                has_do = true;
                break;
              }
            }
          }
          if (has_do) {
            break;
          }
        }
        if (!has_do) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": ArNH";
            cg.FixAtom(AE, fgNH1, h_elm, 0, generated);
          }
          count += 1;
        }
        else if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": Ar(C=O)N, skipping";
        }
      }
      else {
        if ((d1 + d2) > 2.70 && v < 140) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYNH";
            if (d1 > 1.4 && d2 > 1.4) {
              cg.FixAtom(AE, fgNH1t, h_elm, 0, generated);
            }
            else {
              cg.FixAtom(AE, fgNH1, h_elm, 0, generated);
            }
          }
          count += 1;
        }
      }
    }
    else if (AE.Count() == 3) {
      // remove coordination bond ...
      vec3d a = AE.GetCrd(0);
      a -= atom.crd();
      vec3d b = AE.GetCrd(1);
      b -= atom.crd();
      vec3d c = AE.GetCrd(2);
      c -= atom.crd();
      double v1 = a.CAngle(b);  v1 = acos(v1) * 180 / M_PI;
      double v2 = a.CAngle(c);  v2 = acos(v2) * 180 / M_PI;
      double v3 = b.CAngle(c);  v3 = acos(v3) * 180 / M_PI;
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      double d3 = AE.GetCrd(2).DistanceTo(atom.crd());
      if ((v1 + v2 + v3) < 350 && d1 > 1.45 && d2 > 1.45 && d3 > 1.45) {
        if (d1 > 1.75 || d2 > 1.75 || d3 > 1.75) {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R2HN->M";
            cg.FixAtom(AE, fgNH1, h_elm, 0, generated);
          }
          count += 1;
        }
        else if (cg.Options.GetBoolOption("nr3", false, true)) {
          // this excludes P-N bonds, http://www.olex2.org/olex2-bugs/359
          if (d1 < 1.65 && d2 < 1.65 && d3 < 1.65 &&
            d1 >= 1.49 && d2 >= 1.49 && d3 >= 1.49)
          {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R3NH+";
              cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
            }
            count += 1;
          }
        }
      }
    }
  }
  if (atom.GetType() == iOxygenZ) {  // oxygen
    if (AE.IsEmpty()) {
      if (!dry_run) {
        TAtomEnvi pivoting = UnitCell->GetAtomPossibleHBonds(AE);
        UnitCell->FilterHBonds(AE, pivoting, true);
        RemoveNonHBonding(pivoting, 2);
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": OH2";
        cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
      }
      count += 2;
    }
    else if (AE.Count() == 1) {
      const double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if (d > 1.3) {  // otherwise a double bond
        TAtomEnvi pivoting = UnitCell->GetAtomPossibleHBonds(AE);
        // d < 1.8 - move bonds only if not coordination
        UnitCell->FilterHBonds(AE, pivoting, d < 1.8);
        RemoveNonHBonding(pivoting, d > 1.8 ? 2 : 1);
        if (AE.GetType(0) == iChlorineZ) {
          ;
        }
        else  if (AE.GetType(0) == iCarbonZ) {  // carbon
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": COH";
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
          count += 1;
        }
        else  if (AE.GetType(0) == iSulphurZ) {
          if (d > 1.48) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SOH";
              cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
            }
            count += 1;
          }
        }
        else  if (AE.GetType(0) == iPhosphorusZ) {
          if (d > 1.54) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": POH";
              cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
            }
            count += 1;
          }
        }
        else  if (AE.GetType(0) == iSiliconZ) {
          if (d > 1.6) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SiOH";
              cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
            }
            count += 1;
          }
        }
        else  if (AE.GetType(0) == iBoronZ) {
          if (d < 1.38) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": B(III)OH";
              cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
            }
            count += 1;
          }
        }
        else  if (AE.GetType(0) == iNitrogenZ) {
          if (d > 1.37) {
            if (!dry_run) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": NOH";
              cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
            }
            count += 1;
          }
        }
        else if (d > 1.8) {  // coordination bond?
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
              ": possibly M-OH2";
            cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
          }
          count += 2;
        }
      }
    }
    else if (AE.Count() == 2) {
      const double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      const double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if ((d1 > 1.8 && d2 < 1.8 && d2 > 1.38) ||
        (d2 > 1.8 && d1 < 1.8 && d1 > 1.38))
      {
        if (!dry_run) {
          TAtomEnvi pivoting = UnitCell->GetAtomPossibleHBonds(AE);
          UnitCell->FilterHBonds(AE, pivoting, false);
          RemoveNonHBonding(pivoting, 2);
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
            ": possibly M-O(H)R";
          if (pivoting.Count() == 1) {
            AE.Delete(d1 > 1.8 ? 0 : 1);
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
          else {
            cg.FixAtom(AE, fgOH1, h_elm, 0, generated);
          }
        }
        count += 1;
      }
    }
    else if (AE.Count() == 3) {
      if (XElementLib::IsTransitionalMetal(AE.GetType(0)) &&
        XElementLib::IsTransitionalMetal(AE.GetType(1)) &&
        XElementLib::IsTransitionalMetal(AE.GetType(2)))
      {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
            ": possibly M3-O(H)";
          cg.FixAtom(AE, fgOH1, h_elm, 0, generated);
        }
        count += 1;
      }
    }
  }
  else if (atom.GetType() == iBoronZ) {  // boron
    if (AE.Count() == 3) {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume(
        cnt,
        (AE.GetCrd(0) - cnt).Normalise() + cnt,
        (AE.GetCrd(1) - cnt).Normalise() + cnt,
        (AE.GetCrd(2) - cnt).Normalise() + cnt);
      if (v > 0.1) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZBH";
          cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
        }
        count += 1;
      }
    }
    else if (AE.Count() == 4) {
      vec3d a, b;
      double sumAng = 0;
      for (size_t i = 0; i < AE.Count(); i++) {
        a = AE.GetCrd(i);
        a -= atom.crd();
        for (size_t j = i + 1; j < AE.Count(); j++) {
          b = AE.GetCrd(j);
          b -= atom.crd();
          double ca = b.CAngle(a);
          if (ca < -1)  ca = -1;
          if (ca > 1)   ca = 1;
          sumAng += acos(ca);
        }
      }
      if (sumAng * 180 / M_PI > 700) {   //!! not sure it works, lol
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X4BH";
          cg.FixAtom(AE, fgBH1, h_elm, 0, generated);
        }
        count += 1;
      }
    }
    else if (AE.Count() == 5) {
      if (!dry_run) {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X5BH";
        cg.FixAtom(AE, fgBH1, h_elm, 0, generated);
      }
      count += 1;
    }
  }
  else if (atom.GetType() == iSiliconZ) {
    if (AE.Count() == 3) {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume(
        cnt,
        (AE.GetCrd(0) - cnt).Normalise() + cnt,
        (AE.GetCrd(1) - cnt).Normalise() + cnt,
        (AE.GetCrd(2) - cnt).Normalise() + cnt);
      if (v > 0.1) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZSiH";
          cg.FixAtom(AE, fgSiH1, h_elm, 0, generated);
        }
        count += 1;
      }
    }
    else if (AE.Count() == 2) {  // no validation yet...
      if (!dry_run) {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYSiH2";
        cg.FixAtom(AE, fgSiH2, h_elm, 0, generated);
      }
      count += 2;
    }
  }
  else if (atom.GetType() == iSulphurZ) {
    if (AE.Count() == 1 && AE.GetType(0) == iCarbonZ) {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if (d > 1.72) {
        if (!dry_run) {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": CSH";
          cg.FixAtom(AE, fgSH1, h_elm, 0, generated);
        }
        count += 1;
      }
    }
  }
  ProcessingAtoms.Delete(ProcessingAtoms.IndexOf(&atom));
  return count;
}
//..............................................................................
size_t TLattice::_ProcessRingHAdd(AConstraintGenerator& cg,
  const ElementPList& rcont, const TSAtomPList& atoms, bool dry_run)
{
  TTypeList<TSAtomPList> rings;
  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  for (size_t i = 0; i < FragmentCount(); i++) {
    GetFragment(i).FindRings(rcont, rings);
  }
  size_t count = 0;
  for( size_t i=0; i < rings.Count(); i++ )  {
    double rms = TSPlane::CalcRMSD(rings[i]);
    if( rms < 0.05 && TNetwork::IsRingRegular( rings[i]) )  {
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        TAtomEnvi AE = UnitCell->GetAtomEnviList(*rings[i][j]);
        if( AE.Count() == 3 )  {
          const vec3d cnt = AE.GetBase().crd();
          try  {
            const double v = olx_tetrahedron_volume(
              cnt,
              (AE.GetCrd(0)-cnt).Normalise() + cnt,
              (AE.GetCrd(1)-cnt).Normalise() + cnt,
              (AE.GetCrd(2)-cnt).Normalise() + cnt);
            if( v < 0.1 )  continue;  // coordination or substituted
          }
          catch(...)  {  continue;  }
        }
        for( size_t k=0; k < AE.Count(); k++ )  {
          if ((AE.GetCrd(k) - rings[i][j]->crd()).QLength() > 4.0) {
            AE.Delete(k--);
          }
        }
        if( AE.Count() == 2 && rings[i][j]->GetType() == iCarbonZ &&
            atoms.IndexOf(AE.GetBase()) != InvalidIndex )
        {
          if (!dry_run) {
            TBasicApp::NewLogEntry(logInfo) << rings[i][j]->GetLabel() <<
              ": X(Y=C)H (ring)";
            cg.FixAtom(AE, fgCH1, h_elm);
          }
          rings[i][j]->CAtom().SetHAttached(true);
          count++;
        }
      }
    }
  }
  return count;
}
//..............................................................................
size_t TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms,
  bool dry_run)
{
  if (atoms.IsEmpty()) {
    return 0;
  }
  ElementPList CTypes;
  CTypes.Add(XElementLib::FindBySymbol("C"));
  CTypes.Add(XElementLib::FindBySymbol("N"));
  CTypes.Add(XElementLib::FindBySymbol("O"));
  CTypes.Add(XElementLib::FindBySymbol("B"));
  CTypes.Add(XElementLib::FindBySymbol("Si"));
  CTypes.Add(XElementLib::FindBySymbol("S"));
  TSAtomPList ProcessingAtoms;

  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->CAtom().SetHAttached(false);
  }

  // treat rings
  ElementPList rcont;
  rcont.Add(CTypes[0]);
  for (size_t i = 0; i < 4; i++) {
    rcont.Add(rcont[0]);
  }
  size_t count = 0;
  count += _ProcessRingHAdd(cg, rcont, atoms, dry_run); // Cp
  rcont.Add(rcont[0]);
  count += _ProcessRingHAdd(cg, rcont, atoms, dry_run); // Ph
  rcont.GetLast() = CTypes[1];
  count += _ProcessRingHAdd(cg, rcont, atoms, dry_run); // Py

  TAsymmUnit &au = GetAsymmUnit();
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
  TSAtomPList waters;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->IsDeleted() || !atoms[i]->CAtom().IsAvailable() ||
      atoms[i]->CAtom().GetTag() != 0)
    {
      continue;
    }
    // mark the atoms processed
    atoms[i]->CAtom().SetTag(1);
    bool consider = false;
    for (size_t j = 0; j < CTypes.Count(); j++) {
      if (atoms[i]->GetType() == *CTypes[j]) {
        consider = true;
        break;
      }
    }
    if (!consider) {
      continue;
    }
    for (size_t j = 0; j < atoms[i]->NodeCount(); j++) {
      TSAtom& A = atoms[i]->Node(j);
      if (A.IsDeleted()) {
        continue;
      }
      if (A.GetType() == iHydrogenZ) {
        consider = false;
        break;
      }
    }
    for (size_t j = 0; j < atoms[i]->CAtom().AttachedSiteCount(); j++) {
      if (atoms[i]->CAtom().GetAttachedAtom(j).GetType() == iHydrogenZ &&
        !atoms[i]->CAtom().GetAttachedAtom(j).IsDeleted())
      {
        consider = false;
        break;
      }
    }
    if (!consider) {
      continue;
    }
    if (atoms[i]->GetType() == iOxygenZ) {
      size_t nhc = 0;
      for (size_t si = 0; si < atoms[i]->CAtom().AttachedSiteCount(); si++) {
        TCAtom::Site &s = atoms[i]->CAtom().GetAttachedSite(si);
        if (s.atom->IsDeleted() || s.atom->GetType().z < 2) {
          continue;
        }
        nhc++;
      }
      if (nhc == 0) {
        waters << atoms[i];
        atoms[i]->CAtom().SetTag(1);
        continue;
      }
    }
    count += _AnalyseAtomHAdd(cg, *atoms[i], ProcessingAtoms, dry_run);
  }
  for (size_t i = 0; i < waters.Count(); i++) {
    count += _AnalyseAtomHAdd(cg, *waters[i], ProcessingAtoms, dry_run);
  }
  /* // this might be useful for hadd on grown structures
  GetUnitCell().AddEllipsoid(au.AtomCount()-au_cnt);
  GetUnitCell().FindSymmEq();
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(-1));
  for( size_t i=au_cnt; i < au.AtomCount(); i++ )
    GenerateAtom(au.GetAtom(i), *Matrices[0]);
    */
  return count;
}
//..............................................................................
void TLattice::RemoveNonHBonding(TAtomEnvi& Envi, size_t max) {
  // find pivots linked through a metal and remove them
  for (size_t i = 0; i < Envi.Count(); i++) {
    TSAtom* SA = FindSAtom(Envi.GetCAtom(i));
    TAtomEnvi AE = UnitCell->GetAtomEnviList(*SA);
    bool remove = false;
    for (size_t j = 0; j < AE.Count(); j++) {
      if (XElementLib::IsMetal(AE.GetCAtom(j).GetType())) {
        for (size_t k = 0; k < AE.GetCAtom(j).AttachedSiteCount(); k++) {
          const TCAtom::Site& s = AE.GetCAtom(j).GetAttachedSite(k);
          if (s.atom->GetId() == Envi.GetBase().CAtom().GetId()) {
            double d = GetAsymmUnit().Orthogonalise(
              s.matrix * s.atom->ccrd() - Envi.GetBase().ccrd()).QLength();
            if (d < 1e-3) {
              remove = true;
              break;
            }
          }
          if (remove) {
            break;
          }
        }
      }
      if (remove) {
        break;
      }
    }
    if (remove) {
      Envi.Delete(i--);
      continue;
    }
    if (SA->GetType() == iOxygenZ) {
      if (AE.Count() == 1) {
        const double d = AE.GetCrd(0).DistanceTo(SA->crd());
        if (d > 1.8 && XElementLib::IsMetal(SA->GetType())) { // coordination bond?
          Envi.Exclude(SA->CAtom());
          i--;
        }
      }
      else if (AE.Count() == 2) {  // not much can be done here ... needs thinking
        //Envi.Exclude( SA->CAtom() );
        // commented 17.03.08, just trying to what the shortest distance will give
      }
      //else if( AE.Count() == 3 )  {  // coordinated water molecule
      //  Envi.Exclude(SA->CAtom());
      //}
    }
    else if (SA->GetType() == iNitrogenZ) {
      if (AE.Count() > 3) {
        Envi.Exclude(SA->CAtom());
        i--;
      }
    }
  }
  // all similar length  .... Q peaks might help :)
  if (Envi.Count() > max) {
    TAtomEnvi AE = UnitCell->GetAtomQEnviList(Envi.GetBase());
    TSizeList to_exclude;
    for (size_t i = 0; i < AE.Count(); i++) {
      const double d = Envi.GetBase().crd().DistanceTo(AE.GetCrd(i));
      if (d < 0.65 || d > 1.2) {
        to_exclude.Add(i);
      }
    }
    AE.ExcludeIndices(to_exclude);

    TSizeList to_leave;
    for (size_t ei = 0; ei < AE.Count(); ei++) {
      vec3d vec1 = AE.GetCrd(ei) - Envi.GetBase().crd();
      double min_v = 1e6;
      size_t min_idx = 0;
      for (size_t i = 0; i < Envi.Count(); i++) {
        vec3d vec2 = Envi.GetCrd(i) - Envi.GetBase().crd();
        double sv = olx_abs(acos(vec2.CAngle(vec1)));
        if (sv < min_v) {
          min_idx = i;
          min_v = sv;
        }
      }
      to_leave.Add(min_idx);
    }
    if (AE.Count() > 0) {
      Envi.LeaveIndices(to_leave);
    }
  }
  // choose the shortest bonds ...
  if (Envi.Count() > max) {
    sorted::PrimitiveAssociation<double, size_t> hits;
    for (size_t i = 0; i < Envi.Count(); i++) {
      double d = Envi.GetBase().crd().DistanceTo(Envi.GetCrd(i));
      // prioritise sligtly longer intramolecular bonds
      if (Envi.GetMatrix(i).IsFirst() &&
        Envi.GetBase().CAtom().GetFragmentId() == Envi.GetCAtom(i).GetFragmentId())
      {
        d -= 0.15;
      }
      hits.Add(d, i);
    }
    TSizeList to_exclude;
    while (hits.Count() > max &&
      ((hits.GetLastKey() - hits.GetKey(0)) > 0.15))
    {
      to_exclude.Add(hits.GetValue(hits.Count() - 1));
      hits.Delete(hits.Count() - 1);
    }
    Envi.ExcludeIndices(to_exclude);
  }
  Envi.SortByDistance();
}
//..............................................................................
void TLattice::SetAnis(const TCAtomPList& atoms, bool anis, int anharmonic) {
  if (atoms.IsEmpty()) {
    return;
  }
  if (!anis) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (olx_is_valid_index(atoms[i]->GetEllpId())) {
        GetAsymmUnit().NullEllp(atoms[i]->GetEllpId());
        atoms[i]->AssignEllp(0);
      }
    }
    GetAsymmUnit().PackEllps();
  }
  else {
    evecd ee(6);
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetEllipsoid() == 0) {
        ee[0] = ee[1] = ee[2] = atoms[i]->GetUiso();
        atoms[i]->UpdateEllp(ee);
      }
      if (anharmonic > 2) {
        atoms[i]->GetEllipsoid()->SetAnharmonicPart(new GramCharlier(anharmonic));
      }
      else {
        atoms[i]->GetEllipsoid()->SetAnharmonicPart(0);
      }
    }
  }
  GetUnitCell().UpdateEllipsoids();
  RestoreADPs(false);
}
//..............................................................................
void TLattice::ToDataItem(TDataItem& item) const {
  TXApp& app = TXApp::GetInstance();
  item.AddField("delta", Delta);
  item.AddField("deltai", DeltaI);
  GetAsymmUnit().ToDataItem(item.AddItem("AUnit"));
  TDataItem& mat = item.AddItem("Matrices");
  const size_t mat_c = Matrices.Count();
  /* save matrices, change matrix tags to the position in the list and remember
    old tags
  */
  TArrayList<uint32_t> m_tags(mat_c);
  for (size_t i = 0; i < mat_c; i++) {
    mat.AddItem("symop", TSymmParser::MatrixToSymmEx(*Matrices[i]))
      .AddField("id", Matrices[i]->GetId());
    m_tags[i] = Matrices[i]->GetId();
    Matrices[i]->SetRawId((uint32_t)i);
  }
  // initialise bond tags
  size_t sbond_tag = 0;
  for (size_t i = 0; i < Objects.bonds.Count(); i++) {
    if (Objects.bonds[i].IsDeleted()) {
      continue;
    }
    Objects.bonds[i].SetTag(sbond_tag++);
  }
  // initialise atom tags
  size_t satom_tag = 0;
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    if (Objects.atoms[i].IsDeleted()) {
      continue;
    }
    Objects.atoms[i].SetTag(satom_tag++);
  }
  // initialise fragment tags
  size_t frag_tag = 0;
  Network->SetTag(-1);
  for (size_t i = 0; i < Fragments.Count(); i++) {
    Fragments[i]->SetTag(frag_tag++);
  }
  // save satoms - only the original CAtom Tag and the generating matrix tag
  TDataItem& atoms = item.AddItem("Atoms");
  for (size_t i = 0; i < Objects.atoms.Count(); i++) {
    if (!Objects.atoms[i].IsDeleted()) {
      Objects.atoms[i].ToDataItem(atoms.AddItem("Atom"));
    }
  }
  // save bonds
  TDataItem& bonds = item.AddItem("Bonds");
  for (size_t i = 0; i < Objects.bonds.Count(); i++) {
    if (!Objects.bonds[i].IsDeleted()) {
      Objects.bonds[i].ToDataItem(bonds.AddItem("Bond"));
    }
  }
  // save fragments
  TDataItem& frags = item.AddItem("Fragments");
  for (size_t i = 0; i < Fragments.Count(); i++) {
    Fragments[i]->ToDataItem(frags.AddItem("Fragment"));
  }
  // restore original matrix tags
  for (size_t i = 0; i < mat_c; i++) {
    Matrices[i]->SetRawId(m_tags[i]);
  }
  // save planes
  TSPlanePList valid_planes;
  for (size_t i = 0; i < Objects.planes.Count(); i++) {
    if (Objects.planes[i].IsDeleted()) {
      continue;
    }
    size_t p_ac = 0;
    for (size_t j = 0; j < Objects.planes[i].Count(); j++)
      if (Objects.planes[i].GetAtom(j).IsAvailable()) {
        p_ac++;
      }
    if (p_ac >= 3) {// a plane must contain at least three atoms
      valid_planes.Add(Objects.planes[i]);
    }
    else {
      Objects.planes[i].SetDeleted(true);
    }
  }
  TPtrList<TSPlane::Def> defs;
  TDataItem& planes = item.AddItem("Planes");
  for (size_t i = 0; i < valid_planes.Count(); i++) {
    size_t di = defs.IndexOf(PlaneDefs[valid_planes[i]->GetDefId()]);
    if (di == InvalidIndex) {
      di = defs.Count();
      defs.Add(PlaneDefs[valid_planes[i]->GetDefId()]);
    }
    size_t odi = valid_planes[i]->GetDefId();
    valid_planes[i]->_SetDefId(di);
    valid_planes[i]->ToDataItem(planes.AddItem("Plane"), app);
    valid_planes[i]->_SetDefId(odi);
  }
  TDataItem &plane_defs = item.AddItem("Plane_defs");
  for (size_t i = 0; i < defs.Count(); i++) {
    defs[i]->ToDataItem(plane_defs.AddItem("Def"), app);
  }
}
//..............................................................................
void TLattice::FromDataItem(const TDataItem& item) {
  TActionQueueLock ql(&OnAtomsDeleted);
  Clear(true);
  ClearPlaneDefinitions();
  Delta = item.GetFieldByName("delta").ToDouble();
  DeltaI = item.GetFieldByName("deltai").ToDouble();
  GetAsymmUnit().FromDataItem(item.GetItemByName("AUnit"));
  GetUnitCell().InitMatrices();
  const TDataItem& mat = item.GetItemByName("Matrices");
  Matrices.SetCapacity(mat.ItemCount());
  for (size_t i = 0; i < mat.ItemCount(); i++) {
    smatd* m = new smatd(
      TSymmParser::SymmToMatrix(mat.GetItemByIndex(i).GetValue()));
    GetUnitCell().InitMatrixId(*Matrices.Add(m));
    m->SetRawId(mat.GetItemByIndex(i).GetFieldByName("id").ToUInt());
  }
  // precreate fragments
  const TDataItem& frags = item.GetItemByName("Fragments");
  Fragments.SetCapacity(frags.ItemCount());
  for (size_t i = 0; i < frags.ItemCount(); i++) {
    Fragments.Add(new TNetwork(this, 0));
  }
  // precreate bonds
  const TDataItem& bonds = item.GetItemByName("Bonds");
  Objects.bonds.IncCapacity(bonds.ItemCount());
  for (size_t i = 0; i < bonds.ItemCount(); i++) {
    Objects.bonds.New(0);
  }
  // precreate and load atoms
  const TDataItem& atoms = item.GetItemByName("Atoms");
  Objects.atoms.IncCapacity(atoms.ItemCount());
  for (size_t i = 0; i < atoms.ItemCount(); i++) {
    Objects.atoms.New(0);
  }
  for (size_t i = 0; i < atoms.ItemCount(); i++) {
    Objects.atoms[i].FromDataItem(atoms.GetItemByIndex(i), *this);
  }
  // load bonds
  for (size_t i = 0; i < bonds.ItemCount(); i++) {
    Objects.bonds[i].FromDataItem(bonds.GetItemByIndex(i), *this);
  }
  // load fragments
  for (size_t i = 0; i < frags.ItemCount(); i++) {
    Fragments[i]->FromDataItem(frags.GetItemByIndex(i));
  }
}
//..............................................................................
void TLattice::FinaliseLoading() {
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    GetAsymmUnit().GetAtom(i).SetDeleted(false);
  }
  for (size_t i = 0; i < Fragments.Count(); i++)  {
    Fragments[i]->SetOwnerId(i);
    for (size_t j = 0; j < Fragments[i]->NodeCount(); j++) {
      Fragments[i]->Node(j).CAtom().SetFragmentId((uint32_t)i);
    }
  }
  BuildAtomRegistry();
}
//..............................................................................
void TLattice::LoadPlanes_(const TDataItem& item, bool rebuild_defs) {
  TDataItem* plane_defs = item.FindAnyItem("Plane_defs");
  const TXApp& app = TXApp::GetInstance();
  if (plane_defs != 0 && !rebuild_defs) {
    for (size_t i = 0; i < plane_defs->ItemCount(); i++) {
      PlaneDefs.AddNew(plane_defs->GetItemByIndex(i), app);
    }
  }
  TDataItem& planes = item.GetItemByName("Planes");
  for (size_t i = 0; i < planes.ItemCount(); i++) {
    TSPlane& p = Objects.planes.New(Network);
    p.FromDataItem(planes.GetItemByIndex(i), app);
    if (p.GetDefId() == InvalidIndex || rebuild_defs) {
      TSPlane::Def def = p.GetDef();
      size_t di = InvalidIndex;
      for (size_t j = 0; j < PlaneDefs.Count(); j++) {
        if (PlaneDefs[j] == def) {
          di = j;
          break;
        }
      }
      if (di == InvalidIndex) {
        p._SetDefId(PlaneDefs.Count());
        PlaneDefs.AddNew(def);
      }
      else {
        p._SetDefId(di);
      }
    }
  }
}
//..............................................................................
void TLattice::SetGrowInfo(GrowInfo* grow_info) {
  olx_del_obj(_GrowInfo);
  _GrowInfo = grow_info;
}
//..............................................................................
void TLattice::SetPlaneDefinitions(const TTypeList<TSPlane::Def> &pd) {
  PlaneDefs = pd;
}
//..............................................................................
TLattice::GrowInfo* TLattice::GetGrowInfo() const {
  if (!IsGenerated()) {
    return 0;
  }
  const TAsymmUnit& au = GetAsymmUnit();
  GrowInfo& gi = *(new GrowInfo);
  gi.unc_matrix_count = GetUnitCell().MatrixCount();
  gi.info.SetCount(au.AtomCount());
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (sa.IsDeleted()) {
      continue;
    }
    gi.info[sa.CAtom().GetId()] << sa.GetMatrix().GetId();
  }
  return &gi;
}
//..............................................................................
bool TLattice::ApplyGrowInfo() {
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  if (_GrowInfo == 0 || !Objects.atoms.IsEmpty() || !Matrices.IsEmpty() ||
    uc.MatrixCount() != _GrowInfo->unc_matrix_count)
  {
    if (_GrowInfo != 0) {
      delete _GrowInfo;
      _GrowInfo = 0;
    }
    return false;
  }
  ClearMatrices();
  olx_pdict<uint32_t, size_t> mmap;
  size_t cnt = olx_sum(_GrowInfo->info,
    FunctionAccessor::MakeConst(
      (size_t (TUIntList::*)() const) &TUIntList::Count)); // GCC!!!
  Objects.atoms.IncCapacity(cnt);
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom& ca = au.GetAtom(i);
    // we still need masked and detached atoms here
    if (ca.IsDeleted()) {
      continue;
    }
    // number of Q-peaks might grow - use the last matrices to grow new peaks
    const TArrayList<uint32_t>& mi = _GrowInfo->info[
      i >= _GrowInfo->info.Count() ? (_GrowInfo->info.Count()-1) : i];
    if (mi.IsEmpty()) { // new atom? generate in the AU
      if (!Matrices.IsEmpty()) {
        GenerateAtom(ca, *Matrices[0]);
      }
    }
    else {
      for (size_t j = 0; j < mi.Count(); j++) {
        smatd m = smatd::FromId(mi[j], uc.GetMatrix(smatd::GetContainerId(mi[j])));
        size_t idx = mmap.Find(mi[j], InvalidIndex);
        if (idx == InvalidIndex) {
          Matrices.Add(new smatd(m));
          idx = Matrices.Count() - 1;
          mmap.Add(mi[j], idx);
        }
        GenerateAtom(ca, *Matrices[idx]);
      }
    }
  }
  delete _GrowInfo;
  _GrowInfo = 0;
  return true;
}
//..............................................................................
void TLattice::_CreateFrags(TCAtom& start, TCAtomPList& dest) {
  start.SetTag(1);
  dest.Add(start);
  for (size_t i = 0; i < start.AttachedSiteCount(); i++) {
    const TCAtom::Site& site = start.GetAttachedSite(i);
    if (site.atom->GetTag() != 0) {
      continue;
    }
    _CreateFrags(*site.atom, dest);
  }
}
//..............................................................................
olxstr TLattice::CalcMoietyStr(bool html) const {
  const TAsymmUnit& au = GetAsymmUnit();
  TTypeList<AnAssociation3<double, ContentList, size_t> > frags =
    CalcMoiety();
  olxstr_buf rv;
  olxstr so = "<sub>", sc = "</sub>";
  for (size_t i = 0; i < frags.Count(); i++) {
    if (!rv.IsEmpty()) {
      rv << ", ";
    }
    if (frags[i].GetA() != 1) {
      rv << olx_round(frags[i].GetA(), 1000) << '(';
    }
    for (size_t j = 0; j < frags[i].GetB().Count(); j++) {
      rv << frags[i].GetB()[j].element->symbol;
      if (frags[i].GetB()[j].count != 1) {
        if (html) {
          rv << so << olx_round(frags[i].GetB()[j].count, 1000) << sc;
        }
        else {
          rv << olx_round(frags[i].GetB()[j].count, 1000);
        }
      }
      if ((j + 1) < frags[i].GetB().Count()) {
        rv << ' ';
      }
    }
    if (frags[i].GetA() != 1) {
      rv << ')';
    }
  }
  return rv;
}
//..............................................................................
TTypeList<AnAssociation3<double, ContentList, size_t> >::const_list_type
TLattice::CalcMoiety() const
{
  const TAsymmUnit& au = GetAsymmUnit();
  TTypeList<TCAtomPList> cfrags;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ) {
      au.GetAtom(i).SetTag(1);  // ignore
    }
    else {
      au.GetAtom(i).SetTag(0); // unprocessed
    }
  }
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).GetTag() == 0) {
      _CreateFrags(au.GetAtom(i), cfrags.AddNew());
    }
  }
  // multiplicity,content, reference fragment index
  TTypeList<AnAssociation3<double, ContentList, size_t> > frags;
  for (size_t i = 0; i < cfrags.Count(); i++) {
    ElementDict _cld;
    for (size_t j = 0; j < cfrags[i].Count(); j++) {
      _cld.Add(&cfrags[i][j]->GetType(), 0) += cfrags[i][j]->GetOccu();
    }
    ContentList cl(_cld.Count(), false);
    for (size_t j = 0; j < _cld.Count(); j++) {
      cl.Set(j, new ElementCount(*_cld.GetKey(j), _cld.GetValue(j)));
    }
    XElementLib::SortContentList(cl);
    bool uniq = true;
    double overall_occu = 0;
    for (size_t j = 0; j < cfrags[i].Count(); j++) {
      const double occu = cfrags[i][j]->GetOccu();
      if (overall_occu == 0) {
        overall_occu = occu;
      }
      else if (overall_occu != -1 && olx_abs(overall_occu - occu) > 0.01) {
        overall_occu = -1;
        break;
      }
    }
    for (size_t j = 0; j < frags.Count(); j++) {
      if (frags[j].GetB().Count() != cl.Count()) {
        continue;
      }
      bool equals = true;
      if (frags[j].GetB()[0].element != cl[0].element) {
        equals = false;
      }
      else {
        for (size_t k = 1; k < cl.Count(); k++) {
          if (frags[j].GetB()[k].element != cl[k].element ||
            olx_abs((frags[j].GetB()[k].count /
              frags[j].GetB()[0].count) - (cl[k].count / cl[0].count)) > 0.01)
          {
            equals = false;
            break;
          }
        }
      }
      if (equals) {
        frags[j].a += cl[0].count / frags[j].GetB()[0].count;
        uniq = false;
        break;
      }
    }
    if (uniq) {
      if (olx_abs(overall_occu) == 1) {
        frags.AddNew(1, cl, i);
      }
      else {  // apply overal atom occupancy
        for (size_t j = 0; j < cl.Count(); j++) {
          cl[j].count /= overall_occu;
        }
        frags.AddNew(overall_occu, cl, i);
      }
    }
  }
  // apply Z multiplier...
  const double zp_mult = (double)GetUnitCell().MatrixCount() / olx_max(au.GetZ(), 1);
  if (zp_mult != 1) {
    for (size_t i = 0; i < frags.Count(); i++) {
      const TCAtomPList& l = cfrags[frags[i].GetC()];
      bool polymeric = false;
      const size_t generators = GetFragmentGrowMatrices(l, false, &polymeric).Count();
      const int gd = int(generators == 0 ? 1 : generators);
      double mult = polymeric ? zp_mult : gd;
      if (!polymeric) {
        frags[i].a *= zp_mult / gd;
      }
      for (size_t j = 0; j < frags[i].GetB().Count(); j++) {
        frags[i].b[j].count *= mult;
      }
    }
  }
  return frags;
}
//..............................................................................
void TLattice::RestoreADPs(bool restoreCoordinates) {
  TUnitCell& uc = GetUnitCell();
  const TAsymmUnit& au = GetAsymmUnit();
  uc.UpdateEllipsoids();
  const size_t ac = Objects.atoms.Count();
  for (size_t i = 0; i < ac; i++) {
    TSAtom& sa = Objects.atoms[i];
    if (restoreCoordinates) {
      sa.crd() = au.Orthogonalise(sa.ccrd());
    }
    if (sa.CAtom().GetEllipsoid() != 0) {
      sa.SetEllipsoid(
        &uc.GetEllipsoid(sa.GetMatrix().GetContainerId(), sa.CAtom().GetId()));
    }
    else
      sa.SetEllipsoid(0);
  }
  for (size_t i = 0; i < uc.EllpCount(); i++) {
    TEllipsoid* elp = uc.GetEllp(i);
    if (elp != 0) {
      elp->SetTag(0);
    }
  }
}
//..............................................................................
void TLattice::BuildAtomRegistry() {
  Objects.atomRegistry.Clear();
  if (Matrices.IsEmpty()) {
    return;
  }
  vec3i mind(100, 100, 100), maxd(-100, -100, -100);
  const size_t ac = Objects.atoms.Count();
  TTypeList<TSAtom::Ref> refs(ac);
  for (size_t i = 0; i < ac; i++) {
    TSAtom &sa = Objects.atoms[i];
    if (!sa.IsAvailable()) {
      continue;
    }
    refs[i] = sa.GetRef();
    vec3i::UpdateMinMax(smatd::GetT(refs[i].matrix_id), mind, maxd);
  }
  if (ac == 0) {
    maxd = mind = vec3i(0);
  }
  if (mind[0] == 100) {
    return;
  }
  maxd[0] += 1;  maxd[1] += 1;  maxd[2] += 1;
  AtomRegistry::RegistryType& registry = Objects.atomRegistry.Init(mind, maxd);
  for (size_t i = 0; i < ac; i++) {
    TSAtom* sa = &Objects.atoms[i];
    if (!sa->IsAvailable() || sa->CAtom().IsMasked()) {
      continue;
    }
    const vec3i t = smatd::GetT(refs[i].matrix_id);
    TArrayList<TSAtomPList*>* aum_slice = registry.Value(t);
    if (aum_slice == 0) {
      const size_t matr_cnt = GetUnitCell().MatrixCount();
      aum_slice = (registry.Value(t) =
        new TArrayList<TSAtomPList*>(matr_cnt, olx_list_init::zero()));
    }
    uint8_t  c_id = smatd::GetContainerId(refs[i].matrix_id);
    TSAtomPList* au_slice = (*aum_slice)[c_id];
    if (au_slice == 0) {
      const size_t atom_cnt = GetAsymmUnit().AtomCount();
      au_slice = ((*aum_slice)[c_id] = new TSAtomPList(atom_cnt));
    }
    else if ((*au_slice)[refs[i].atom_id] != 0 &&
      (*au_slice)[refs[i].atom_id] != sa)
    {
      (*au_slice)[refs[i].atom_id]->SetDeleted(true);
    }
    (*au_slice)[refs[i].atom_id] = sa;
  }
}
//..............................................................................
void TLattice::AddLatticeContent(const TLattice& latt) {
  if (latt.IsGenerated()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "cannot adopt grown structure");
  }
  TSAtomPList new_atoms;
  TSBondPList new_bonds;
  for (size_t i = 0; i < latt.Objects.atoms.Count(); i++) {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TCAtom& ca = GetAsymmUnit().NewAtom();
    GetAsymmUnit().CartesianToCell(ca.ccrd() = src_a.crd());
    ca.SetType(src_a.GetType());
    ca.SetLabel(src_a.GetLabel(), false);
    TSAtom* sa = new_atoms.Add(Objects.atoms.New(Network));
    sa->CAtom(ca);
    sa->crd() = GetAsymmUnit().Orthogonalise(sa->ccrd());
    sa->_SetMatrix(*Matrices[0]);
  }
  for (size_t i = 0; i < latt.Objects.bonds.Count(); i++) {
    const TSBond& src_b = latt.Objects.bonds[i];
    TSBond* sb = new_bonds.Add(Objects.bonds.New(Network));
    sb->SetA(*new_atoms[src_b.A().GetOwnerId()]);
    sb->SetB(*new_atoms[src_b.B().GetOwnerId()]);
  }
  for (size_t i = 0; i < latt.Objects.atoms.Count(); i++) {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TSAtom& sa = *new_atoms[i];
    for (size_t j = 0; j < src_a.NodeCount(); j++) {
      sa.AddNode(*new_atoms[src_a.Node(j).GetOwnerId()]);
    }
    for (size_t j = 0; j < src_a.BondCount(); j++) {
      sa.AddBond(*new_bonds[src_a.Bond(j).GetOwnerId()]);
    }
  }
  for (size_t i = 0; i < latt.FragmentCount(); i++) {
    const TNetwork& src_n = latt.GetFragment(i);
    TNetwork& net = *Fragments.Add(new TNetwork(this, Network));
    net.SetOwnerId(Fragments.Count() - 1);
    for (size_t j = 0; j < src_n.NodeCount(); j++) {
      TSAtom& a = *new_atoms[src_n.Node(j).GetOwnerId()];
      net.AddNode(a);
      a.SetNetwork(net);
    }
    for (size_t j = 0; j < src_n.BondCount(); j++) {
      TSBond& b = *new_bonds[src_n.Bond(j).GetOwnerId()];
      net.AddBond(b);
      b.SetNetwork(net);
    }
  }
  GetUnitCell().UpdateEllipsoids();
  RestoreADPs(false);
}
//..............................................................................
void TLattice::SetDelta(double v)  {
  if( Delta != v )  {
    Delta = v;
    UpdateConnectivity();
  }
}
//..............................................................................
void TLattice::SetDeltaI(double v)  {
  if( DeltaI != v )  {
    DeltaI = v;
    GetUnitCell().FindSymmEq();
    UpdateConnectivity();
  }
}
//..............................................................................
void TLattice::undoDelete(TUndoData *data) {
  TDeleteUndo *undo = dynamic_cast<TDeleteUndo*>(data);
  for (size_t i = 0; i < undo->SAtomIds.Count(); i++) {
    RestoreAtom(undo->SAtomIds[i]);
  }
  UpdateConnectivity();
}
//..............................................................................
TUndoData *TLattice::ValidateHGroups(bool reinit, bool report) {
  TCAtomPList deleted;
  TAsymmUnit &au = GetAsymmUnit();
  RefinementModel &rm = *au.GetRefMod();
  // mark atom in EXYZ to count them once only
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(-1));
  for (size_t i = 0; i < rm.ExyzGroups.Count(); i++) {
    for (size_t j = 0; j < rm.ExyzGroups[i].Count(); j++) {
      rm.ExyzGroups[i][j].SetTag(i);
    }
  }
  while (true) {
    size_t deleted_cnt = deleted.Count();
    for (size_t i=0; i < rm.AfixGroups.Count(); i++) {
      TAfixGroup &ag = rm.AfixGroups[i];
      if (!ag.HasImplicitPivot() || ag.IsEmpty()) {
        continue;
      }
      int part = 0;
      for (size_t j = 0; j < ag.Count(); j++) {
        if (ag[j].IsDeleted()) {
          continue;
        }
        part = ag[j].GetPart();
        break;
      }
      size_t attached_cnt=0, metal_cnt=0, dependent_cnt=0;
      olx_pset<index_t> exyz_counted;
      for (size_t j=0; j < ag.GetPivot().AttachedSiteCount(); j++) {
        TCAtom::Site &s = ag.GetPivot().GetAttachedSite(j);
        if (s.atom->IsDeleted() || s.atom->GetType().z < 2) {
          continue;
        }
        if (TAfixGroup::HasImplicitPivot(s.atom->GetAfix())) {
          dependent_cnt++;
          continue;
        }
        if (part == 0 || s.atom->GetPart() == 0 || s.atom->GetPart() == part
          || rm.Conn.ArePartsGroupped(part, s.atom->GetPart()))
        {
          if (!s.matrix.IsFirst() && s.atom->GetPart() < 0) {
            bool contains = false;
            for (size_t k = 0; k < ag.GetPivot().EquivCount(); k++) {
              if (ag.GetPivot().GetEquiv(k).GetId() == s.matrix.GetId()) {
                contains = true;
              }
            }
            if (contains) {
              continue;
            }
          }
          // check if EXYZ
          if (s.atom->GetTag() >= 0) {
            if (exyz_counted.Contains(s.atom->GetTag())) {
              continue;
            }
            else {
              exyz_counted.Add(s.atom->GetTag());
            }
          }
          if (XElementLib::IsMetal(s.atom->GetType())) {
            metal_cnt++;
          }
          attached_cnt++;
        }
      }
      size_t acnt = attached_cnt + dependent_cnt;
      bool valid = true;
      int m = ag.GetM();
      switch (m) {
      case 1: // XYZC-H
        valid = (acnt == 3) || (acnt-metal_cnt == 3);
        break;
      case 2: // XYC-H2
      case 4: // XYC-H
        valid = (acnt == 2) || (acnt-metal_cnt == 2);
        break;
      case 3: // X-H3
      case 8: // O-H
      case 9: // X=C-H2
      case 12: // X-H3 - disordered
      case 13: // X-H3
      case 14: // O-H
      case 16: // CC-H
        valid = (acnt == 1) || (acnt-metal_cnt == 1);
        break;
      case 15: // {R4/5}B-H
        valid = (acnt == 4 || acnt == 5) || (
          (acnt-metal_cnt == 4) || (acnt-metal_cnt == 5));
        break;
      }
      if (!valid) {
        for (size_t gi=0; gi < ag.Count(); gi++) {
          deleted.Add(ag[gi])->SetDeleted(true);
        }
        if (report) {
          TBasicApp::NewLogEntry(logError) << "Pivot atom " <<
            ag.GetPivot().GetLabel() << " has wrong connectivity for the given "
            "AFIX group and the group was removed. Please revise your model.";
          TBasicApp::NewLogEntry() << ag.GetPivot().GetResiLabel() << ", AFIX " <<
            ag.GetAfix() << ", attached atoms " << attached_cnt <<
            ", attached metals " << metal_cnt <<
            ", attached dependent groups " << dependent_cnt;
        }
        ag.Clear();
      }
    }
    if (deleted.IsEmpty()) {
      return 0;
    }
    if (deleted.Count() == deleted_cnt) {
      break;
    }
  }
  TDeleteUndo *du = new TDeleteUndo(
    UndoAction::New(this, &TLattice::undoDelete));
  for (size_t i=0; i < deleted.Count(); i++) {
    TSAtomPList dl = Objects.atomRegistry.FindAll(*deleted[i]);
    du->SAtomIds.SetCapacity(du->SAtomIds.Count()+dl.Count());
    for (size_t j = 0; j < dl.Count(); j++) {
      du->AddSAtom(*dl[j]);
    }
  }
  if (reinit) {
    Init();
  }
  return du;
}
//..............................................................................
bool TLattice::IsPolymeric(bool use_peaks) const {
  using namespace olx_analysis;
  TAsymmUnit &au = GetAsymmUnit();
  olx_object_ptr<TEBitArray> df;
  if (!use_peaks) {
    df = new TEBitArray(au.AtomCount());
    // store Q-atom state
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).GetType() == iQPeakZ) {
        df->Set(i, au.GetAtom(i).IsDeleted());
        au.GetAtom(i).SetDeleted(true);
      }
    }
  }
  TTypeList<fragments::fragment> fl = fragments::extract(GetAsymmUnit());
  if (!use_peaks) {
    // restore the Q-peaks state
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).GetType() == iQPeakZ) {
        au.GetAtom(i).SetDeleted((*df)[i]);
      }
    }
  }
  for (size_t i = 0; i < fl.Count(); i++) {
    if (fl[i].is_polymeric()) {
      return true;
    }
  }
  return false;
}
//..............................................................................
olx_object_ptr<TLattice::GrowInfo> TLattice::Match(
  const TTypeList<Atom3DId>& ids)
{
  const TAsymmUnit& au = GetAsymmUnit();
  const TUnitCell& uc = GetUnitCell();
  olx_object_ptr<GrowInfo> gi = new GrowInfo();
  gi->info.SetCount(au.AtomCount());
  gi->unc_matrix_count = uc.MatrixCount();
  const TCAtomPList& atoms = au.GetAtoms();
  const smatd_list& matrices = uc.GetMatrices();
  for (size_t i = 0; i < ids.Count(); i++) {
    vec3d crd = ids[i].get_crd();
    short z = ids[i].get_z();
    for (size_t ai = 0; ai < atoms.Count(); ai++) {
      if (atoms[ai]->GetType().z != z) {
        continue;
      }
      for (size_t mi = 0; mi < matrices.Count(); mi++) {
        vec3d v = crd - matrices[mi] * atoms[ai]->ccrd();
        const vec3i shift = v.Round<int>();
        au.CellToCartesian(v -= shift);
        const double qd = v.QLength();
        if (qd < 1e-4) {
          smatd m = matrices[mi];
          m.t += shift;
          uc.InitMatrixId(m);
          gi->info[atoms[ai]->GetId()].Add(m.GetId());
          break;
        }
      }
    }
  }
  return gi;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TLattice::LibGetFragmentCount(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(olxstr(FragmentCount()));
}
//..............................................................................
void TLattice::LibGetMoiety(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(
    CalcMoietyStr(Params.IsEmpty() ? false : Params[0].ToBool()));
}
//..............................................................................
void TLattice::LibGetFragmentAtoms(const TStrObjList& Params, TMacroData& E) {
  size_t index = Params[0].ToSizeT();
  if (index >= FragmentCount())
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, FragmentCount());
  olxstr rv;
  for (size_t i = 0; i < Fragments[index]->NodeCount(); i++) {
    rv << Fragments[index]->Node(i).GetLabel();
    if ((i + 1) < Fragments[index]->NodeCount()) {
      rv << ',';
    }
  }
  E.SetRetVal(rv);
}
//..............................................................................
void TLattice::LibIsGrown(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(IsGenerated());
}
//..............................................................................
void TLattice::LibIsPolymeric(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(IsPolymeric(Params.IsEmpty() ? false : Params[0].ToBool()));
}
//..............................................................................
IOlxObject* TLattice::VPtr::get_ptr() const {
  return &TXApp::GetInstance().XFile().GetLattice();
}
//..............................................................................
TLibrary*  TLattice::ExportLibrary(const olxstr& name)  {
  olx_vptr<TLattice> thisp(new VPtr);
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("latt") : name);
  lib->Register(
    new TFunction<TLattice>(thisp,  &TLattice::LibGetFragmentCount,
    "GetFragmentCount", fpNone,
    "Returns number of fragments in the lattice")
  );
  lib->Register(
    new TFunction<TLattice>(thisp,  &TLattice::LibGetFragmentAtoms,
    "GetFragmentAtoms", fpOne,
    "Returns a comma separated list of atoms in specified fragment")
  );
  lib->Register(
    new TFunction<TLattice>(thisp,  &TLattice::LibGetMoiety,
    "GetMoiety", fpNone|fpOne,
    "Returns molecular moiety. HTML formatted depending on the first boolean "
      "argument [false].")
  );
  lib->Register(
    new TFunction<TLattice>(thisp,  &TLattice::LibIsGrown,
    "IsGrown", fpNone,
    "Returns true if the structure is grow")
  );
  lib->Register(
    new TFunction<TLattice>(thisp, &TLattice::LibIsPolymeric,
      "IsPolymeric", fpNone|fpOne,
      "Returns true if the structure is polymeric. The use of Q-peaks is "
      "controlled by the first boolean argument [false]")
  );
  return lib;
}
