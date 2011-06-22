/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_cl_chnexp_H
#define __olx_cl_chnexp_H
#include "chembase.h"
#include "estrlist.h"
#include "edict.h"

BeginChemNamespace()

class TCHNExp: public IEObject  {
  TStrPObjList<olxstr,double> Exp; // Objects - double for content
  TTypeList<TCHNExp> Dependencies; // TCHNExp list of dependencies
  double FMult;
protected:
  void Clear();
public:
  TCHNExp() : FMult(1) { }
  virtual ~TCHNExp() {  Clear();  }
  // return summ formula of the compound with elements seprated by the Separator
  olxstr SummFormula(const olxstr& Separator);
  double MolWeight();
  olxstr Composition();
  // calculates C, H and N and mol weight contents of  compound
  // to get %: %(c) = C*100/Mr
  void CHN(double& C, double& H, double& N, double& Mr) const;
  // expects a distrionary with keys of TBasicAtomInfo::GetIndex()
  double CHN(olxdict<short, double, TPrimitiveComparator>& rv) const;
  void LoadFromExpression(const olxstr &E);
  void CalcSummFormula(TStrPObjList<olxstr,double>& Exp) const;
  void SetMult(const olxstr& S)  {  FMult = S.ToDouble();  }
  void SetMult(double v)  {  FMult = v;  }
  inline double GetMult() const {  return FMult;  }
};

EndChemNamespace()
#endif
