/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
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

double TSPlane::CalcRMSD(const TSAtomPList& atoms) {
  if (atoms.Count() < 3) return -1;
  TArrayList<olx_pair_t<vec3d, double> > Points(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    Points[i].a = atoms[i]->crd();
    Points[i].b = 1;
  }
  vec3d p, c;
  return CalcPlane(Points, p, c);
}
//..............................................................................
double TSPlane::CalcRMSD(const TAtomEnvi& atoms) {
  if (atoms.Count() < 3) return -1;
  TArrayList<olx_pair_t<vec3d, double> > Points(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    Points[i].a = atoms.GetCrd(i);
    Points[i].b = 1;
  }
  vec3d p, c;
  return CalcPlane(Points, p, c);
}
//..............................................................................
void TSPlane::Init(const TTypeList< olx_pair_t<TSAtom*, double> >& atoms) {
  TTypeList<olx_pair_t<vec3d, double> > points;
  points.SetCapacity(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    points.AddNew(atoms[i].GetA()->crd(), atoms[i].GetB());
  }
  _Init(points);
  Crds.Clear().AddAll(atoms);
}
//..............................................................................
void TSPlane::_Init(const TTypeList<olx_pair_t<vec3d, double> >& points) {
  vec3d rms;
  CalcPlanes(points, Normals, rms, Center);
  const double nl = GetNormal().Length();
  Distance = GetNormal().DotProd(Center) / nl;
  Normals[0] /= nl;
  if (Normals[2].DotProd(points[0].GetA() - Center) < 0) {
    Normals[1] *= -1;
    Normals[2] *= -1;
  }
  wRMSD = rms[0];
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
bool TSPlane::CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms,
  vec3d& center)
{
  TTypeList< olx_pair_t<vec3d, double> > Points;
  Points.SetCapacity(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    Points.AddNew(atoms[i]->crd(), 1.0);
  }
  return CalcPlanes(Points, params, rms, center);
}
//..............................................................................
double TSPlane::CalcPlane(const TSAtomPList& atoms,
  vec3d& Params, vec3d& center, const short type)
{
  TTypeList< olx_pair_t<vec3d, double> > Points;
  Points.SetCapacity(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    Points.AddNew(atoms[i]->crd(), 1.0);
  }
  return CalcPlane(Points, Params, center, type);
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
void TSPlane::ToDataItem(TDataItem& item) const {
  item.AddField("def_id", GetDefId());
  size_t cnt = 0;
  olxstr iname("atom");
  for (size_t i = 0; i < Crds.Count(); i++) {
    if (Crds[i].GetA()->IsDeleted()) {
      continue;
    }
    item.AddItem(iname, Crds[i].GetB()).AddField("atom_id",
      Crds[i].GetA()->GetTag());
  }
  item.AddField("normal", PersUtil::VecToStr(GetNormal()));
}
//..............................................................................
void TSPlane::FromDataItem(const TDataItem& item) {
  olxstr di = item.FindField("def_id");
  if (!di.IsEmpty()) {
    DefId = di.ToSizeT();
  }
  Crds.Clear();
  ASObjectProvider& objects = Network->GetLattice().GetObjects();
  for (size_t i = 0; i < item.ItemCount(); i++) {
    Crds.AddNew(
      &objects.atoms[item.GetItemByIndex(i).GetFieldByName("atom_id").ToInt()],
      item.GetItemByIndex(i).GetValue().ToDouble());
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
vec3d TSPlane::GetCrystallographicDirection() const {
  return GetCrystallographicDirection(
    GetNetwork().GetLattice().GetAsymmUnit().GetCellToCartesian(),
    GetNormal(),
    GetCenter());
}
//..............................................................................
vec3d TSPlane::GetCrystallographicDirection(const mat3d &m,
  const vec3d &n, const vec3d &p)
{
  vec3d hkl;
  double d = n.DotProd(p);
  if (olx_abs(d) < 1e-3) {
    return hkl(1000);
  }
  for (int i=0; i < 3; i++) {
    hkl[i] = n.DotProd(m[i])/d;
  }
  return hkl;
}
//..............................................................................
//..............................................................................
TSPlane::Def::Def(const TSPlane& plane)
  : atoms(plane.Count()), sides(0)
{
  for (size_t i = 0; i < plane.Count(); i++) {
    atoms.Set(i, new DefData(plane.GetAtom(i).GetRef(), plane.GetWeight(i)));
  }
  if( plane.Count() == 0 )  return;
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
void TSPlane::Def::DefData::ToDataItem(TDataItem& item) const {
  item.AddField("weight", weight);
  ref.ToDataItem(item.AddItem("atom"));
}
//..............................................................................
void TSPlane::Def::DefData::FromDataItem(const TDataItem& item)  {
  weight = item.GetFieldByName("weight").ToDouble();
  ref.FromDataItem(item.GetItemByIndex(0));  //!! may be changed if extended
}
//..............................................................................
void TSPlane::Def::ToDataItem(TDataItem& item) const {
  item.AddField("sides", sides);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].ToDataItem(item.AddItem(i + 1));
  }
}
//..............................................................................
void TSPlane::Def::FromDataItem(const TDataItem& item)  {
  atoms.Clear();
  sides = item.GetFieldByName("sides").ToSizeT();
  for (size_t i = 0; i < item.ItemCount(); i++) {
    atoms.AddNew(item.GetItemByIndex(i));
  }
}
//..............................................................................
TSPlane* TSPlane::Def::FromAtomRegistry(ASObjectProvider& ar, size_t def_id,
  TNetwork* parent, const smatd& matr) const
{
  TTypeList<olx_pair_t<TSAtom*, double> > points;
  const TUnitCell& uc = parent->GetLattice().GetUnitCell();
  const TAsymmUnit& au = parent->GetLattice().GetAsymmUnit();
  if (matr.IsFirst()) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom::Ref &ref = atoms[i].ref;
      smatd m = smatd::FromId(ref.matrix_id,
        uc.GetMatrix(smatd::GetContainerId(ref.matrix_id)));
      TSAtom* sa = ar.atomRegistry.Find(
        TSAtom::GetRef(au.GetAtom(ref.catom_id), m));
      if (sa == NULL)  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  else {
    for (size_t i = 0; i < atoms.Count(); i++) {
      TSAtom::Ref &ref = atoms[i].ref;
      smatd m = uc.MulMatrix(smatd::FromId(ref.matrix_id,
        uc.GetMatrix(smatd::GetContainerId(ref.matrix_id))), matr);
      TSAtom* sa = ar.atomRegistry.Find(
        TSAtom::GetRef(au.GetAtom(ref.catom_id), m));
      if (sa == NULL)  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  TSPlane& p = ar.planes.New(parent);
  p.DefId = def_id;
  p.Init(points);
  return &p;
}
//..............................................................................
