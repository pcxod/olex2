/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "analysis.h"
#include "ed_analysis.h"
#include "auto.h"

using namespace analysis;

ConstTypeList<Result> analysis::AnalyseADP(const TLattice &latt)  {
  TTypeList<Result> res;
  return res;
}
//.............................................................................
ConstTypeList<Result> analysis::AnalyseEDMap(TLattice &latt)  {
  EDMapAnalysis ed_analysis;
  return ed_analysis.Analyse(latt);
}
//.............................................................................
ConstTypeList<Result> analysis::AnalyseGeometry(TLattice &latt)  {
  if( TAutoDB::GetInstance() == NULL )  {
    TXApp &app = TXApp::GetInstance();
    olxstr autodbf(app.GetBaseDir() + "acidb.db");
    TEGC::AddP(new TAutoDB(*((TXFile*)app.XFile().Replicate()), app));
    if( TEFile::Exists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream(dbf);
    }
  }
  EDMapAnalysis ed_analysis;
  ConstTypeList<TAutoDB::TAnalysisResult> ares =
    TAutoDB::GetInstance()->AnalyseStructure(latt);
  TTypeList<Result> res;
  for( size_t i=0; i < ares.Count(); i++ )  {
    TTypeList<TAutoDB::Type> hits;
    if( !ares[i].list3.IsEmpty() )
      hits = ares[i].list3;
    else if( !ares[i].list2.IsEmpty() )
      hits = ares[i].list2;
    else  {
      hits = ares[i].list1;
      hits.AddListC(ares[i].enforced);
    }
    const cm_Element *type = NULL;
    if( hits.Count() > 1 )  {
      if( ares[i].atom->GetType() == iQPeakZ )
        type = &hits[0].type;
      else  {
        ElementPList elements;
        for( size_t j=0; j < hits.Count(); j++ )
          elements << hits[j].type;
        elements << ares[i].atom->GetType();
        type = ed_analysis.TryAtomType(latt, *ares[i].atom, elements)[0].GetA();
      }
    }
    else if( hits.Count() == 1 )
      type = &hits[0].type;
    if( type == NULL || type == &ares[i].atom->GetType() )
      continue;
    res.Add(new Result(*ares[i].atom, *type));
  }
  return res;
}
//.............................................................................
