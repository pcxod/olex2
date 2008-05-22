#ifndef splaneH
#define splaneH

#include "xbase.h"
#include "elist.h"
#include "vpoint.h"
#include "typelist.h"
#include "tptrlist.h"
#include "satom.h"

BeginXlibNamespace()

struct TPlaneSort  {  // used in sort of plane points
  TVPointD V;
  TVPointD *Crd;
};

class TSPlane:public TSObject
{
private:
//  int FTag;
  TVPointDList Crds;
  TVPointD FNormal, FCenter;
  double FDistance;
  bool  Deleted;
public:
  TSPlane(TNetwork* Parent);
  virtual ~TSPlane();

  DefPropB(Deleted)

  inline int CrdCount()              const {  return Crds.Count(); }
  void AddCrd(const TVPointD& Crd);
  TVPointD& Crd(int i)                     {  return Crds[i]; }
  const TVPointD& GetCrd(int i)      const {  return Crds[i]; }

  inline TVPointD& Normal()                {  return FNormal; }
  inline TVPointD& Center()                {  return FCenter; }
  inline const TVPointD& GetNormal() const {  return FNormal; }
  inline const TVPointD& GetCenter() const {  return FCenter; }

  double DistanceTo(const TVPointD &Crd) const;
  double DistanceTo(const TSAtom& A) const;
  double Angle( const TVPointD &A,  const TVPointD &B) const;
  double Angle( const class TSBond& B) const;
  double Angle( const TSPlane& P) const;
  double D() const {  return FDistance; }
  double Z(double X, double Y) const;
  void D(double v) {  FDistance = v; }
// static members
  static void CalcPlane(const TTypeList< AnAssociation2<TVPointD, double> >& Points, TVectorD& Params);
  // returns a summ of sqrt( (distances from atoms to the plane)^2) divided by the number of atoms
  static double CalcRMS(const TSAtomPList& atoms);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;
EndXlibNamespace()
#endif

