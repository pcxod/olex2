//---------------------------------------------------------------------------//
// TSPlane implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "splane.h"
#include "ematrix.h"
#include "satom.h"
#include "sbond.h"
#include "lattice.h"
#include "unitcell.h"
#include "pers_util.h"

//..............................................................................
double TSPlane::CalcRMS(const TSAtomPList& atoms)  {
  if( atoms.Count() < 3 )  return -1;
  vec3d p, c;
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity( atoms.Count() );
  for( size_t i=0; i < atoms.Count(); i++ )
    Points.AddNew(atoms[i]->crd(), 1);
  return CalcPlane(Points, p, c);
}
//..............................................................................
void TSPlane::Init(const TTypeList< AnAssociation2<TSAtom*, double> >& atoms)  {
  TTypeList<AnAssociation2<vec3d, double> > points;
  points.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    points.AddNew(atoms[i].GetA()->crd(), atoms[i].GetB());
  _Init(points);
  Crds.Clear().AddListC(atoms);
}
//..............................................................................
void TSPlane::_Init(const TTypeList<AnAssociation2<vec3d, double> >& points)  {
  vec3d rms;
  CalcPlanes(points, Normals, rms, Center, true);
  Distance = GetNormal().DotProd(Center)/GetNormal().Length();
  Normals[0].Normalise();
  Normals[1] = (points[0].GetA() - Center).Normalise();
  Normals[2] = Normals[0].XProdVec(Normals[1]).Normalise();
  wRMSD = rms[0];
}
//..............................................................................
double TSPlane::CalcRMSD() const {
  if( Crds.IsEmpty() )  return 0;
  double qd = 0;
  for( size_t i=0; i < Crds.Count(); i++ )
    qd += olx_sqr(DistanceTo(*Crds[i].GetA()));
  return sqrt(qd/Crds.Count());
}
//..............................................................................
double TSPlane::CalcWeightedRMSD() const {
  if( Crds.IsEmpty() )  return 0;
  double qd = 0, qw = 0;
  for( size_t i=0; i < Crds.Count(); i++ )  {
    qd += olx_sqr(DistanceTo(*Crds[i].GetA())*Crds[i].GetB());
    qw += olx_sqr(Crds[i].GetB());
  }
  return sqrt(qd/qw);
}
//..............................................................................
bool TSPlane::CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms, vec3d& center) {
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    Points.AddNew(atoms[i]->crd(), 1.0);
  return CalcPlanes(Points, params, rms, center);
}
//..............................................................................
double TSPlane::CalcPlane(const TSAtomPList& atoms, 
                        vec3d& Params, vec3d& center, const short type)  
{
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    Points.AddNew(atoms[i]->crd(), 1.0);
  return CalcPlane(Points, Params, center, type);
}
//..............................................................................
double TSPlane::Angle(const TSBond& Bd) const {
  return Angle(Bd.A().crd(), Bd.B().crd());
}
//..............................................................................
void TSPlane::ToDataItem(TDataItem& item) const {
  item.AddField("regular", IsRegular());
  size_t cnt = 0;
  for( size_t i=0; i < Crds.Count(); i++ )  {
    if( Crds[i].GetA()->IsDeleted() )  continue;
    item.AddItem(cnt++, Crds[i].GetB()).AddField("atom_id", Crds[i].GetA()->GetTag()); 
  }
}
//..............................................................................
void TSPlane::FromDataItem(const TDataItem& item)  {
  Crds.Clear();
  ASObjectProvider& objects = Network->GetLattice().GetObjects();
  for( size_t i=0; i < item.ItemCount(); i++ )  {
    Crds.AddNew(&objects.atoms[item.GetItem(i).GetRequiredField("atom_id").ToInt()], 
      item.GetItem(i).GetValue().ToDouble());
  }
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for( size_t i=0; i < Crds.Count(); i++ )
    points.AddNew( Crds[i].GetA()->crd(), Crds[i].GetB());
  _Init(points);
  SetRegular(item.GetFieldValue("regular", FalseString).ToBool());
}
//..............................................................................
TSPlane::Def::Def(const TSPlane& plane) : atoms(plane.Count()), regular(plane.IsRegular())  {
  for( size_t i=0; i < plane.Count(); i++ )
    atoms.Set(i, new DefData(plane.GetAtom(i).GetRef(), plane.GetWeight(i)));
  if( plane.Count() == 0 )  return;
  if( !plane.GetAtom(0).IsAUAtom() )  {
    const TUnitCell& uc = plane.GetNetwork().GetLattice().GetUnitCell();
    const smatd im = uc.InvMatrix(plane.GetAtom(0).GetMatrix(0));
    for( size_t i=0; i < atoms.Count(); i++ )  {
      atoms[i].ref.matrix_id = uc.MulMatrix(
        smatd::FromId(atoms[i].ref.matrix_id,
        uc.GetMatrix(smatd::GetContainerId(atoms[i].ref.matrix_id))), im).GetId();
    }
  }
  atoms.QuickSorter.Sort<TComparablePtrComparator>(atoms);
}
//..............................................................................
void TSPlane::Def::DefData::ToDataItem(TDataItem& item) const {
  item.AddField("weight", weight);
  ref.ToDataItem(item.AddItem("atom"));
}
//..............................................................................
void TSPlane::Def::DefData::FromDataItem(const TDataItem& item)  {
  weight = item.GetRequiredField("weight").ToDouble();
  ref.FromDataItem(item.GetItem(0));  //!! may be changed if extended
}
//..............................................................................
void TSPlane::Def::ToDataItem(TDataItem& item) const {
  item.AddField("regular", regular);
  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i].ToDataItem(item.AddItem(i+1));
}
//..............................................................................
void TSPlane::Def::FromDataItem(const TDataItem& item)  {
  atoms.Clear();
  regular = item.GetRequiredField("regular").ToBool();
  for( size_t i=0; i < item.ItemCount(); i++ )
    atoms.AddNew(item.GetItem(i));
}
//..............................................................................
TSPlane* TSPlane::Def::FromAtomRegistry(ASObjectProvider& ar, size_t def_id, TNetwork* parent, const smatd& matr) const {
  TTypeList<AnAssociation2<TSAtom*, double> > points;
  mat3d equiv;
  equiv.I();
  if( matr.IsFirst() )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* sa = ar.atomRegistry.Find(atoms[i].ref);
      if( sa == NULL )  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  else  {
    const TUnitCell& uc = parent->GetLattice().GetUnitCell();
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom::Ref ref = atoms[i].ref;
      smatd m = smatd::FromId(ref.matrix_id, uc.GetMatrix(smatd::GetContainerId(ref.matrix_id)))*matr;
      uc.InitMatrixId(m);
      if( i == 0 )
        equiv = m.r;
      ref.matrix_id = m.GetId();
      TSAtom* sa = ar.atomRegistry.Find(ref);
      if( sa == NULL )  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  TSPlane& p = ar.planes.New(parent);
  p.DefId = def_id;
  p.Init(points);
  p.SetRegular(regular);
  return &p;
}
