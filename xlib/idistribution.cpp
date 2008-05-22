//----------------------------------------------------------------------------//
// namespace TChemObj: TIDistribution
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "idistribution.h"
#include "atominfo.h"
#include "ipattern.h"

//..............................................................................
bool PAddEvaluator(TPolynomMember *PM)
{
  static double MaxC = 0;
  if( !PM )
  {
    MaxC = 0;
    return false; // does not matter
  }
  TPMember *P;
  TIsotope *Is;
  double W = 1;
  TEList *L = PM->Members();
  for( int i=0; i < L->Count(); i++ )
  {
    P = (TPMember*)L->Item(i);
    Is = (TIsotope*)P->Data;
    for( int j=0; j < P->Extent; j++ )
      W *= Is->GetW();
  }
  W *= PM->GetMult();
  if( W > MaxC )
  {
    MaxC = W;
    return true;
  }
  if( MaxC )
  {
    if( W/MaxC < 0.001 )
      return false;
    else
      return true;
  }
  return true;
}
//..............................................................................
double PEvaluator(TPolynomMember *PM)
{
  TPMember *P;
  TIsotope *Is;
  double W = 1;
  TEList *L = PM->Members();
  for( int i=0; i < L->Count(); i++ )
  {
    P = (TPMember*)L->Item(i);
    Is = (TIsotope*)P->Data;
    for( int j=0; j < P->Extent; j++ )
      W *= Is->GetW();
  }
  W *= PM->GetMult();
  return W;
}
//..............................................................................
int  PPolyEvaluator(const void *V, const void *V1)
{
  TPolynomMember  *PM  = (TPolynomMember*)V,
          *PM1 = (TPolynomMember*)V1;
  TPMember *P;
  TIsotope *Is;
  double W = PM->GetMult(),
        W1=PM1->GetMult();
  TEList *L = PM->Members();
  int i;
  for( i=0; i < L->Count(); i++ )
  {
    P = (TPMember*)L->Item(i);
    Is = (TIsotope*)P->Data;
    for( int j=0; j < P->Extent; j++ )
      W *= Is->GetW();
  }
  L = PM1->Members();
  for( i=0; i < L->Count(); i++ )
  {
    P = (TPMember*)L->Item(i);
    Is = (TIsotope*)P->Data;
    for( int j=0; j < P->Extent; j++ )
      W1 *= Is->GetW();
  }
  if( W < W1 )  return 1;
  if( W > W1 )  return -1;
  return 0;
}
//..............................................................................
int  _PolynomesSort(const void *I, const void *I1)
{
  return ((TPolynom*)I)->Members()->Count() - ((TPolynom*)I1)->Members()->Count();
}
//..............................................................................
int  _SeriesSort(const void *I, const void *I1)
{
  return ((TEList*)I)->Count() - ((TEList*)I1)->Count();
}
//..............................................................................
int  _SerieSort(const void *I, const void *I1)
{
  TSPoint *P = (TSPoint*)I, *P1 = (TSPoint*)I1;
  if( P->X < P1->X )  return 1;
  if( P->X > P1->X )  return -1;
  return 0;
}
//----------------------------------------------------------------------------//
// TIsotopeData 
//----------------------------------------------------------------------------//
TIsotopeData::TIsotopeData()
{
  M = 0;
  W = 1;
  Children = new TEList;
}
//..............................................................................
TIsotopeData::~TIsotopeData()
{
  for( int i=0; i < Children->Count(); i++ )
    delete (TIsotopeData*)Children->Item(i);
  delete Children;
}
//..............................................................................
void TIsotopeData::Evail(TEList *S, double &eM, double &eW)
{
  double cM, cW;
  cM = eM + M;   cW = eW*W;
  for( int i=0; i < Children->Count(); i++ )
  {
    cM = eM + M;   cW = eW*W;
    ((TIsotopeData*)Children->Item(i))->Evail(S, cM, cW);
  }
  if( !Children->Count() )
  {
    TSPoint *P = new TSPoint;
    P->X = cM;
    P->Y = cW;
    S->Add(P);
  }
  else
  {
    eM = cM;  cW = cW;
  }
}
//----------------------------------------------------------------------------//
// TIDistribution
//----------------------------------------------------------------------------//
TIDistribution::TIDistribution()
{
  FPolynomes = new TEList;
  FThreshold = 0.00001; // 0.001%
  FMaxPoints = 25000;
}
//..............................................................................
TIDistribution::~TIDistribution()
{
  for( int i = 0; i < FPolynomes->Count(); i++ )
    delete (TPolynom*)FPolynomes->Item(i);
  delete FPolynomes;
}
//..............................................................................
void TIDistribution::AddIsotope(TBasicAtomInfo *ai, int count)
{
  TPolynom *P = new TPolynom(NULL, PEvaluator, PPolyEvaluator), *P1;
  TPolynomMember *PM;
  TPMember *PPM;
  TEList *Serie, *Serie1;
  for( int i=0; i < ai->IsotopeCount(); i++ )  {
    PM = P->AddMember();
    PPM = PM->AddMember();
    PPM->Id = i;
    PPM->Data = &ai->GetIsotope(i);
  }
//  P1 = P->Pow(count);  // exact calculation
  P1 = P->PowX(20, count);
//  BasicApp->Log->Information(P1->Values());
  P1->SetThreshold(FThreshold);
  Serie = PolynomToSerie(P1);
  delete P1;
  for( int i=30; i < 100; i+=10 )
  {
    P1 = P->PowX(i, count);
    P1->SetThreshold(FThreshold);
    Serie1 = PolynomToSerie(P1);
    if( Serie->Count() == Serie1->Count() )
    {
      DeleteSerie(Serie1);
      break;
    }
    else
    {
      DeleteSerie(Serie);
      delete P1;
      Serie = Serie1;
    }
  }
  DeleteSerie(Serie);
/****************************************************************/
   //  ShowMessage(P1->Values());
/****************************************************************/
  FPolynomes->Add(P1);
  delete P;
}
//..............................................................................
void TIDistribution::Evail(TPolynomMember *PM, double &M, double &W)
{
  TPMember *P;
  TIsotope *Is;
  W *= PM->GetMult();
  TEList *L = PM->Members();
  for( int i=0; i < L->Count(); i++ )
  {
    P = (TPMember*)L->Item(i);
    Is = (TIsotope*)P->Data;
    M += Is->GetMr() * P->Extent;
    for( int j=0; j < P->Extent; j++ )
      W *= Is->GetW();
  }
}
//..............................................................................
void  TIDistribution::CombineSerie(TEList *S, double threshold)
{
  TSPoint *SP, *SP1;
  S->Sort(_SerieSort);
  for( int j=0; j < S->Count(); j++ )
  {
    SP = (TSPoint*)S->Item(j);
    j++;
    if( j == S->Count() ) break;
    SP1 = (TSPoint*)S->Item(j);
    while( fabs(SP->X - SP1->X) < threshold )
    {
      SP->Y += SP1->Y;
      S->Item(j) = NULL;
      delete SP1;
      j++;
      if( j == S->Count() ) break;
      SP1 = (TSPoint*)S->Item(j);
    }
    j--;
  }
  S->Pack();
}
//..............................................................................
void  TIDistribution::DeleteSerie(TEList *L)
{
  for( int k=0; k < L->Count(); k++ )
    delete (TSPoint*)L->Item(k);
  delete L;
}
//..............................................................................
TEList *  TIDistribution::PolynomToSerie(TPolynom *P)
{
  TEList *Serie = new TEList;
  TEList *L;
//  int c;
  TSPoint *SP;
  TPolynomMember *PM;
  L = P->Members();
  for( int j=0; j < L->Count(); j++ )
  {
    PM = (TPolynomMember*)L->Item(j);
    SP = new TSPoint;
    SP->X = 0;
    SP->Y = 1;
    Evail(PM, SP->X, SP->Y);
    Serie->Add(SP);
  }
    CombineSerie(Serie, 0.5);
  return Serie;
}
//..............................................................................
void TIDistribution::Calc(TEList *S)
{
  TIsotopeData *D, *D1;
  TEList *L, *Layer = new TEList;
  TPolynom *P;
//  TPolynomMember *PM;
  Layer->Add(&FRoot);
  int c, i;
  double M, W, EC;
  TEList *Series = new TEList;
  TSPoint *SP;
  while( 1 )
  {
    EC = 1;
    for( i=0; i < FPolynomes->Count(); i++ )
    {
      P = (TPolynom*)FPolynomes->Item(i);
      Series->Add(PolynomToSerie(P));
    }
    for( i=0; i < Series->Count(); i++ )
    {
      L = (TEList*)Series->Item(i);
      EC *= (L->Count());
    }
    if( EC > FMaxPoints )
    {
      if( FThreshold*10 > 0.5 )
        break;
      for( i=0; i < Series->Count(); i++ )
      {
        L = (TEList*)Series->Item(i);
        DeleteSerie(L);
      }
      Series->Clear();
      FThreshold *= 10;
      for( i=0; i < FPolynomes->Count(); i++ )
      {
        P = (TPolynom*)FPolynomes->Item(i);
        if( P->Members()->Count() > 1 )
          P->SetThreshold(FThreshold);
      }
    }
    else
      break;
  }
  Series->Sort(_SeriesSort);
  for( i=0; i < Series->Count(); i++ )
  {
    L = (TEList*)Series->Item(i);
    c = Layer->Count();
    Layer->SetCapacity(c + L->Count());
    for( int j = 0; j < c; j++ )
    {
      D = (TIsotopeData*)Layer->Item(j);
      for( int k=0; k < L->Count(); k++ )
      {
        SP = (TSPoint*)L->Item(k);
        D1 = new TIsotopeData;
        D1->M = SP->X;
        D1->W = SP->Y;
        D->Children->Add(D1);
        Layer->Add(D1);
      }
      Layer->Item(j) = NULL;
    }
    Layer->Pack();
    DeleteSerie(L);
  }
  delete Layer;
  delete Series;
  M = 0;  W = 1;
//    IsToAdd(-1);  // reset static variable 
  FRoot.Evail(S, M, W);
}
//..............................................................................

