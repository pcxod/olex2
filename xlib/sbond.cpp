/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "sbond.h"
#include "lattice.h"
#include "xapp.h"

TSBond::TSBond(TNetwork *P) :
  TBasicBond<TNetwork,TSAtom>(P), Order(sboUndefined)
{
  SetType(sotBond);
  Deleted = false;
}
//..............................................................................
void TSBond::OnAtomSet() {
  if (FA != 0 && FB != 0) {
    if (FA->GetType().z < FB->GetType().z) {
      olx_swap(FA, FB);
    }
    else if (FA->GetType().z == FB->GetType().z) {
      if (FA->GetLabel().Compare(FB->GetLabel()) < 0) {
        olx_swap(FA, FB);
      }
    }
  }
}
//..............................................................................
TSBond::Ref TSBond::GetRef(const TSAtom& a_, const TSAtom& b_) {
  const TSAtom* a = &a_, * b = &b_;
  if (a->GetType().z < b->GetType().z) {
    olx_swap(a, b);
  }
  else if (a->GetType().z == b->GetType().z) {
    if (a->GetLabel().Compare(b->GetLabel()) < 0) {
      olx_swap(a, b);
    }
  }
  return Ref(a->GetRef(), b->GetRef());
}
//..............................................................................
TSBond::Ref TSBond::GetRef(const TCAtom& a_, const smatd* ma_,
  const TCAtom& b_, const smatd* mb_)
{
  const TCAtom* a = &a_, * b = &b_;
  const smatd* ma = ma_, * mb = mb_;
  if (a->GetType().z < b->GetType().z) {
    olx_swap(a, b);
    olx_swap(ma, mb);
  }
  else if (a->GetType().z == b->GetType().z) {
    if (a->GetLabel().Compare(b->GetLabel()) < 0) {
      olx_swap(a, b);
      olx_swap(ma, mb);
    }
  }
  return Ref(TSAtom::Ref(*a, ma == 0 ? ~0 : ma->GetId()),
    TSAtom::Ref(*b, mb == 0 ? ~0 : mb->GetId()));
}
//..............................................................................
void TSBond::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network->GetTag());
  item.AddField("a_id", FA->GetTag());
  item.AddField("b_id", FB->GetTag());
  item.AddField("type", GetType());
  item.AddField("order", GetOrder());
}
//..............................................................................
void TSBond::FromDataItem(const TDataItem& item, TLattice& parent) {
  Network = &parent.GetFragment(item.GetFieldByName("net_id").ToInt());
  FA = &parent.GetObjects().atoms[item.GetFieldByName("a_id").ToInt()];
  FB = &parent.GetObjects().atoms[item.GetFieldByName("b_id").ToInt()];
  Type = item.GetFieldByName("type").ToInt();
  SetOrder(item.FindField("order", '0').ToInt());
}
//..............................................................................
short TSBond::PercieveOrder(const cm_Element &a,
  const cm_Element &b, double l)
{
  if (a == iOxygenZ || b == iOxygenZ) {
    if (a == iSulphurZ) {
      if (l < 1.48) return 2;
    }
    if (b == iNitrogenZ) {
      if (l < 1.10) return 3;
      if (l < 1.22) return 2;
    }
    if (b == iCarbonZ) {
      if (l < 1.15) return 3;
      if (l < 1.30) return 2;
    }
  }
  else if (a == iNitrogenZ || b == iNitrogenZ) {
    if (a == iNitrogenZ && b == iNitrogenZ) {
      if (l < 1.15) return 3;
      if (l < 1.22) return 2;
      if (l < 1.41) return 4;
    }
    else if (b == iCarbonZ) {
      if (l < 1.20) return 3;
      if (l < 1.30) return 2;
      if (l < 1.43) return 4;
    }
  }
  else if (a == iCarbonZ || b == iCarbonZ) {
    if (a == iCarbonZ && b == iCarbonZ) {
      if (l < 1.20) return 3;
      if (l < 1.36) return 2;
      if (l < 1.45) return 4;
    }
  }
  return 0;
}
//..............................................................................
int TSBond::Ref::Compare(const Ref& r) const {
  if (a <= b) {
    if (r.a <= r.b) {
      return cmp(a, r.a, b, r.b);
    }
    else {
      return cmp(a, r.b, b, r.a);
    }
  }
  else {
    if (r.a <= r.b) {
      return cmp(b, r.a, a, r.b);
    }
    else {
      return cmp(b, r.b, a, r.a);
    }
  }
  const int rv = a.Compare(r.a);
  return rv == 0 ? b.Compare(r.b) : rv;
}
//..............................................................................
void TSBond::Ref::ToDataItem(TDataItem& item, const TXApp& app, bool use_id) const {
  a.ToDataItem(item.AddItem("a"), app, use_id);
  b.ToDataItem(item.AddItem("b"), app, use_id);
}
//..............................................................................
bool TSBond::Ref::IsValid(const TXApp& app) const {
  return a.IsValid(app) && b.IsValid(app);
}
//..............................................................................
void TSBond::Ref::FromDataItem(const TDataItem& item, const class TXApp& app) {
  a.FromDataItem(item.GetItemByName('a'), app);
  b.FromDataItem(item.GetItemByName('b'), app);
}
//..............................................................................
