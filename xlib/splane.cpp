/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "splane.h"
#include "ematrix.h"
#include "satom.h"
#include "sbond.h"
#include "lattice.h"
#include "unitcell.h"
#include "pers_util.h"
#include "xapp.h"
#include "math/plane.h"

double TSPlane::CalcRMSD(const TSAtomPList& atoms) {
  if (atoms.Count() < 3) {
    return -1;
  }
  return plane::mean<>::calc(atoms, TSAtom::CrdAccessor()).rms[0];
}
//..............................................................................
double TSPlane::CalcRMSD(const TAtomEnvi& atoms) {
  if (atoms.Count() < 3) {
    return -1;
  }
  vec3d_list Points(atoms.Count(), true);
  for (size_t i = 0; i < atoms.Count(); i++) {
    Points[i] = atoms.GetCrd(i);
  }
  return plane::mean<>::calc(Points).rms[0];
}
//..............................................................................
void TSPlane::Init(const TTypeList< olx_pair_t<TSAtom*, double> >& atoms) {
  TTypeList<olx_pair_t<vec3d, double> > points;
  points.SetCapacity(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    points.AddNew(atoms[i].GetA()->crd(), atoms[i].GetB());
  }
  _Init(points);
  Crds.Clear().AddCopyAll(atoms);
}
//..............................................................................
void TSPlane::_Init(const TTypeList<olx_pair_t<vec3d, double> >& points) {
  plane::mean<>::out po = plane::mean<>::calc_for_pairs(points);
  Normals = po.normals;
  Center = po.center;
  const double nl = GetNormal().Length();
  Distance = GetNormal().DotProd(Center) / nl;
  Normals[0] /= nl;
  if (Normals[2].DotProd(points[0].GetA() - Center) < 0) {
    Normals[1] *= -1;
    Normals[2] *= -1;
  }
  wRMSD = po.rms[0];
}
//..............................................................................
double TSPlane::CalcRMSD() const {
  if (Crds.IsEmpty()) {
    return 0;
  }
  double qd = 0;
  for (size_t i = 0; i < Crds.Count(); i++) {
    qd += olx_sqr(DistanceTo(*Crds[i].GetA()));
  }
  return sqrt(qd/Crds.Count());
}
//..............................................................................
double TSPlane::CalcWeightedRMSD() const {
  if (Crds.IsEmpty()) {
    return 0;
  }
  double qd = 0, qw = 0;
  for (size_t i = 0; i < Crds.Count(); i++) {
    qd += olx_sqr(DistanceTo(*Crds[i].GetA())*Crds[i].GetB());
    qw += olx_sqr(Crds[i].GetB());
  }
  return sqrt(qd / qw);
}
//..............................................................................
double TSPlane::Angle(const TSBond& Bd) const {
  return Angle(Bd.A().crd(), Bd.B().crd());
}
//..............................................................................
void TSPlane::Invert() {
  Normals *= -1;
  Distance = -Distance;
  olx_reverse(Crds);
}
//..............................................................................
void TSPlane::ToDataItem(TDataItem& item, const TXApp& app) const {
  item.AddField("def_id", GetDefId());
  size_t cnt = 0;
  olxstr iname("atom");
  for (size_t i = 0; i < Crds.Count(); i++) {
    if (Crds[i].GetA()->IsDeleted()) {
      continue;
    }
    Crds[i].GetA()->GetRef().ToDataItem(item.AddItem(iname, Crds[i].GetB()), app);
  }
  item.AddField("normal", PersUtil::VecToStr(GetNormal()));
}
//..............................................................................
void TSPlane::FromDataItem(const TDataItem& item, const TXApp& app) {
  olxstr di = item.FindField("def_id");
  if (!di.IsEmpty()) {
    DefId = di.ToSizeT();
  }
  Crds.Clear();
  ASObjectProvider &p = GetNetwork().GetLattice().GetObjects();
  for (size_t i = 0; i < item.ItemCount(); i++) {
    TDataItem& ai = item.GetItemByIndex(i);
    // compatibility... 2020-05-14
    {
      olxstr old_id = ai.FindField("atom_id");
      if (!old_id.IsEmpty()) {
        size_t id = old_id.ToSizeT();
        if (id > p.atoms.Count()) {
          TBasicApp::NewLogEntry(logError, false, __OlxSourceInfo) <<
            "Invalid atom id for plane: " << id;
        }
        Crds.AddNew(&p.atoms[id],
          ai.GetValue().ToDouble());
        continue;
      }
    }
    Crds.AddNew(&app.GetSAtom(TSAtom::Ref(ai, app)), ai.GetValue().ToDouble());
  }
  TTypeList< olx_pair_t<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for (size_t i = 0; i < Crds.Count(); i++) {
    points.AddNew(Crds[i].GetA()->crd(), Crds[i].GetB());
  }
  _Init(points);
  olxstr ns = item.FindField("normal");
  if (!ns.IsEmpty()) {
    vec3d n = PersUtil::VecFromStr<vec3d>(ns);
    if (GetNormal().DotProd(n) < 0) {
      Invert();
    }
  }
}
//..............................................................................
olxstr TSPlane::StrRepr() const {
  olxstr rv;
  const vec3d &n = GetNormal();
  for (int i=0; i < 3; i++) {
    if (olx_abs(n[i]) > 1e-5) {
      if (!rv.IsEmpty()) {
        rv << ' ';
        if (n[i] > 0) rv << '+';
      }
      rv << olxstr::FormatFloat(3, n[i]) << '*' << olxch('X'+i);
    }
  }
  if (olx_abs(GetD()) > 1e-5) {
    rv << ' ';
    if (GetD() > 0) rv << '+';
    rv << olxstr::FormatFloat(3, GetD());
  }
  return rv << " = 0";
}
//..............................................................................
vec3d TSPlane::GetCrystallographicDirection(bool reciprocal, bool exact) const {
  const TAsymmUnit& au = GetNetwork().GetLattice().GetAsymmUnit();
  if (exact) {
    return GetCrystallographicDirection(
      reciprocal ? au.GetCellToCartesian() : au.GetCartesianToCell(),
      GetNormal(), GetCenter());
  }
  return GetCrystallographicDirection(
     reciprocal ? au.GetCellToCartesian() : au.GetCartesianToCell(),
     GetNormal());
}
//..............................................................................
vec3d TSPlane::GetCrystallographicDirection(const mat3d &M,
  const vec3d &N)
{
  vec3d hkl = (M * N).Normalise();
  double min_v = 100;
  for (int i=0; i < 3; i++) {
    double av = olx_abs(hkl[i]);
    if (av > 1e-3 && av < min_v) {
      min_v = av;
    }
  }
  return hkl / min_v;
}
//..............................................................................
vec3d TSPlane::GetCrystallographicDirection(const mat3d& m,
  const vec3d& n, const vec3d& p)
{
  double d = n.DotProd(p);
  if (olx_abs(d) < 1e-3) {
    return vec3d();
  }
  return (m * n) / d;
}
//..............................................................................
//..............................................................................
TSPlane::Def::Def(const TSPlane& plane)
  : atoms(plane.Count()), sides(0)
{
  for (size_t i = 0; i < plane.Count(); i++) {
    atoms.Set(i, new DefData(plane.GetAtom(i).GetRef(), plane.GetWeight(i)));
  }
  if (plane.Count() == 0) {
    return;
  }
  if (!plane.GetAtom(0).IsAUAtom()) {
    const TUnitCell& uc = plane.GetNetwork().GetLattice().GetUnitCell();
    const smatd im = uc.InvMatrix(plane.GetAtom(0).GetMatrix());
    for (size_t i = 0; i < atoms.Count(); i++) {
      atoms[i].ref.matrix_id = uc.MulMatrixId(
        smatd::FromId(atoms[i].ref.matrix_id,
          uc.GetMatrix(smatd::GetContainerId(atoms[i].ref.matrix_id))), im);
    }
  }
  QuickSorter::Sort(atoms);
}
//..............................................................................
void TSPlane::Def::DefData::ToDataItem(TDataItem& item, const TXApp& app, bool use_id) const {
  item.AddField("weight", weight);
  ref.ToDataItem(item.AddItem("atom"), app, use_id);
}
//..............................................................................
void TSPlane::Def::ToDataItem(TDataItem& item, class TXApp& app, bool use_id) const {
  item.AddField("sides", sides);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].ToDataItem(item.AddItem(i + 1), app, use_id);
  }
}
//..............................................................................
void TSPlane::Def::FromDataItem(const TDataItem& item, const TXApp& app) {
  atoms.Clear();
  sides = item.GetFieldByName("sides").ToSizeT();
  for (size_t i = 0; i < item.ItemCount(); i++) {
    atoms.AddNew(item.GetItemByIndex(i), app);
  }
}
//..............................................................................
TSPlane* TSPlane::Def::FromAtomRegistry(const TXApp& app, ASObjectProvider& ar,
  size_t def_id, TNetwork* parent, const smatd& matr) const
{
  TTypeList<olx_pair_t<TSAtom*, double> > points;
  if (matr.IsFirst()) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom* sa = atoms[i].ref.GetLattice(app)
        .GetAtomRegistry().Find(atoms[i].ref);
      if (sa == 0) {
        return 0;
      }
      points.AddNew(sa, atoms[i].weight);
    }
  }
  else {
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom::Ref &ref = atoms[i].ref;
      const TUnitCell &uc = atoms[i].ref.GetLattice(app).GetUnitCell();
      smatd m = uc.MulMatrix(smatd::FromId(ref.matrix_id,
        uc.GetMatrix(smatd::GetContainerId(ref.matrix_id))), matr);
      TSAtom* sa = atoms[i].ref.GetLattice(app)
        .GetAtomRegistry().Find(ref.atom_id, m.GetId());
      if (sa == 0) {
        return 0;
      }
      points.AddNew(sa, atoms[i].weight);
    }
  }
  TSPlane& p = ar.planes.New(parent);
  p.DefId = def_id;
  p.Init(points);
  return &p;
}
//..............................................................................
int TSPlane::Def::Compare(const TSPlane::Def& d) const {
  int r = olx_cmp(atoms.Count(), d.atoms.Count());
  if (r != 0) {
    return r;
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    r = atoms[i].Compare(d.atoms[i]);
    if (r == 0) {
      r = olx_cmp(atoms[i].weight, d.atoms[i].weight);
    }
    if (r != 0) {
      return r;
    }
  }
  return olx_cmp(sides, d.sides);
}
//..............................................................................
