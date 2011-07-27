/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_analysis_H
#define __olx_xlib_analysis_H
#include "../lattice.h"
#include "typelist.h"
BeginXlibNamespace()

namespace analysis {
  struct Result {
    TCAtom &atom;
    const cm_Element &proposed;
    Result(TCAtom &_atom, const cm_Element &_proposed)
      : atom(_atom), proposed(_proposed)  {}
  };
  ConstTypeList<Result> AnalyseADP(const TLattice &latt);
  ConstTypeList<Result> AnalyseEDMap(TLattice &latt);
  ConstTypeList<Result> AnalyseGeometry(TLattice &latt);
};
EndXlibNamespace()
#endif
