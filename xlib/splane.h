#ifndef splaneH
#define splaneH

#include "xbase.h"
#include "elist.h"
#include "typelist.h"
#include "tptrlist.h"
#include "satom.h"

BeginXlibNamespace()

const short plane_best = 1,
            plane_worst = 2;

class TSPlane : public TSObject<TNetwork>  {
private:
  TTypeList< AnAssociation2<TSAtom*, double> > Crds;
  vec3d FNormal, FCenter;
  double FDistance;
  bool  Deleted, Regular;
public:
  TSPlane(TNetwork* Parent);
  virtual ~TSPlane();

  DefPropB(Deleted)

  inline int CrdCount()              const {  return Crds.Count(); }
  // an association point, weight is provided
  void Init(const TTypeList< AnAssociation2<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const {  return FNormal; }
  inline const vec3d& GetCenter() const {  return FCenter; }

  double DistanceTo(const vec3d &Crd) const;
  double DistanceTo(const TSAtom& A) const;
  double Angle( const vec3d &A,  const vec3d &B) const;
  double Angle( const class TSBond& B) const;
  double Angle( const TSPlane& P) const;
  double D() const {  return FDistance; }
  double Z(double X, double Y) const;
  void D(double v) {  FDistance = v; }
  int Count() const {  return Crds.Count();  }
  const TSAtom& GetAtom(int i) const {  return *Crds[i].GetA();  }
  TSAtom& Atom(int i) {  return *Crds[i].A();  }
  double Weight(int i) const {  return Crds[i].GetB();  }

  // this is justa flag for the owner - is not used by the object itself
  DefPropB(Regular)

// static members
  /* calculates the A,B and C for the best plane Ax*By*Cz+D=0, D can be calculated as
   D = center.DotProd({A,B,C})
   for the point, weight association
   returns sqrt(minimal eigen value/point.Count())
  */
  static double CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  static double CalcPlane(const TSAtomPList& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // returns sqrt(minimal eigen value/point.Count())
  static double CalcRMS(const TSAtomPList& atoms);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;
EndXlibNamespace()
#endif

