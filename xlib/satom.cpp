#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "satom.h"
#include "network.h"
#include "lattice.h"
#include "asymmunit.h"
#include "symmparser.h"

          
//----------------------------------------------------------------------------//
// TSAtom function bodies
//----------------------------------------------------------------------------//

TSAtom::TSAtom(TNetwork *N) : TBasicNode<TSAtom, TSBond>(N)  {
  SetType(sotAtom);
  Flags = 0;
}
//..............................................................................
TSAtom::~TSAtom()  {  }
//..............................................................................
void  TSAtom::CAtom(TCAtom& S)  {
  FCAtom    = &S;
  FCCenter  = S.ccrd();
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
  FCenter    = S->crd();
  FCCenter   = S->ccrd();
  FCAtom     = &S->CAtom();
  SetGrown( S->IsGrown() );
  Matrices.Assign(S->Matrices);
}
//..............................................................................
bool TSAtom::IsGrown() {
  if( (Flags & satomGrown) == 0 )  return false;
  int subs = 0;
  for( int i=0; i < Nodes.Count(); i++ )
    if( Nodes[i]->IsDeleted() )
      subs--;
  if( subs < 0 )
    SetGrown(false);
  return (Flags & satomGrown) != 0;
}
//..............................................................................
olxstr TSAtom::GetGuiLabel() const  {  
  olxstr rv(FCAtom->GetLabel());
  if( FCAtom->GetResiId() != -1 )  {
    rv << '_' <<
      FCAtom->GetParent()->GetResidue(FCAtom->GetResiId()).GetNumber();
  }
  if( Network == NULL || (Matrices[0]->r.IsI() && Matrices[0]->t.IsNull()) )
    return rv;
  else
    return rv << '.' << TSymmParser::MatrixToSymmCode(Network->GetLattice().GetUnitCell(), *Matrices[0]);
}
//..............................................................................
olxstr TSAtom::GetGuiLabelEx() const  {  
  olxstr rv(FCAtom->GetLabel());
  if( FCAtom->GetResiId() != -1 )  {
    rv << '_' <<
      FCAtom->GetParent()->GetResidue(FCAtom->GetResiId()).GetNumber();
  }
  if( Network == NULL || (Matrices[0]->r.IsI() && Matrices[0]->t.IsNull()) )
    return rv;
  else
    return rv << '(' << TSymmParser::MatrixToSymmEx(*Matrices[0]) << ')';
}

