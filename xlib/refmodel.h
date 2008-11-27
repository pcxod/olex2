#ifndef __olx_ref_model_H
#define __olx_ref_model_H
#include "asymmunit.h"
#include "xscatterer.h"
#include "experimental.h"

BeginXlibNamespace()

class RefinementModel  {
  // in INS file is EQUV command
  smatd_list UsedSymm;
  TCSTypeList<olxstr, XScatterer*> SfacData;  // label + params
protected:
  olxstr HKLF,
         HKLSource;
  olxstr RefinementMethod,  // L.S. or CGLS
         SolutionMethod;
public:
  RefinementModel(TAsymmUnit& au);
  virtual ~RefinementModel() {  Clear();  }
  ExperimentalDetails expl;
  TSRestraintList rDFIX,  // restrained distances (DFIX)
                  rDANG,  // restrained angles (DANG)
                  rSADI,  // similar distances (SADI)
                  rCHIV,  // restrained atomic volume (CHIV)
                  rFLAT,  // planar groups (FLAT)
                  rDELU,  // rigid bond restraints (DELU)
                  rSIMU,  // similar Uij (SIMU)
                  rISOR,  // Uij components approximate to isotropic behavior (ISOR)
                  rEADP;  // equivalent adp, constraint
  TDoubleList FVAR;                 
  TSameGroupList  rSAME;
  TAfixGroups AfixGroups;
  TExyzGroups ExyzGroups;

  evecd used_weight, proposed_weight;
  eveci LS;      // up to four params
  evecd PLAN;  // up to three params
  
  const olxstr& GetHKLSource() const {  return HKLSource;  }
  //TODO: handle the change
  void SetHKLSource(const olxstr& src) {
    HKLSource = src;
  }

  const olxstr& GetHKLF() const {  return HKLF;  }
  void SetHKLF(const olxstr& hklf) {
    HKLF = hklf;
  }

  const olxstr& GetRefinementMethod() const {  return RefinementMethod;  }
  void SetRefinementMethod(const olxstr& rm) {
    RefinementMethod = rm;
  }

  const olxstr& GetSolutionMethod() const {  return SolutionMethod;  }
  void SetSolutionMethod(const olxstr& sm)  {
    SolutionMethod = sm;
  }
  
  void SetIterations( int v ) {  
    if( LS.Count() == 0 ) LS.Resize(1);
    LS[0] = v;  
  }
  void SetPlan(int v)        {  
    if( PLAN.Count() == 0 )  PLAN.Resize(1);
    PLAN[0] = v;  
  }

  TAsymmUnit& aunit;
  // clears restraints, SFAC and used symm
  void Clear();
  // adss new symmetry matrics, used in restraints/constraints 
  const smatd& AddUsedSymm(const smatd& matr);
  //removes the matrix or decriments the reference count
  void RemUsedSymm(const smatd& matr);
  // returns the number of the used symmetry matrices
  inline int UsedSymmCount()     const {  return UsedSymm.Count();  }
  // returns used symmetry matric at specified index
  inline const smatd& GetUsedSymm(size_t ind)  {  return UsedSymm[ind];  }
  // return index of given symmetry matrix in the list or -1, if it is not in the list
  inline int UsedSymmIndex(const smatd& matr)  const {  return UsedSymm.IndexOf(matr);  }
  // deletes all used symmetry matrices
  inline void ClearUsedSymm()          {  UsedSymm.Clear();  }
  
  // adds new custom scatterer
  void AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double mu, double r, double wt);
  // returns number of custom scatterers
  inline int SfacCount()  const  {  return SfacData.Count();  }
  // returns scatterer label at specified index
  inline const olxstr& GetSfacLabel(size_t index) const  {
    return SfacData.GetComparable(index);
  }
  // returns scatterer at specified index
  inline XScatterer& GetSfacData(size_t index) const  {
    return *SfacData.GetObject(index);
  }
  // finds scatterer by label, returns NULL if nothing found
  inline XScatterer* FindSfacData(const olxstr& label) const  {
    return SfacData[label];
  }
  // returns the restrained distance or -1
  double FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2);
  
  RefinementModel& Assign(const RefinementModel& rm, bool AssignAUnit);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()

#endif
