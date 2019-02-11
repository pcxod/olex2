/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_conn_ext_H
#define __olx_conn_ext_H
#include "catom.h"

BeginXlibNamespace()

const short
  def_max_bonds = 12;

struct CXBondInfo {
  TCAtom& to;
  const smatd* matr;
  CXBondInfo(const CXBondInfo& bi)
    : to(bi.to), matr(bi.matr)
  {}
  CXBondInfo(TCAtom& ca, const smatd* m = 0)
    : to(ca), matr(m)
  {}

  olxstr ToString(const TCAtom & from) const {
    olxstr_buf rv;
    // to is in a residue
    if (to.GetResiId() != 0) {
      rv << '_' << to.GetResiId() <<
        ' ' << from.GetResiLabel() <<
        ' ' << to.GetLabel();
    }
    // from is in a residue
    else if (from.GetResiId() != 0) {
      rv << '_' << from.GetResiId() << ' ' <<
        from.GetLabel() << ' ' << to.GetLabel();
    }
    // both in main residue
    else {
      rv << from.GetLabel() << ' ' << to.GetLabel();
    }
    return olxstr(rv);
  }
};
typedef TTypeList<CXBondInfo> BondInfoList;

struct CXConnInfoBase {
  short maxBonds;
  double r;
  CXConnInfoBase()
    : maxBonds(def_max_bonds), r(-1)
  {}
  CXConnInfoBase(const CXConnInfoBase& ci)
    : maxBonds(ci.maxBonds), r(ci.r)
  {}
  CXConnInfoBase& operator = (const CXConnInfoBase& ci) {
    maxBonds = ci.maxBonds;
    r = ci.r;
    return *this;
  }
};

struct CXConnInfo : public CXConnInfoBase {
  BondInfoList BondsToCreate, BondsToRemove;
};

EndXlibNamespace()
#endif
