#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "ipattern.h"
#include "chnexp.h"
#include "atominfo.h"
#include "estrlist.h"
#include "idistribution.h"
#include "bapp.h"
#include "log.h"


int TSPoint::SPointsSortA(const void *I, const void *I1)
{
  if( ((TSPoint*)I)->Y < ((TSPoint*)I1)->Y )  return 1;
  if( ((TSPoint*)I)->Y > ((TSPoint*)I1)->Y )  return -1;
  return 0;
}
//..............................................................................
int TSPoint::SPointsSortB(const void *I, const void *I1)
{
  if( ((TSPoint*)I)->X < ((TSPoint*)I1)->X )  return 1;
  if( ((TSPoint*)I)->X > ((TSPoint*)I1)->X )  return -1;
  return 0;
}

//---------------------------------------------------------------------------
TIPattern::TIPattern(TAtomsInfo *AI)
{
  Points = new TEList;
  AtomsInfo = AI;
}
//..............................................................................
TIPattern::~TIPattern()
{
  Clear();
  delete Points;
}
//..............................................................................
void TIPattern::Clear()
{
  for( int i=0; i < Points->Count(); i++ )
    delete (TSPoint*)Points->Item(i);
  Points->Clear();
}
//..............................................................................
bool TIPattern::Calc(olxstr Exp, olxstr &Msg, bool Combine, double Delta)  {
  Clear();
  olxstr Tmp;
  TCHNExp CHN(AtomsInfo);
  TStrPObjList<olxstr,double> SL;
  TIDistribution ID;
  TBasicAtomInfo *AI;
  CHN.LoadFromExpression(Exp);
  CHN.MolWeight(); // to check the correctness of the formula
  double MaxY;
  if( Msg.Length() )
  {
    return false;
  }
  CHN.CalcSummFormula(SL);
  for( int i=0; i < SL.Count(); i++ )  {
    AI = AtomsInfo->FindAtomInfoBySymbol( SL[i] );
    int occupancy = (int)SL.GetObject(i);
    if( occupancy == 0 )  {
      occupancy++;
      TBasicApp::GetLog().Error(
        olxstr("The occupancy is set to 1 for ") << AI->GetSymbol() << " the molecular weight might be incorrect");
    }
    ID.AddIsotope(AI, occupancy);
  }
  ID.Calc(Points);
  if( Points->Count() != 0 )  {
    if( Combine )
      TIDistribution::CombineSerie(Points, Delta);
    Points->Sort(TSPoint::SPointsSortA);
    // normalisation of the serie
    MaxY = ((TSPoint*)Points->Item(0))->Y;
    for( int i=0; i < Points->Count(); i++ )
      ((TSPoint*)Points->Item(i))->Y = ((TSPoint*)Points->Item(i))->Y/MaxY*100; // percents
  }
  return true;
}
//..............................................................................
void TIPattern::SortDataByMolWeight(){  Points->Sort( TSPoint::SPointsSortB);  }
//..............................................................................
void TIPattern::SortDataByItensity() {  Points->Sort( TSPoint::SPointsSortA);  }
//..............................................................................

