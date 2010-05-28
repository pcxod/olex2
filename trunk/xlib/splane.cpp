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
  CalcPlane(points, Normal, Center);
  Distance = GetNormal().DotProd(Center)/GetNormal().Length();
  Normal.Normalise();
}
//..............................................................................
bool TSPlane::CalcPlanes(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
                        mat3d& Params, vec3d& rms, vec3d& center, bool sort)  
{
  if( Points.Count() < 3 )  return false;
  center.Null();
  mat3d m;
  double mass = 0;
  center.Null();
  for( size_t i=0; i < Points.Count(); i++ )  {
    center += Points[i].GetA()*Points[i].GetB();
    mass += Points[i].GetB();
  }
  center /= mass;

  for( size_t i=0; i < Points.Count(); i++ )  {
    vec3d t = Points[i].GetA() - center;
    double wght = Points[i].GetB()*Points[i].GetB();
    m[0][0] += (t[0]*t[0]*wght);
    m[0][1] += (t[0]*t[1]*wght);
    m[0][2] += (t[0]*t[2]*wght);

    m[1][1] += (t[1]*t[1]*wght);
    m[1][2] += (t[1]*t[2]*wght);

    m[2][2] += (t[2]*t[2]*wght);
  } 
  m[1][0] = m[0][1];
  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  mat3d::EigenValues(m, Params.I());
  // optimised version will create slightly negative values!
  if( sort )  {
    for( int i=0; i < 3; i++ )
      rms[i] = (m[i][i] < 0 ? 0 : sqrt(m[i][i]/Points.Count()));
    bool swaps = true;
    while( swaps )  {
      swaps = false;
      for( int i=0; i < 2; i++ )  {
        if( rms[i] > rms[i+1] )  {
          olx_swap(Params[i], Params[i+1]);
          olx_swap(rms[i], rms[i+1]);
          swaps = true;
        }
      }
    }
  }
  return true;
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
double TSPlane::CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
                        vec3d& Params, vec3d& center, const short type)  
{
  mat3d normals;
  vec3d rms;
  if( CalcPlanes(Points, normals, rms, center) )  {
    if( type == plane_best )  {
      Params = normals[0];
      return rms[0];
    }
    else if( type == plane_worst )  {
      Params = normals[2];
      return rms[2];
    }
    Params = normals[1];
    return rms[1];
  }
  return -1;
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
  size_t cnt = 0;
  for( size_t i=0; i < Crds.Count(); i++ )  {
    if( Crds[i].GetA()->IsDeleted() )  continue;
    item.AddItem(cnt++, Crds[i].GetB()).AddField("atom_id", Crds[i].GetA()->GetTag()); 
  }
}
//..............................................................................
void TSPlane::FromDataItem(TDataItem& item)  {
  Crds.Clear();
  for( size_t i=0; i < item.ItemCount(); i++ )  {
    Crds.AddNew(&Network->GetLattice().GetAtom(item.GetItem(i).GetRequiredField("atom_id").ToInt()), 
      item.GetItem(i).GetValue().ToDouble());
  }
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for( size_t i=0; i < Crds.Count(); i++ )
    points.AddNew( Crds[i].GetA()->crd(), Crds[i].GetB());
  _Init(points);
}
//..............................................................................
TSPlane::Def::Def(const TSPlane& plane) : atoms(plane.Count()), regular(plane.IsRegular())  {
  for( size_t i=0; i < plane.Count(); i++ )
    atoms.Set(i, new DefData(plane.GetAtom(i).GetRef(), plane.GetWeight(i)));
  if( plane.Count() == 0 )  return;
  if( !plane.GetAtom(0).GetMatrix(0).IsFirst() )  {
    const TUnitCell& uc = plane.GetNetwork().GetLattice().GetUnitCell();
    const smatd im = plane.GetAtom(0).GetMatrix(0).Inverse();
    for( size_t i=0; i < atoms.Count(); i++ )  {
      smatd m = im*smatd::FromId(atoms[i].ref.matrix_id, uc.GetMatrix(smatd::GetContainerId(atoms[i].ref.matrix_id)));
      uc.InitMatrixId(m);
      atoms[i].ref.matrix_id = m.GetId();
    }
  }
  atoms.QuickSorter.Sort<TComparableComparator>(atoms);
}
//..............................................................................
TSPlane* TSPlane::Def::FromAtomRegistry(AtomRegistry& ar, size_t def_id, TNetwork* parent, const smatd& matr) const {
  TTypeList<AnAssociation2<TSAtom*, double> > points;
  mat3d equiv;
  equiv.I();
  if( matr.IsFirst() )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* sa = ar.Find(atoms[i].ref);
      if( sa == NULL )  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  else  {
    const TUnitCell& uc = parent->GetLattice().GetUnitCell();
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom::Ref ref = atoms[i].ref;
      smatd m = matr*smatd::FromId(ref.matrix_id, uc.GetMatrix(smatd::GetContainerId(ref.matrix_id)));
      uc.InitMatrixId(m);
      if( i == 0 )
        equiv = m.r;
      ref.matrix_id = m.GetId();
      TSAtom* sa = ar.Find(ref);
      if( sa == NULL )  return NULL;
      points.AddNew(sa, atoms[i].weight);
    }
  }
  TSPlane* p = new TSPlane(parent, def_id);
  p->Init(points);
  p->SetRegular(regular);
  if( !equiv.IsI() )
    p->SetEquiv( new mat3d(equiv));
  return p;
}
