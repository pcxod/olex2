/*
OLEX crystallographic model, (c) O Dolomanov, 2008
*/
// !!!! change to ifdef
#ifdef __OLX_XMODEL_H
#define __OLX_XMODEL_H

#include "xbase.h"
#include "scat_it.h"
#include "tprlist.h"
#include "estrlist.h"

BeginXlibNamespace()

struct TRefinable {
  double Value;
  bool Fixed;
  TRefinable()  {
    Value = 0;
    Fixed = false;
  }
};

struct TTDP {  // thermal dispalacement parameter (isotropic/anisotropic)
  TRefinable Uiso, // isotr
             U11, U22, U33, U12, U13, U23; 
  bool Anisotropic;
  TADP()  {
    Anisotropic = false;
  }
};

struct TScatterer {
  TLibScatterer *Scatterer;
  TRefinable Occupancy;
  TTDP DP;
};

struct TXSite : public TScatterer {
  olxstr Label;
  TRefinable X, Y, Z;
  TXSite()  { }
};

struct TXDisorderedSite : public TXSite {
  TRefinable Occupancy;  // total occupancy
  TPtrList<TXSite> Sites;
  bool EADP;  // are the scatter ADPS equal?
  TXDisorderedSite()  {
    EADP = true;
  }
  inline int Count() const {  return Sites.Count();  }
  inline const TTDP& GetTDP(int i=0)  const {  return Sites[i].DP();  }
};

struct TXSharedSite : public TXDisorderedSite {
  TRefinable X, Y, Z;
  olxstr Label;
};

struct TXRidgidGroup : public TXSite {
  TPtrList<TXSite> Sites;
  olxstr Label;
};

struct TXRestraint  {
  TPtrList<TXSite> Sites;
  double Value, Esd;
};

class TXModel  {
  TStrPObjList<olxstr, TXSite*> Sites;
  TStrPObjList<olxstr, TXRidgidGroup*> RigidGroups;
  TPtrList<TXRestraint> RDistances, RAngles, RVolumes;  // restrained to a vlaue
  TPtrList<TXRestraint> SDistances, SAngles, SVolumes;  // similar values
public:
  
};

EndXlibNamespace()
#endif

