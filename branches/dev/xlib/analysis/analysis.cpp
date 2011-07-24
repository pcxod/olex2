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
  return EDMapAnalysis().Analyse(latt);
}
//.............................................................................
ConstTypeList<Result> analysis::AnalyseGeometry(const TLattice &latt)  {
  TTypeList<Result> res;
  return res;
}
//.............................................................................
