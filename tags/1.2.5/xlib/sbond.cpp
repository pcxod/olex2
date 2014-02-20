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

TSBond::TSBond(TNetwork *P) :
  TBasicBond<TNetwork,TSAtom>(P), Order(sboUndefined)
{
  SetType(sotBond);
  Deleted = false;
}
//..............................................................................
void TSBond::OnAtomSet()  {
  if( FA && FB )  {
    if( FA->GetType().z < FB->GetType().z )
      olx_swap(FA, FB);
    else if( FA->GetType().z == FB->GetType().z )  {
      if( FA->GetLabel().Compare(FB->GetLabel()) < 0 )
        olx_swap(FA, FB);
    }
  }
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
