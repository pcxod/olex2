//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TSBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "sbond.h"
#include "satom.h"

//----------------------------------------------------------------------------//
// TSBond function bodies
//----------------------------------------------------------------------------//
TSBond::TSBond(TNetwork *P):TBasicBond(P)  {
  SetType(sotBond);
  Deleted = false;
}
//..............................................................................
void TSBond::Create(){  return; }
//..............................................................................
TSBond::~TSBond()
{
  ;
}
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
double TSBond::Length(){  return (double)FA->Center().DistanceTo(FB->Center()); }
//..............................................................................

