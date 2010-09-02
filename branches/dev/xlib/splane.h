#ifndef __olx_xl_splane_H
#define __olx_xl_splane_H
#include "xbase.h"
#include "typelist.h"
#include "satom.h"

BeginXlibNamespace()

const uint16_t
  plane_best = 1,
  plane_worst = 2;
const uint16_t
  plane_flag_deleted = 0x0001,
  plane_flag_regular = 0x0002;

class TSPlane : public TSObject<TNetwork>  {
  TTypeList< AnAssociation2<TSAtom*, double> > Crds;
  vec3d Center;
  mat3d Normals;
  double Distance;
  uint16_t Flags;
  size_t DefId;
  void _Init(const TTypeList<AnAssociation2<vec3d, double> >& points);
public:
  TSPlane(TNetwork* Parent, size_t def_id = InvalidIndex) : TSObject<TNetwork>(Parent), 
    Distance(0), Flags(0), DefId(def_id)  {}
  virtual ~TSPlane()  {}

  DefPropBFIsSet(Deleted, Flags, plane_flag_deleted)
  // this is just a flag for the owner - is not used by the object itself
  DefPropBFIsSet(Regular, Flags, plane_flag_regular)

  inline size_t CrdCount() const {  return Crds.Count(); }
  // an association point, weight is provided
  void Init(const TTypeList<AnAssociation2<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const {  return Normals[0]; }
  inline const mat3d& GetBasis() const {  return Normals;  }
  inline const vec3d& GetCenter() const {  return Center; }

  double DistanceTo(const vec3d& Crd) const {  return Crd.DotProd(GetNormal()) - Distance;  }
  double DistanceTo(const TSAtom& a) const {  return DistanceTo(a.crd());  }
  double Angle(const vec3d& A, const vec3d& B) const {  return acos(GetNormal().CAngle(B-A))*180/M_PI;  }
  double Angle(const vec3d& v) const {  return acos(GetNormal().CAngle(v))*180/M_PI;  }
  double Angle(const class TSBond& B) const;
  double Angle(const TSPlane& P) const {  return Angle(P.GetNormal());  }
  double GetD() const {  return Distance;  }
  double GetZ(const double& X, const double& Y) const {
    return (GetNormal()[2] == 0) ? 0.0 : (GetNormal()[0]*X + GetNormal()[1]*Y + Distance)/GetNormal()[2];
  }
  size_t Count() const {  return Crds.Count();  }
  const TSAtom& GetAtom(size_t i) const {  return *Crds[i].GetA();  }
  TSAtom& GetAtom(size_t i) {  return *Crds[i].A();  }
  double GetWeight(size_t i) const {  return Crds[i].GetB();  }

// static members
  /* calculates all three planes - best, worst and the complimentary, 
  the normals are sorted by rms ascending, so the best plane is at [0] and the worst - at [2]
  returns true if the function succeded (point cound > 2)
  */
  template <class List> static bool CalcPlanes(const List& Points, 
    mat3d& params, vec3d& rms, vec3d& center, bool sort=true);
  // a convinience function for non-weighted plane
  static bool CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms, vec3d& center);
  /* calculates the A,B and C for the best/worst plane Ax*By*Cz+D=0, D can be calculated as
   D = center.DotProd({A,B,C})
   for the point, weight association
   returns sqrt(smallest eigen value/point.Count())
  */
  template <class List> static double CalcPlane(const List& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // a convinience function for non-weighted planes
  static double CalcPlane(const TSAtomPList& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // returns sqrt(smallest eigen value/point.Count())
  static double CalcRMS(const TSAtomPList& atoms);

  class Def : public ACollectionItem {
    struct DefData {
      TSAtom::Ref ref;
      double weight;
      DefData(const TSAtom::Ref& r, double w) : ref(r), weight(w)  {}
      DefData(const DefData& r) : ref(r.ref), weight(r.weight)  {}
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
    };
    TTypeList<DefData> atoms;
    bool regular;
  public:
    Def(const TSPlane& plane);
    Def(const Def& r) : atoms(r.atoms), regular(r.regular)  {}
    Def& operator = (const Def& r)  {
      atoms = r.atoms;
      regular = r.regular;
      return *this;
    }
    bool operator == (const Def& d)  const {
      if( atoms.Count() != d.atoms.Count() )  return false;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( atoms[i].ref.catom_id != d.atoms[i].ref.catom_id ||
          atoms[i].ref.matrix_id != d.atoms[i].ref.matrix_id )
          return false;
      }
      return true;
    }
    TSPlane* FromAtomRegistry(class AtomRegistry& ar, size_t def_id, class TNetwork* parent, const smatd& matr) const;
  };

  Def GetDef() const { return Def(*this);  }
  size_t GetDefId() const {  return DefId; }
  // not for external use
  void _SetDefId(size_t id)  {  DefId = id; }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;

  
template <class List>  // AnAssociation2<vec3d, double> list, returning & on []
bool TSPlane::CalcPlanes(const List& Points, mat3d& Params, vec3d& rms, vec3d& center, bool sort)  {
  if( Points.Count() < 3 )  return false;
  center.Null();
  double mass = 0;
  center.Null();
  for( size_t i=0; i < Points.Count(); i++ )  {
    center += Points[i].GetA()*Points[i].GetB();
    mass += Points[i].GetB();
  }
  if( mass == 0 )  return false;
  center /= mass;
  mat3d m;
  for( size_t i=0; i < Points.Count(); i++ )  {
    const vec3d t = Points[i].GetA() - center;
    const double wght = olx_sqr(Points[i].GetB());
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
// returns RMS or a negative number if an error occured
template <class List>
double TSPlane::CalcPlane(const List& Points, vec3d& Params, vec3d& center, const short type)  {
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

