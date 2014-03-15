/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_splane_H
#define __olx_xl_splane_H
#include "xbase.h"
#include "typelist.h"
#include "satom.h"

BeginXlibNamespace()

class TAtomEnvi;

const uint16_t
  plane_best = 1,
  plane_worst = 2;
const uint16_t
  plane_flag_deleted = 0x0001,
  plane_flag_regular = 0x0002;

class TSPlane : public TSObject<TNetwork>  {
  TTypeList<olx_pair_t<TSAtom*, double> > Crds;
  vec3d Center;
  mat3d Normals;
  double Distance, wRMSD;  uint16_t Flags;
  size_t DefId;
  void _Init(const TTypeList<olx_pair_t<vec3d, double> >& points);
  // for Crd sorting by atom tag
  struct AtomTagAccessor {
    index_t operator() (
      const olx_pair_t<TSAtom*, double> *p) const
    {
      return p->GetA()->GetTag();
    }
  };
public:
  TSPlane() :TSObject<TNetwork>(NULL) {}
  TSPlane(TNetwork* Parent, size_t def_id = InvalidIndex)
    : TSObject<TNetwork>(Parent), Distance(0), wRMSD(0), Flags(0),
      DefId(def_id)  {}
  virtual ~TSPlane()  {}

  DefPropBFIsSet(Deleted, Flags, plane_flag_deleted)
  // this is just a flag for the owner - is not used by the object itself
  DefPropBFIsSet(Regular, Flags, plane_flag_regular)

  // an association point, weight is provided
  void Init(const TTypeList<olx_pair_t<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const {  return Normals[0]; }
  // note that the Normal (or Z) is the first row of the matrix
  inline const mat3d& GetBasis() const {  return Normals;  }
  inline const vec3d& GetCenter() const {  return Center; }

  double DistanceTo(const vec3d& Crd) const {
    return Crd.DotProd(GetNormal()) - Distance;
  }
  double DistanceTo(const TSAtom& a) const {  return DistanceTo(a.crd());  }
  double Angle(const vec3d& A, const vec3d& B) const {
    return acos(GetNormal().CAngle(B-A))*180/M_PI;
  }
  double Angle(const vec3d& v) const {
    return acos(GetNormal().CAngle(v))*180/M_PI;
  }
  double Angle(const class TSBond& B) const;
  double Angle(const TSPlane& P) const {  return Angle(P.GetNormal());  }
  double GetD() const {  return Distance;  }
  // direct calculation with unit weights
  double CalcRMSD() const;
  // calculation with actual weights (should be the same as GetWeightedRMSD)
  double CalcWeightedRMSD() const;
  double GetWeightedRMSD() const {  return wRMSD;  }
  double GetZ(const double& X, const double& Y) const {
    return (GetNormal()[2] == 0) ? 0.0
      : (GetNormal()[0]*X + GetNormal()[1]*Y + Distance)/GetNormal()[2];
  }
  size_t Count() const {  return Crds.Count();  }
  const TSAtom& GetAtom(size_t i) const {  return *Crds[i].GetA();  }
  TSAtom& GetAtom(size_t i) {  return *Crds[i].a;  }
  double GetWeight(size_t i) const {  return Crds[i].GetB();  }
  void _PlaneSortByAtomTags() {
    InsertSorter::Sort(Crds,
      ComplexComparator::Make(AtomTagAccessor(), TPrimitiveComparator()),
      DummySortListener());
  }
  /* returns inverse intersects with the lattice vectors, the vector is divided
  by modulus of the smallest non-zero value. Takes the orthogonalisation matrix
  plane normal and a point on the plane
  */
  static vec3d GetCrystallographicDirection(const mat3d &m,
    const vec3d &n, const vec3d &p);
  vec3d GetCrystallographicDirection() const;
// static members
  /* calculates all three planes - best, worst and the complimentary, 
  the normals are sorted by rms ascending, so the best plane is at [0] and the
  worst - at [2] Returns true if the function succeded (point cound > 2)
  */
  template <class List> static bool CalcPlanes(const List& Points, 
    mat3d& params, vec3d& rms, vec3d& center, bool sort=true);
  // a convinience function for non-weighted plane
  static bool CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms,
    vec3d& center);
  /* calculates the A,B and C for the best/worst plane Ax*By*Cz+D=0, D can be
  calculated as D = center.DotProd({A,B,C})
   for the point, weight association
   returns sqrt(smallest eigen value/point.Count())
  */
  template <class List> static double CalcPlane(const List& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // a convinience function for non-weighted planes
  static double CalcPlane(const TSAtomPList& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // returns sqrt(smallest eigen value/point.Count())
  static double CalcRMSD(const TSAtomPList& atoms);
  static double CalcRMSD(const TAtomEnvi& atoms);
  olxstr StrRepr() const;

  class Def : public ACollectionItem {
    struct DefData {
      TSAtom::Ref ref;
      double weight;
      DefData() {}
      DefData(const TSAtom::Ref& r, double w) : ref(r), weight(w)  {}
      DefData(const DefData& r) : ref(r.ref), weight(r.weight)  {}
      DefData(const TDataItem& item) : ref(~0,~0) {  FromDataItem(item);  }
      DefData& operator = (const DefData& r)  {  
        ref = r.ref;  
        weight = r.weight;
        return *this;
      }
      int Compare(const DefData& d) const {
        int diff = olx_cmp(ref.catom_id, d.ref.catom_id);
        if( diff == 0 )
          diff = olx_cmp(ref.matrix_id, d.ref.matrix_id);
        return diff;
      }
      void ToDataItem(TDataItem& item) const;
      void FromDataItem(const TDataItem& item);
    };
    TTypeList<DefData> atoms;
    bool regular;
  public:
    Def() {}
    Def(const TSPlane& plane);
    Def(const Def& r) : atoms(r.atoms), regular(r.regular)  {}
    Def(const TDataItem& item)  {  FromDataItem(item);  }
    Def& operator = (const Def& r)  {
      atoms = r.atoms;
      regular = r.regular;
      return *this;
    }
    bool operator == (const Def& d)  const {
      if( atoms.Count() != d.atoms.Count() )  return false;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( atoms[i].ref.catom_id != d.atoms[i].ref.catom_id ||
          atoms[i].ref.matrix_id != d.atoms[i].ref.matrix_id ||
          atoms[i].weight != d.atoms[i].weight )
          return false;
      }
      return true;
    }
    TSPlane* FromAtomRegistry(struct ASObjectProvider& ar, size_t def_id,
      class TNetwork* parent, const smatd& matr) const;
    void ToDataItem(TDataItem& item) const;
    void FromDataItem(const TDataItem& item);
  };

  Def GetDef() const { return Def(*this);  }
  size_t GetDefId() const {  return DefId; }
  // not for external use
  void _SetDefId(size_t id)  {  DefId = id; }
  
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;

/* RMSD will be 'valid', i.e. as equal to directly calculated only for
unit/equal weights, otherwise it will become smaller than directly calculated
one since the priority will be given to some points and
RMSD'=(sum(w^2*distances^2)/sum(w^2))^0.5, where distance will be smaller for
higher weights... there are functions to calculate both values 
*/
template <class List>  // olx_pair_t<vec3d, double> list, returning & on []
bool TSPlane::CalcPlanes(const List& Points, mat3d& Params, vec3d& rms,
  vec3d& center, bool sort)
{
  if( Points.Count() < 3 )  return false;
  center.Null();
  double mass = 0, qmass = 0;
  center.Null();
  for( size_t i=0; i < Points.Count(); i++ )  {
    center += Points[i].GetA()*Points[i].GetB();
    mass += Points[i].GetB();
    qmass += olx_sqr(Points[i].GetB());
  }
  if( mass == 0 )  return false;
  center /= mass;
  mat3d m;
  for( size_t i=0; i < Points.Count(); i++ )  {
    const vec3d t = (Points[i].GetA() - center)*Points[i].GetB();
    m[0][0] += (t[0]*t[0]);
    m[0][1] += (t[0]*t[1]);
    m[0][2] += (t[0]*t[2]);
    m[1][1] += (t[1]*t[1]);
    m[1][2] += (t[1]*t[2]);
    m[2][2] += (t[2]*t[2]);
  } 
  m[1][0] = m[0][1];
  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  mat3d::EigenValues(m /= qmass, Params.I());
  if( sort )  {
    for( int i=0; i < 3; i++ )
      rms[i] = (m[i][i] < 0 ? 0 : sqrt(m[i][i]));
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
// returns RMS or a negative number if an error occured
template <class List>
double TSPlane::CalcPlane(const List& Points, vec3d& Params, vec3d& center,
  const short type)
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

EndXlibNamespace()
#endif

