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

using namespace analysis;

ConstTypeList<Result> analysis::AnalyseADP(const TLattice &latt)  {
  TTypeList<Result> res;
  return res;
}
//.............................................................................
ConstTypeList<Result> analysis::AnalyseEDMap(TLattice &latt)  {
  EDMapAnalysis ed_analysis;
  ElementPList elements;
  elements.Add(latt.GetObjects().atoms[0].GetType());
  elements.Add(XElementLib::PrevZ(latt.GetObjects().atoms[0].GetType()));
  elements.Add(XElementLib::NextZ(latt.GetObjects().atoms[0].GetType()));
  TTypeList<AnAssociation2<const cm_Element*, double> > res =
    ed_analysis.TryAtomType(latt, latt.GetObjects().atoms[0].CAtom(), elements);
  for( size_t i=0; i < res.Count(); i++ )
    TBasicApp::NewLogEntry() << res[i].GetA()->symbol << ": " << res[i].GetB();
  return ed_analysis.Analyse(latt);
}
//.............................................................................
ConstTypeList<Result> analysis::AnalyseGeometry(const TLattice &latt)  {
  TTypeList<Result> res;
  return res;
}
//.............................................................................
