#ifndef __olx_ref_model_H
#define __olx_ref_model_H
#include "xbase.h"
#include "experimental.h"

BeginXlibNamespace()

class TRefinementModel  {
    
public:
  TExperimentalDetails expl;
  TSRestraintList rDfix,  // restrained distances (DFIX)
                  rAfix,  // restrained angles (DANG)
                  rDsim,  // similar distances (SADI)
                  rVfix,  // restrained atomic volume (CHIV)
                  rPfix,  // planar groups (FLAT)
                  rRBnd,  // rigid bond restraints (DELU)
                  rUsim,  // similar Uij (SIMU)
                  rUiso,  // Uij components approximate to isotropic behavior (ISOR)
                  rEADP;  // equivalent adp, constraint
                 
  TSameGroupList  rSAME;
  TAfixGroups AfixGroups;
  TExyzGroups ExyzGroups;
}
EndXlibNamespace()

#endif
