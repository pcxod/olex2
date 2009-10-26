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
#include "emath.h"

//..............................................................................
bool PAddEvaluator(const TPolynomMember& PM)  {
  static double MaxC = 0;
  //if( !PM )  {
  //  MaxC = 0;
  //  return false; // does not matter
  //}
  double W = 1;
  for( size_t i=0; i < PM.Members().Count(); i++ )  {
    const TPMember& P = PM.Members()[i];
    TIsotope* Is = (TIsotope*)P.Data;
    for( int j=0; j < P.Extent; j++ )
      W *= Is->GetW();
  }
  W *= PM.GetMult();
  if( W > MaxC )  {
    MaxC = W;
    return true;
  }
  if( MaxC != 0 )  {
    if( W/MaxC < 0.001 )
      return false;
    else
      return true;
  }
  return true;
}
//..............................................................................
double PEvaluator(const TPolynomMember& PM)  {
  double W = 1;
  for( size_t i=0; i < PM.Members().Count(); i++ ) {
    const TPMember& P = PM.Members()[i];
    TIsotope* Is = (TIsotope*)P.Data;
    for( int j=0; j < P.Extent; j++ )
      W *= Is->GetW();
  }
  W *= PM.GetMult();
  return W;
}
//..............................................................................
int PPolyEvaluator(const TPolynomMember& P, const TPolynomMember& P1) {
  double W = P.GetMult(),
         W1=P1.GetMult();
  const TTypeList<TPMember>& L = P.Members();
  for( size_t i=0; i < L.Count(); i++ )  {
    TIsotope* Is = (TIsotope*)L[i].Data;
    for( int j=0; j < L[i].Extent; j++ )
      W *= Is->GetW();
  }
  const TTypeList<TPMember>& L1 = P1.Members();
  for( size_t i=0; i < L1.Count(); i++ )  {
    TIsotope* Is = (TIsotope*)L1[i].Data;
    for( int j=0; j < L1[i].Extent; j++ )
      W1 *= Is->GetW();
  }
  if( W < W1 )  return 1;
  if( W > W1 )  return -1;
  return 0;
}
//..............................................................................
int _SeriesSort(const TPolySerie& I, const TPolySerie& I1) {
  return olx_cmp_size_t(I.Count(), I1.Count());
}
//..............................................................................
int  _SerieSort(const TSPoint& I, const TSPoint& I1)  {
  if( I.X < I1.X )  return 1;
  if( I.X > I1.X )  return -1;
  return 0;
}
//----------------------------------------------------------------------------//
// TIsotopeData 
//----------------------------------------------------------------------------//
void TIsotopeData::Evail(TPolySerie& S, double &eM, double &eW) {
  double cM = eM + M, 
         cW = eW*W;
  for( size_t i=0; i < Children.Count(); i++ )  {
    cM = eM + M;   cW = eW*W;
    Children[i].Evail(S, cM, cW);
  }
  if( Children.IsEmpty() )
    S.AddNew(cM, cW);
  else  {
    eM = cM;  
    cW = cW;
  }
}
//----------------------------------------------------------------------------//
// TIDistribution
//----------------------------------------------------------------------------//
TIDistribution::TIDistribution()  {
  Threshold = 0.00001; // 0.001%
  MaxPoints = 25000;
}
//..............................................................................
void TIDistribution::AddIsotope(TBasicAtomInfo& ai, size_t count)  {
  TPolynom *P = new TPolynom(NULL, PEvaluator, PPolyEvaluator);
  for( size_t i=0; i < ai.IsotopeCount(); i++ )  {
    TPMember& PPM = P->AddMember().AddMember();
    PPM.Id = i;
    PPM.Data = &ai.GetIsotope(i);
  }
//  P1 = P->Pow(count);  // exact calculation
  TPolynom* P1 = P->PowX(20, count);
//  BasicApp->Log->Information(P1->Values());
  P1->SetThreshold(Threshold);
  TPolySerie* Serie = PolynomToSerie(*P1);
  delete P1;
  for( int i=30; i < 100; i+=10 )  {
    P1 = P->PowX(i, count);
    P1->SetThreshold(Threshold);
    TPolySerie* Serie1 = PolynomToSerie(*P1);
    if( Serie->Count() == Serie1->Count() )  {
      delete Serie1;
      break;
    }
    else  {
      delete Serie;
      delete P1;
      Serie = Serie1;
    }
  }
  delete Serie;
/****************************************************************/
   //  ShowMessage(P1->Values());
/****************************************************************/
  Polynomes.Add(P1);
  delete P;
}
//..............................................................................
void TIDistribution::Evail(const TPolynomMember& PM, double& M, double& W) const {
  W *= PM.GetMult();
  for( size_t i=0; i < PM.Members().Count(); i++ )  {
    TIsotope* Is = (TIsotope*)PM.Members()[i].Data;
    M += Is->GetMr() * PM.Members()[i].Extent;
    for( int j=0; j < PM.Members()[i].Extent; j++ )
      W *= Is->GetW();
  }
}
//..............................................................................
void  TIDistribution::CombineSerie(TPolySerie& S, double threshold) {
  S.QuickSorter.SortSF(S, _SerieSort);
  for( size_t j=0; j < S.Count(); j++ )  {
    TSPoint& SP = S[j++];
    if( j == S.Count() ) break;
    TSPoint& SP1 = S[j];
    while( olx_abs(SP.X - SP1.X) < threshold )  {
      SP.Y += SP1.Y;
      S.NullItem(j);
      j++;
      if( j == S.Count() ) break;
    }
    j--;
  }
  S.Pack();
}
//..............................................................................
TPolySerie* TIDistribution::PolynomToSerie(const TPolynom& P) {
  TPolySerie* Serie = new TPolySerie;
  for( size_t j=0; j < P.Members().Count(); j++ )  {
    const TPolynomMember& PM = P.Members()[j];
    TSPoint& SP = Serie->AddNew(0,1);
    Evail(PM, SP.X, SP.Y);
  }
  CombineSerie(*Serie, 0.5);
  return Serie;
}
//..............................................................................
void TIDistribution::Calc(TPolySerie& S)  {
  TPtrList<TIsotopeData> Layer;
  Layer.Add(Root);
  TTypeList<TPolySerie> Series;
  while( true )  {
    double EC = 1;
    for( size_t i=0; i < Polynomes.Count(); i++ )
      Series.Add(PolynomToSerie(Polynomes[i]));
    for( size_t i=0; i < Series.Count(); i++ )
      EC *= Series[i].Count();

    if( EC > MaxPoints )  {
      if( Threshold*10 > 0.5 )  break;
      Series.Clear();
      Threshold *= 10;
      for( size_t i=0; i < Polynomes.Count(); i++ )  {
        if( Polynomes[i].Members().Count() > 1 )
          Polynomes[i].SetThreshold(Threshold);
      }
    }
    else
      break;
  }
  Series.QuickSorter.SortSF(Series, _SeriesSort);
  for( size_t i=0; i < Series.Count(); i++ )  {
    TPolySerie& sr = Series[i];
    size_t c = Layer.Count();
    Layer.SetCapacity(c + sr.Count());
    for( size_t j = 0; j < c; j++ )  {
      for( size_t k=0; k < sr.Count(); k++ )  {
        TIsotopeData* D1 = Layer.Add( new TIsotopeData);
        D1->M = sr[k].X;
        D1->W = sr[k].Y;
        Layer[j]->Children.Add(D1);
      }
      Layer[j] = NULL;
    }
    Layer.Pack();
  }
  double M = 0, W = 1;
  Root.Evail(S, M, W);
}
//..............................................................................

