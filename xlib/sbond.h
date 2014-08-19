/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
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
  sboTripple = 3;


class TSBond: public TBasicBond<class TNetwork, TSAtom>  {
private:
  virtual void OnAtomSet();
protected:
  bool Deleted;
  short Order;
public:
  TSBond(TNetwork* Parent);
  virtual ~TSBond() {}

  DefPropBIsSet(Deleted)
  bool IsAvailable() const {  return (!IsDeleted() && FA->IsAvailable() && FB->IsAvailable()); }
  vec3d GetCenter() const {  return (FA->crd()+FB->crd())/2;  }
  double Length() const {  return FA->crd().DistanceTo(FB->crd());  }
  double QLength() const {  return FA->crd().QDistanceTo(FB->crd());  }

  struct Ref  {
    TSAtom::Ref a, b;
    Ref(const TSAtom::Ref& _a, const TSAtom::Ref& _b)
      : a(_a.catom_id < _b.catom_id ? _a : _b),
        b(_a.catom_id < _b.catom_id ? _b : _a)
    {}
    Ref(const Ref& r) : a(r.a), b(r.b)  {}
    Ref(const TDataItem &di) { FromDataItem(di); }
    Ref& operator = (const Ref& r)  {
      a = r.a;
      b = r.b;
      return *this;
    }
    bool operator == (const TSBond::Ref& r) const {
      //sorted in the constructor, no swapping neeeded
      return (a == r.a && b == r.b);
    }
    void ToDataItem(TDataItem& item) const {
      a.ToDataItem(item.AddItem("a"));
      b.ToDataItem(item.AddItem("b"));
    }
    void FromDataItem(const TDataItem& item)  {
      a.FromDataItem(item.GetItemByName('a'));
      b.FromDataItem(item.GetItemByName('b'));
    }

    int Compare(const Ref& r) const {
      const int rv = a.Compare(r.a);
      return rv == 0 ? b.Compare(r.b) : rv;
    }

    olxstr ToString() const {
      return a.ToString() << b.ToString();
    }
  };

  Ref GetRef() const {  return Ref(FA->GetRef(), FB->GetRef());  }
  // despite the fact that atoms are alsways sorted, compare both end...
  bool operator == (const Ref& r) const {
    return (*FA == r.a && *FB == r.b) || (*FA == r.b && *FB == r.a);
  }
  DefPropP(short, Order)
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item, class TLattice& parent);
};

typedef TTypeList<TSBond> TSBondList;
typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

