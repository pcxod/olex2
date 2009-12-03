//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TSBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "sbond.h"
#include "lattice.h"

//----------------------------------------------------------------------------//
// TSBond function bodies
//----------------------------------------------------------------------------//
TSBond::TSBond(TNetwork *P) : TBasicBond<TNetwork,TSAtom>(P)  {
  SetType(sotBond);
  Deleted = false;
}
//..............................................................................
void TSBond::OnAtomSet()  {
  if( FA && FB )  {
    if( FA->GetAtomInfo().GetMr() < FB->GetAtomInfo().GetMr() )
      olx_swap(FA, FB);
    else if( FA->GetAtomInfo().GetMr() == FB->GetAtomInfo().GetMr() )  {
      if( FA->GetLabel().Compare( FB->GetLabel() ) < 0 )
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
void TSBond::FromDataItem(const TDataItem& item, TPtrList<TNetwork>& net_pool) {
  Network = net_pool[item.GetRequiredField("net_id").ToInt()];
  FA = &Network->GetLattice().GetAtom( item.GetRequiredField("a_id").ToInt() );
  FB = &Network->GetLattice().GetAtom( item.GetRequiredField("b_id").ToInt() );
  Type = item.GetRequiredField("type").ToInt();
}
//..............................................................................
