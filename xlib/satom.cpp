#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "satom.h"
#include "network.h"
#include "lattice.h"
#include "asymmunit.h"

          
//----------------------------------------------------------------------------//
// TSAtom function bodies
//----------------------------------------------------------------------------//

TSAtom::TSAtom(TNetwork *N) : TBasicNode<TSAtom, TSBond>(N)  {
  SetType(sotAtom);
  Grown = Deleted = false;
}
//..............................................................................
TSAtom::~TSAtom()  {  }
//..............................................................................
void  TSAtom::CAtom(TCAtom& S)  {
  FCAtom    = &S;
  FCCenter  = S.CCenter();
  if( Network != NULL )
    Network->GetLattice().GetAsymmUnit().CellToCartesian(FCCenter, FCenter);
  FEllipsoid = S.GetEllipsoid();
}
//..............................................................................
void TSAtom::AtomInfo(TBasicAtomInfo *AI)  {
  FCAtom->AtomInfo(AI);
}
//..............................................................................
void  TSAtom::Assign(TSAtom *S)  {
  TSObject::Assign(S);
  FEllipsoid = S->GetEllipsoid();
  FCenter    = S->Center();
  FCCenter   = S->CCenter();
  FCAtom     = &S->CAtom();
  Grown     = S->IsGrown();
  Matrices.Assign(S->Matrices);
}
//..............................................................................
bool TSAtom::IsGrown() {
  if( !Grown )  return false;
  int subs = 0;
  for( int i=0; i < Nodes.Count(); i++ )
    if( Nodes[i]->IsDeleted() )
      subs--;
  if( subs < 0 )
    Grown = false;
  return Grown;
}
//..............................................................................

