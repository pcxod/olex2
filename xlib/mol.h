/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_mol_H
#define __olx_xl_mol_H
#include "xfiles.h"

BeginXlibNamespace()

struct TMolBond  {
  size_t AtomA, AtomB, BondType;
};

class TMol: public TBasicCFile {
private:
  void Clear();
  TTypeList<TMolBond> Bonds;
protected:
  olxstr MOLAtom(TCAtom& CA);
  olxstr MOLBond(TMolBond& B);
  static const olxstr &padding() {
    static const olxstr p= "  0";
    return p;
  }
public:
  TMol();
  virtual ~TMol();

  inline size_t BondCount() const {  return Bonds.Count();  }
  inline TMolBond& Bond(size_t index) {  return Bonds[index];  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile &, int);
  virtual IOlxObject* Replicate()  const {  return new TMol;  }
};

EndXlibNamespace()
#endif
