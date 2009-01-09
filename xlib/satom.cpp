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

TSAtom::TSAtom(TNetwork *N) : TBasicNode<TNetwork, TSAtom, TSBond>(N)  {
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
void TSAtom::AtomInfo(TBasicAtomInfo* AI)  {
  FCAtom->SetAtomInfo(AI);
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
//..............................................................................
void TSAtom::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network->GetTag());
  TDataItem& nodes = item.AddItem("Nodes");
  for( int i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->IsDeleted() )  continue;
    nodes.AddField("node_id", Nodes[i]->GetTag());
  }
  TDataItem& bonds = item.AddItem("Bonds");
  for( int i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    bonds.AddField("bond_id", Bonds[i]->GetTag());
  }

  item.AddField("atom_id", FCAtom->GetTag());
  TDataItem& matrices = item.AddItem("Matrices");
  for( int i=0; i < Matrices.Count(); i++ )
    matrices.AddField("matr_id", Matrices[i]->GetTag());
}
//..............................................................................
void TSAtom::FromDataItem(const TDataItem& item, TLattice& parent) {
  Network = &parent.GetFragment( item.GetRequiredField("net_id").ToInt() );
  const TDataItem& nodes = item.FindRequiredItem("Nodes");
  Nodes.SetCapacity( nodes.FieldCount() );
  for( int i=0; i < nodes.FieldCount(); i++ )
    Nodes.Add(&parent.GetAtom(nodes.GetField(i).ToInt()));
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity( bonds.FieldCount() );
  for( int i=0; i < bonds.FieldCount(); i++ )
    Bonds.Add(&parent.GetBond(bonds.GetField(i).ToInt()));

  TLattice& latt = Network->GetLattice();
  int ca_id = item.GetRequiredField("atom_id").ToInt();
  CAtom( latt.GetAsymmUnit().GetAtom(ca_id) );
  const TDataItem& matrices = item.FindRequiredItem("Matrices");
  Matrices.SetCapacity( matrices.FieldCount() );
  int fm_id = -1;
  for( int i=0; i < matrices.FieldCount(); i++ )  {
    const int mi = matrices.GetField(i).ToInt();
    Matrices.Add( &latt.GetMatrix(mi) );
    if( i == 0 )
      fm_id = mi;
  }
  if( fm_id != 0 )  // identity matrix
    FCCenter = *Matrices[0] * FCCenter;
  latt.GetAsymmUnit().CellToCartesian(FCCenter, FCenter);
}
//..............................................................................
