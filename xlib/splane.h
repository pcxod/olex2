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
  vec3d Center, Normal;
  double Distance;
  uint16_t Flags;
  size_t DefId;
  uint8_t NormalIndex;
  mat3d* Equiv;
  void _Init(const TTypeList<AnAssociation2<vec3d, double> >& points);
public:
  TSPlane(TNetwork* Parent, size_t def_id = InvalidIndex) : TSObject<TNetwork>(Parent), 
    Distance(0), Flags(0), DefId(def_id), Equiv(NULL)  {}
  virtual ~TSPlane()  {
    if( Equiv != NULL )
      delete Equiv;
  }

  DefPropBFIsSet(Deleted, Flags, plane_flag_deleted)
  // this is just a flag for the owner - is not used by the object itself
  DefPropBFIsSet(Regular, Flags, plane_flag_regular)

  inline size_t CrdCount() const {  return Crds.Count(); }
  // an association point, weight is provided
  void Init(const TTypeList<AnAssociation2<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const {  return Normal; }
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
  static bool CalcPlanes(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
    mat3d& params, vec3d& rms, vec3d& center, bool sort=true);
  // a convinience function for non-weighted plane
  static bool CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms, vec3d& center);
  /* calculates the A,B and C for the best/worst plane Ax*By*Cz+D=0, D can be calculated as
   D = center.DotProd({A,B,C})
   for the point, weight association
   returns sqrt(smallest eigen value/point.Count())
  */
  static double CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
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
        int diff = olx_cmp_size_t(ref.catom_id, d.ref.catom_id);
        if( diff == 0 )
          diff = olx_cmp_size_t(ref.matrix_id, d.ref.matrix_id);
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
  /* identifies the transformation from plane where the first atom is in the AU */
  const mat3d* GetEquiv() {  return Equiv;  }
  /* must be called with object created by new, it wil be deleted in the destructor */
  void SetEquiv(mat3d* m)  {
    if( Equiv != NULL )  delete Equiv;
    Equiv = m;
  }
  size_t GetDefId() const {  return DefId; }
  // not for external use
  void _SetDefId(size_t id)  {  DefId = id; }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;
EndXlibNamespace()
#endif

