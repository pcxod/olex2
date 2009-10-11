#ifndef __olx_conn_ext_H
#define __olx_conn_ext_H
#include "catom.h"

BeginXlibNamespace()

const short
  def_max_bonds = 12;

struct CXBondInfo  {
  TCAtom& to;
  const smatd* matr;
  CXBondInfo(const CXBondInfo& bi) : to(bi.to), matr(bi.matr) {}
  CXBondInfo(TCAtom& ca, const smatd* m=NULL) : to(ca), matr(m) {}
};
typedef TTypeList<CXBondInfo> BondInfoList;

struct CXConnInfoBase  {
  short maxBonds;
  double r;
  CXConnInfoBase() : maxBonds(def_max_bonds), r(-1) {}
  CXConnInfoBase(const CXConnInfoBase& ci) : maxBonds(ci.maxBonds), r(ci.r) {}
  CXConnInfoBase& operator = (const CXConnInfoBase& ci)  {
    maxBonds = ci.maxBonds;
    r = ci.r;
    return *this;
  }
};

struct CXConnInfo : public CXConnInfoBase {
  BondInfoList BondsToCreate, 
    BondsToRemove;
};


EndXlibNamespace()
#endif
