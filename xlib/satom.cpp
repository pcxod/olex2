#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "satom.h"
#include "network.h"
#include "lattice.h"
#include "residue.h"
#include "symmparser.h"
#include "pers_util.h"

          
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
void TSAtom::AtomInfo(TBasicAtomInfo& AI)  {
  FCAtom->SetAtomInfo(AI);
}
//..............................................................................
int TSAtom::_SortBondsByLengthAsc(const TSBond* b1, const TSBond* b2)  {
  const double diff = b1->QLength() - b2->QLength();
  return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
}
//..............................................................................
int TSAtom::_SortBondsByLengthDsc(const TSBond* b1, const TSBond* b2)  {
  const double diff = b2->QLength() - b1->QLength();
  return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
}
//..............................................................................
void TSAtom::Assign(const TSAtom& S)  {
  TSObject<TNetwork>::Assign(S);
  FEllipsoid = S.GetEllipsoid();
  FCenter    = S.crd();
  FCCenter   = S.ccrd();
  FCAtom     = &S.CAtom();
  SetGrown( S.IsGrown() );
  Matrices.Assign(S.Matrices);
}
//..............................................................................
bool TSAtom::IsGrown() const {
  if( (Flags & satom_Grown) == 0 )  return false;
  int subs = 0;
  for( size_t i=0; i < Nodes.Count(); i++ )
    if( Nodes[i]->IsDeleted() )
      subs--;
  if( subs < 0 )
    SetBit(false, Flags, satom_Grown);
  return (Flags & satom_Grown) != 0;
}
//..............................................................................
olxstr TSAtom::GetGuiLabel() const  {  
  olxstr rv(FCAtom->GetLabel());
  if( FCAtom->GetResiId() != 0 )  {
    rv << '_' <<
      FCAtom->GetParent()->GetResidue(FCAtom->GetResiId()).GetNumber();
  }
  if( Network == NULL || (Matrices[0]->r.IsI() && Matrices[0]->t.IsNull()) )
    return rv;
  else
    return rv << '.' << TSymmParser::MatrixToSymmCode(Network->GetLattice().GetUnitCell(), *Matrices[0]);
}
//..............................................................................
void TSAtom::SetNodeCount(size_t cnt)  {
  if( cnt >= (size_t)Nodes.Count() )
    return;
  for( size_t i=cnt; i < Nodes.Count(); i++ )  {
    Nodes[i]->Nodes.Remove(this);
    Nodes[i] = NULL;
  }
  Nodes.Pack();
}
//..............................................................................
void TSAtom::RemoveNode(TSAtom& node)  {
  size_t ind = Nodes.IndexOf(&node);
  if( ind == InvalidIndex )  return;
  node.Nodes.Remove(this);
  Nodes.Delete(ind);
}
//..............................................................................
olxstr TSAtom::GetGuiLabelEx() const  {  
  olxstr rv(FCAtom->GetLabel());
  if( FCAtom->GetResiId() != 0 )  {
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
  // need to save it for overlayed images etc
  item.AddField("crd", PersUtil::VecToStr(FCenter) );
  TDataItem& nodes = item.AddItem("Nodes");
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->IsDeleted() )  continue;
    nodes.AddField("node_id", Nodes[i]->GetTag());
  }
  TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    bonds.AddField("bond_id", Bonds[i]->GetTag());
  }

  item.AddField("atom_id", FCAtom->GetTag());
  TDataItem& matrices = item.AddItem("Matrices");
  for( size_t i=0; i < Matrices.Count(); i++ )
    matrices.AddField("matr_id", Matrices[i]->GetTag());
}
//..............................................................................
void TSAtom::FromDataItem(const TDataItem& item, TLattice& parent) {
  Network = &parent.GetFragment( item.GetRequiredField("net_id").ToSizeT() );
  const TDataItem& nodes = item.FindRequiredItem("Nodes");
  Nodes.SetCapacity( nodes.FieldCount() );
  for( size_t i=0; i < nodes.FieldCount(); i++ )
    Nodes.Add(&parent.GetAtom(nodes.GetField(i).ToSizeT()));
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity( bonds.FieldCount() );
  for( size_t i=0; i < bonds.FieldCount(); i++ )
    Bonds.Add(&parent.GetBond(bonds.GetField(i).ToSizeT()));

  TLattice& latt = Network->GetLattice();
  size_t ca_id = item.GetRequiredField("atom_id").ToSizeT();
  CAtom( latt.GetAsymmUnit().GetAtom(ca_id) );
  const TDataItem& matrices = item.FindRequiredItem("Matrices");
  Matrices.SetCapacity( matrices.FieldCount() );
  for( size_t i=0; i < matrices.FieldCount(); i++ )  {
    const size_t mi = matrices.GetField(i).ToSizeT();
    Matrices.Add( &latt.GetMatrix(mi) );
  }
  FCCenter = *Matrices[0] * FCCenter;
  FCenter = PersUtil::FloatVecFromStr(item.GetRequiredField("crd"));
  //latt.GetAsymmUnit().CellToCartesian(FCCenter, FCenter);
}
//..............................................................................
