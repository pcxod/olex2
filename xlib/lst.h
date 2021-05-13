/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_lst_H
#define __olx_xl_lst_H
#include "xbase.h"
#include "evalue.h"
#include "symmat.h"
#include "bitarray.h"
#include "estrlist.h"
#include "edict.h"
BeginXlibNamespace()

const short slstReflections = 1,
            slslRefineData  = 2;
struct TLstRef  {
  short H, K, L;
  double DF, Res, Fo, Fc;
  bool Deleted;
};

struct TLstSplitAtom  {
  olxstr AtomName;
  vec3d PositionA, PositionB;
};

struct TTrefTry  {
  double CFOM, RAlpha, NQual, SigmaM1, Mabs;
  long Try;
  TEBitArray Semivariants;
};

class TPattAtom {
  olxstr Name;
  vec3d Crd;
  double Occup;
public:
  TPattAtom()  {  Occup = 0;  }
  TPattAtom(const TPattAtom& A)  {
    Name = A.Name;
    Crd = A.Crd;
    Occup = A.Occup;
  }
  TPattAtom& operator = (const TPattAtom& A)  {
    Name = A.Name;
    Crd = A.Crd;
    Occup = A.Occup;
    return *this;
  }

  const olxstr& GetName() const {  return Name;  }
  vec3d& GetCrd()              {  return Crd;   }
  double GetOccup() const       {  return Occup; }

  void SetName(const olxstr& n)  { Name = n;   }
  void SetOccup(double v)          { Occup = v;  }

};

class TLst: public IOlxObject  {
protected:
  TTypeList<TLstRef> FDRefs;
  TTypeList<TTrefTry> TrefTries;
  TTypeList< TTypeList<TPattAtom> > PattSolutions;
  TTypeList<TLstSplitAtom> FSplitAtoms;
  TTypeList< olx_pair_t<olxstr, olxstr> > ErrorMsgs;
  bool Loaded;
public:
  TLst() : Loaded(false) {}
  void Clear();
  ~TLst() {}
  bool LoadFromFile(const olxstr &FN);
  void SynchroniseOmits(class RefinementModel& rm);
  bool ExportHTML( const short Param, TStrList &Html, bool TableDef=true);
  size_t DRefCount() const {  return FDRefs.Count(); }
  const TLstRef& DRef(size_t i) const {  return FDRefs[i]; }
  void DelRef(size_t i)  {  FDRefs.Delete(i);  }
  size_t SplitAtomCount() const {  return FSplitAtoms.Count();  }
  const TLstSplitAtom& SplitAtom(size_t i) const {  return FSplitAtoms[i]; }

  size_t TrefTryCount() const {  return TrefTries.Count(); }
  const TTrefTry& TrefTry(size_t i) const {  return TrefTries[i]; }

  size_t PattSolutionCount() const {  return PattSolutions.Count(); }
  const TTypeList<TPattAtom>& PattSolution(size_t i) const {  return PattSolutions[i];  }

  bool IsLoaded() const {  return Loaded;  }
  size_t ErrMsgCount() const {  return ErrorMsgs.Count();  }
  const olxstr& GetError(size_t i) const {  return ErrorMsgs[i].GetA();  }
  const olxstr& GetCause(size_t i) const {  return ErrorMsgs[i].GetB();  }

  olxstr_dict<olxstr, true> params;
  /* max_shift, mean_shift, F000, Mu, Rho, flack, flack_type (Q, x), wR2, S,
  rS, R1, R1all, param_n, ref_4sig, hole, peak, Rint, Rsig, ref_total,
  ref_unique
   */
};

EndXlibNamespace()
#endif
