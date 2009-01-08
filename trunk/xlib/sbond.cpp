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
TSBond::TSBond(TNetwork *P) : TBasicBond(P)  {
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
double TSBond::Length()  {  return FA->crd().DistanceTo(FB->crd()); }
//..............................................................................
void TSBond::ToDataItem(TDataItem& item) const {
  item.AddCodedField("net_id", Network->GetTag());
  item.AddCodedField("a_id", FA->GetTag());
  item.AddCodedField("b_id", FB->GetTag());
}
//..............................................................................
void TSBond::FromDataItem(const TDataItem& item, TPtrList<TNetwork>& net_pool) {
  //if( item.FieldCount() != 2 )
  //  throw TInvalidArgumentException(__OlxSourceInfo, "data item");
  Network = net_pool[item.RawField(0).ToInt()];
  FA = &Network->GetLattice().GetAtom( item.RawField(1).ToInt() );
  FB = &Network->GetLattice().GetAtom( item.RawField(2).ToInt() );
}
//..............................................................................
