#include "ipattern.h"
#include "chnexp.h"
#include "atominfo.h"
#include "estrlist.h"
#include "idistribution.h"
#include "bapp.h"
#include "log.h"

//..............................................................................
void TIPattern::Clear()  {  Points.Clear();  }
//..............................................................................
bool TIPattern::Calc(const olxstr& Exp, olxstr& Msg, bool Combine, double Delta)  {
  Clear();
  olxstr Tmp;
  TCHNExp CHN;
  TStrPObjList<olxstr,double> SL;
  TIDistribution ID;
  CHN.LoadFromExpression(Exp);
  CHN.MolWeight(); // to check the correctness of the formula
  if( !Msg.IsEmpty() )  {
    return false;
  }
  CHN.CalcSummFormula(SL);
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  for( size_t i=0; i < SL.Count(); i++ )  {
    TBasicAtomInfo* AI = AtomsInfo.FindAtomInfoBySymbol( SL[i] );
    if( AI == NULL )
      return false;
    int occupancy = (int)SL.GetObject(i);
    if( occupancy == 0 )  {
      occupancy++;
      TBasicApp::GetLog().Error(
        olxstr("The occupancy is set to 1 for ") << AI->GetSymbol() << " the molecular weight might be incorrect");
    }
    ID.AddIsotope(*AI, occupancy);
  }
  ID.Calc(Points);
  if( !Points.IsEmpty() )  {
    if( Combine )
      TIDistribution::CombineSerie(Points, Delta);
    Points.QuickSorter.SortSF(Points, &TSPoint::SPointsSortA);
    // normalisation of the serie
    double MaxY = 100.0/Points[0].Y; // normalisation factor
    for( size_t i=0; i < Points.Count(); i++ )
      Points[i].Y *= MaxY; 
  }
  return true;
}
//..............................................................................
void TIPattern::SortDataByMolWeight(){  Points.QuickSorter.SortSF(Points, &TSPoint::SPointsSortB);  }
//..............................................................................
void TIPattern::SortDataByItensity() {  Points.QuickSorter.SortSF(Points, &TSPoint::SPointsSortA);  }
//..............................................................................

