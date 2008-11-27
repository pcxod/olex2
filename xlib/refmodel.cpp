#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"

RefinementModel::RefinementModel(TAsymmUnit& au) : rDFIX(*this, rltBonds), rDANG(*this, rltBonds), 
  rSADI(*this, rltBonds), rCHIV(*this, rltAtoms), rFLAT(*this, rltGroup), rDELU(*this, rltAtoms), 
  rSIMU(*this, rltAtoms), rISOR(*this, rltAtoms), rEADP(*this, rltAtoms), 
  aunit(au)  {
  HKLF = "4";
}
//....................................................................................................
void RefinementModel::Clear() {
  for( int i=0; i < SfacData.Count(); i++ )
    delete SfacData.Object(i);
  SfacData.Clear();
  rDFIX.Clear();
  rDANG.Clear();
  rSADI.Clear();
  rCHIV.Clear();
  rFLAT.Clear();
  rSIMU.Clear();
  rDELU.Clear();
  rISOR.Clear();
  rEADP.Clear();
  rSAME.Clear();
  ExyzGroups.Clear();
  AfixGroups.Clear();
  UsedSymm.Clear();
  used_weight.Resize(0);
  proposed_weight.Resize(0);
  FVAR.Clear();
  HKLF = "4";
  expl.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod = EmptyString;
  HKLSource = EmptyString;
}
//....................................................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr) {
  int ind = UsedSymm.IndexOf(matr);
  smatd* rv = NULL;
  if( ind == -1 )  {
    rv = &UsedSymm.Add( *(new smatd(matr)) );
    rv->SetTag(1);
  }
  else  {
    UsedSymm[ind].IncTag();
    rv = &UsedSymm[ind];
  }
  return *rv;
}
//....................................................................................................
void RefinementModel::RemUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOf(matr);
  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
  UsedSymm[ind].DecTag();
  if( UsedSymm[ind].GetTag() == 0 )
    UsedSymm.Delete(ind);
}
//....................................................................................................
RefinementModel& RefinementModel::Assign(const RefinementModel& rm, bool AssignAUnit) {
  Clear();
  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  LS = rm.LS;
  PLAN = rm.PLAN;
  HKLF = rm.HKLF;
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;
  FVAR = rm.FVAR;
  if( AssignAUnit )
    aunit.Assign(rm.aunit);
  
  rDFIX.Assign(rm.rDFIX);
  rDANG.Assign(rm.rDANG);
  rSADI.Assign(rm.rSADI);
  rCHIV.Assign(rm.rCHIV);
  rFLAT.Assign(rm.rFLAT);
  rSIMU.Assign(rm.rSIMU);
  rDELU.Assign(rm.rDELU);
  rISOR.Assign(rm.rISOR);
  rEADP.Assign(rm.rEADP);
  rSAME.Assign(aunit, rm.rSAME);
  ExyzGroups.Assign(aunit, rm.ExyzGroups);
  AfixGroups.Assign(aunit, rm.AfixGroups);

  for( int i=0; i < rm.UsedSymm.Count(); i++ )
    UsedSymm.AddCCopy( rm.UsedSymm[i] );

  for( int i=0; i < rm.SfacData.Count(); i++ )
    SfacData.Add(rm.SfacData.GetComparable(i), new XScatterer( *rm.SfacData.GetObject(i)) );
  
  
  return *this;
}
//....................................................................................................
void RefinementModel::AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double mu, double r, double wt)  {
  olxstr lb(label.CharAt(0) == '$' ? label.SubStringFrom(1) : label);
  cm_Element* src = XElementLib::FindBySymbol(lb);
  XScatterer* sc;
  if( src != NULL )
    sc = new XScatterer(*src, expl.GetRadiationEnergy());
  else
    sc = new XScatterer;
  sc->SetLabel(lb);
  sc->SetGaussians(a1, a2, a3, a4, b1, b2, b3, b4, c);
  sc->SetAdsorptionCoefficient(mu);
  sc->SetBondingR(r);
  sc->SetWeight(wt);
  SfacData.Add(label, sc);
}
//....................................................................................................
double RefinementModel::FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2)  {
  for(int i=0; i < rDFIX.Count(); i++ )  {
    for( int j=0; j < rDFIX[i].AtomCount(); j+=2 )  {
      if( (rDFIX[i].GetAtom(j).GetAtom() == &a1 && rDFIX[i].GetAtom(j+1).GetAtom() == &a2) ||
          (rDFIX[i].GetAtom(j).GetAtom() == &a2 && rDFIX[i].GetAtom(j+1).GetAtom() == &a1) )  {
        return rDFIX[i].GetValue();
      }
    }
  }
  return -1;
}
//....................................................................................................
void RefinementModel::ToDataItem(TDataItem& item) const {
  // save used equivalent positions
  TDataItem& eqiv = item.AddItem("eqiv");
  for( int i=0; i < UsedSymm.Count(); i++ )  
    eqiv.AddItem(i, TSymmParser::MatrixToSymmEx(UsedSymm[i]));

  AfixGroups.ToDataItem(item.AddItem("afix"));
  ExyzGroups.ToDataItem(item.AddItem("exyz"));
  rSAME.ToDataItem(item.AddItem("same"));
  rDFIX.ToDataItem(item.AddItem("dfix"));
  rDANG.ToDataItem(item.AddItem("dang"));
  rSADI.ToDataItem(item.AddItem("sadi"));
  rCHIV.ToDataItem(item.AddItem("chiv"));
  rFLAT.ToDataItem(item.AddItem("flat"));
  rDELU.ToDataItem(item.AddItem("delu"));
  rSIMU.ToDataItem(item.AddItem("simu"));
  rISOR.ToDataItem(item.AddItem("isor"));
  rEADP.ToDataItem(item.AddItem("eadp"));
  
}
//....................................................................................................
void RefinementModel::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//....................................................................................................
