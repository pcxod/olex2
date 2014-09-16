/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ipattern.h"
#include "chemdata.h"
#include "chnexp.h"
#include "estrlist.h"
#include "idistribution.h"
#include "bapp.h"
#include "log.h"

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
  for( size_t i=0; i < SL.Count(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbol(SL[i]);
    if( elm == NULL )
      return false;
    int occupancy = (int)SL.GetObject(i);
    if( occupancy == 0 )  {
      occupancy++;
      TBasicApp::NewLogEntry(logError) << "The occupancy is set to 1 for " << elm->symbol <<
        " the molecular weight might be incorrect";
    }
    ID.AddIsotope(*elm, occupancy);
  }
  ID.Calc(Points);
  if( !Points.IsEmpty() )  {
    if( Combine )
      TIDistribution::CombineSerie(Points, Delta);
    QuickSorter::SortSF(Points, &TSPoint::SPointsSortA);
    // normalisation of the serie
    double MaxY = 100.0/Points[0].Y; // normalisation factor
    for( size_t i=0; i < Points.Count(); i++ )
      Points[i].Y *= MaxY; 
  }
  return true;
}
//..............................................................................
void TIPattern::SortDataByMolWeight() {
  QuickSorter::SortSF(Points, &TSPoint::SPointsSortB);
}
//..............................................................................
void TIPattern::SortDataByItensity() {
  QuickSorter::SortSF(Points, &TSPoint::SPointsSortA);
}
//..............................................................................
