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
    if( FA->GetAtomInfo().GetMr() < FB->GetAtomInfo().GetMr() )  {
      TSAtom *Tmp = FB;
      FB = FA;
      FA = Tmp;
      return;
    }
    if( FA->GetAtomInfo().GetMr() == FB->GetAtomInfo().GetMr() )  {
      if( FA->GetLabel().Compare( FB->GetLabel() ) < 0 )  {
        TSAtom *Tmp = FB;
        FB = FA;
        FA = Tmp;
        return;
      }
    }
  }
}
//..............................................................................
double TSBond::Length() const {  return FA->crd().DistanceTo(FB->crd()); }
//..............................................................................
void TSBond::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network->GetTag());
  item.AddField("a_id", FA->GetTag());
  item.AddField("b_id", FB->GetTag());
}
//..............................................................................
void TSBond::FromDataItem(const TDataItem& item, TPtrList<TNetwork>& net_pool) {
  //if( item.FieldCount() != 2 )
  //  throw TInvalidArgumentException(__OlxSourceInfo, "data item");
  Network = net_pool[item.GetField(0).ToInt()];
  FA = &Network->GetLattice().GetAtom( item.GetField(1).ToInt() );
  FB = &Network->GetLattice().GetAtom( item.GetField(2).ToInt() );
}
//..............................................................................
