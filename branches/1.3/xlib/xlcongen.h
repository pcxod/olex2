/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xlcongen_H
#define __olx_xl_xlcongen_H
#include "congen.h"
#include "ins.h"
BeginXlibNamespace()

class TXlConGen : public AConstraintGenerator {
public:
  TXlConGen(RefinementModel& rm)
    : AConstraintGenerator(rm)
  {}
  virtual bool FixParam(const short paramMask, TStrList& res,
    const TCAtomPList& atoms, const TFixedValueList& values);
  virtual bool FixAtom(TAtomEnvi& envi, const short Group,
    const cm_Element& atomType,
    TAtomEnvi* pivoting = 0, TCAtomPList* generated = 0);
  virtual void AnalyseMultipart(const TAtomEnvi& envi,
    const TTypeList<TCAtomPList>& parts);
  // translates shelxl AFIX, HFIX to olex2 notation
  static int OlexToShelx(short code, const TAtomEnvi& envi,
    TAtomEnvi* pivot = 0);
  static short ShelxToOlex(int shelx_code, const TAtomEnvi& envi);
  /* guesses AFIX code for enviroment where peaks are potentially H atoms.
  Returns 0 if the environment cannot be interpreted
  */
  static int AfixFromQEnvironment(const TSAtom &a);
  /* Normalises some environments like for O-H and H2O fragments
  */
  static bool NormaliseEnvironment(TSAtom &a);
};

EndXlibNamespace()
#endif
