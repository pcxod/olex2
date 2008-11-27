#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"
#include "hkl.h"

RefinementModel::RefinementModel(TAsymmUnit& au) : rDFIX(*this, rltBonds), rDANG(*this, rltBonds), 
  rSADI(*this, rltBonds), rCHIV(*this, rltAtoms), rFLAT(*this, rltGroup), rDELU(*this, rltAtoms), 
  rSIMU(*this, rltAtoms), rISOR(*this, rltAtoms), rEADP(*this, rltAtoms), 
  aunit(au)  {
  SetDefaults();
}
//....................................................................................................
void RefinementModel::SetDefaults() {
  HKLF = 4;
  HKLF_s = def_HKLF_s;
  HKLF_mat.I();
  HKLF_wt = def_HKLF_wt;
  HKLF_m = def_HKLF_m;
  MERG = def_MERG;
  OMIT_s = def_OMIT_s;
  OMIT_2t = def_OMIT_2t;
  OMIT_set = TWIN_set = false;
  TWIN_n = def_TWIN_n;
  TWIN_mat.I() *= -1;
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
  expl.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod = EmptyString;
  HKLSource = EmptyString;
  Omits.Clear();
  BASF.Clear();
  SetDefaults();
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
  HKLF_s = rm.HKLF_s;
  HKLF_mat = rm.HKLF_mat;
  HKLF_wt = rm.HKLF_wt;
  HKLF_m = rm.HKLF_m;
  MERG = rm.MERG;
  OMIT_s = rm.OMIT_s;
  OMIT_2t = rm.OMIT_2t;
  OMIT_set = rm.OMIT_set;
  Omits = rm.Omits;
  TWIN_mat = rm.TWIN_mat;
  TWIN_n = rm.TWIN_n;
  TWIN_set = rm.TWIN_set;
  BASF = rm.BASF;
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
void RefinementModel::SetHKLSource(const olxstr& src) {
  if( HKLSource == src )  return;
  HKLSource = src;
  //if( TEFile::FileExists(src) )  {
  //  THklFile hf;
  //  hf.LoadFromFile(src);
  //  hf.
  //}
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
