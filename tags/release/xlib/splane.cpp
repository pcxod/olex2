//---------------------------------------------------------------------------//
// TSPlane implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "splane.h"
#include "ematrix.h"
#include "satom.h"
#include "sbond.h"
#include "lattice.h"
#include "pers_util.h"

//..............................................................................
double TSPlane::CalcRMS(const TSAtomPList& atoms)  {
  if( atoms.Count() < 3 )  return -1;
  vec3d p, c;
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity( atoms.Count() );
  for( size_t i=0; i < atoms.Count(); i++ )
    Points.AddNew( atoms[i]->crd(), 1);
  return CalcPlane(Points, p, c);
}
//..............................................................................
void TSPlane::Init(const TTypeList< AnAssociation2<TSAtom*, double> >& atoms)  {
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    points.AddNew( atoms[i].GetA()->crd(), atoms[i].GetB());
  CalcPlane(points, FNormal, FCenter);
  FDistance = FNormal.DotProd(FCenter)/FNormal.Length();
  FNormal.Normalise();
  Crds.Clear();
  Crds.AddListC(atoms);
}
//..............................................................................
bool TSPlane::CalcPlanes(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
                        mat3d& Params, vec3d& rms, vec3d& center)  
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
  for( int i=0; i < 3; i++ )  {
    if( m[i][i] < 0 ) // optimised version will create slightly negative values!
      rms[i] = 0;
    else
      rms[i] = sqrt(m[i][i]/Points.Count());
  }
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
  return true;
}
//..............................................................................
bool TSPlane::CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms, vec3d& center) {
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    Points.AddNew( atoms[i]->crd(), 1.0 );
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
    Points.AddNew( atoms[i]->crd(), 1.0 );
  return CalcPlane(Points, Params, center, type);
}
//..............................................................................
double TSPlane::Angle(const TSBond& Bd) const  {
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
void TSPlane::FromDataItem(TDataItem& item) {
  Crds.Clear();
  for( size_t i=0; i < item.ItemCount(); i++ )  {
    Crds.AddNew( &Network->GetLattice().GetAtom(item.GetItem(i).GetRequiredField("atom_id").ToInt()), 
      item.GetItem(i).GetValue().ToDouble());
  }
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for( size_t i=0; i < Crds.Count(); i++ )
    points.AddNew( Crds[i].GetA()->crd(), Crds[i].GetB());
  CalcPlane(points, FNormal, FCenter);
  FDistance = FNormal.DotProd(FCenter)/FNormal.Length();
  FNormal.Normalise();
}
//..............................................................................
