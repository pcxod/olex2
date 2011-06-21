#include "sbond.h"
#include "lattice.h"

TSBond::TSBond(TNetwork *P) : TBasicBond<TNetwork,TSAtom>(P)  {
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
}
//..............................................................................
void TSBond::FromDataItem(const TDataItem& item, TLattice& parent) {
  Network = &parent.GetFragment(item.GetRequiredField("net_id").ToInt());
  FA = &parent.GetObjects().atoms[item.GetRequiredField("a_id").ToInt()];
  FB = &parent.GetObjects().atoms[item.GetRequiredField("b_id").ToInt()];
  Type = item.GetRequiredField("type").ToInt();
}
//..............................................................................
