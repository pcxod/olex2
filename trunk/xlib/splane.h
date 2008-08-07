#ifndef splaneH
#define splaneH

#include "xbase.h"
#include "elist.h"
#include "typelist.h"
#include "tptrlist.h"
#include "satom.h"

BeginXlibNamespace()

class TSPlane:public TSObject  {
private:
//  int FTag;
  vec3d_list Crds;
  vec3d FNormal, FCenter;
  double FDistance;
  bool  Deleted;
public:
  TSPlane(TNetwork* Parent);
  virtual ~TSPlane();

  DefPropB(Deleted)

  inline int CrdCount()              const {  return Crds.Count(); }
  // an association point, weight is provided
  void Init(const TTypeList< AnAssociation2<vec3d, double> >& Points);
  vec3d& Crd(int i)                     {  return Crds[i]; }
  const vec3d& GetCrd(int i)      const {  return Crds[i]; }

  inline vec3d& Normal()                {  return FNormal; }
  inline vec3d& Center()                {  return FCenter; }
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
// static members
  /* calculates the A,B and C for the best plane Ax*By*Cz+D=0, D can be calculated as
   D = center.DotProd({A,B,C})
   for the point, weight association
  */
  static void CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
    vec3d& Params, vec3d& center);
  // returns a summ of sqrt( (distances from atoms to the plane)^2) divided by the number of atoms
  static double CalcRMS(const TSAtomPList& atoms);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;
EndXlibNamespace()
#endif

