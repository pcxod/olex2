/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_sbond_H
#define __olx_xl_sbond_H
#include "satom.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"

BeginXlibNamespace()

const short
  sboUndefined = 0,
  sboSingle = 1,
  sboDouble = 2,
  sboTripple = 3,
  sboAromatic = 4;


class TSBond : public TBasicBond<class TNetwork, TSAtom> {
private:
  virtual void OnAtomSet();
protected:
  bool Deleted;
  short Order;
public:
  TSBond(TNetwork* Parent);
  virtual ~TSBond() {}

  DefPropBIsSet(Deleted);
  bool IsAvailable() const {
    return (!IsDeleted() && FA->IsAvailable() && FB->IsAvailable());
  }
  vec3d GetCenter() const { return (FA->crd() + FB->crd()) / 2; }
  double Length() const { return FA->crd().DistanceTo(FB->crd()); }
  double QLength() const { return FA->crd().QDistanceTo(FB->crd()); }

  struct Ref : public comparable_operators_full<Ref> {
    TSAtom::Ref a, b;
    Ref() {}
    Ref(const TSAtom::Ref& _a, const TSAtom::Ref& _b)
      : a(_a), b(_b)
    {}
    Ref(const Ref& r) : a(r.a), b(r.b) {}
    Ref(const TDataItem& di, const class TXApp& app)
    : a(di.GetItemByName('a'), app),
      b(di.GetItemByName('b'), app)
    {}
    Ref& operator = (const Ref& r) {
      a = r.a;
      b = r.b;
      return *this;
    }

    int Compare(const Ref& r) const;

    void ToDataItem(TDataItem& item, const class TXApp& app, bool use_id = false) const;
    bool IsValid(const class TXApp& app) const;
    void FromDataItem(const TDataItem& item, const TXApp& app);
  private:
    int cmp(const TSAtom::Ref& a1, const TSAtom::Ref& b1,
      const TSAtom::Ref& a2, const TSAtom::Ref& b2) const
    {
      int r = a1.Compare(b1);
      if (r == 0) {
        r = a2.Compare(b2);
      }
      return r;
    }
  };

  Ref GetRef() const { return Ref(FA->GetRef(), FB->GetRef()); }
  // despite the fact that atoms are alsways sorted, compare both end...
  bool operator == (const Ref& r) const {
    return (*FA == r.a && *FB == r.b) || (*FA == r.b && *FB == r.a);
  }
  static Ref GetRef(const TSAtom &a, const TSAtom &b);
  static Ref GetRef(const TCAtom& a, const TCAtom& b) {
    return GetRef(a, 0, b, 0);
  }
  static Ref GetRef(const TCAtom& a, const smatd* ma, const TCAtom& b, const smatd* mb);
  /* returns MOL file compatible bond order:
  1 = Single, 2 = Double, 3 = Triple, 4 = Aromatic, 0 - undefined
  The first element argument is the heavier one
  */
  DefPropP(short, Order);
  static short PercieveOrder(const cm_Element& a,
      const cm_Element& b, double d);
  virtual void ToDataItem(TDataItem& item) const;
  virtual void FromDataItem(const TDataItem& item, TLattice& parent);
};

typedef TTypeList<TSBond> TSBondList;
typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

