/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "satom.h"
#include "network.h"
#include "lattice.h"
#include "residue.h"
#include "symmparser.h"
#include "unitcell.h"
#include "pers_util.h"
#include "index_range.h"
          
TSAtom::TSAtom(TNetwork *N) : TBasicNode<TNetwork, TSAtom, TSBond>(N)  {
  SetType(sotAtom);
  Flags = 0;
}
//..............................................................................
void  TSAtom::CAtom(TCAtom& S)  {
  FCAtom = &S;
  FCCenter = S.ccrd();
  FEllipsoid = S.GetEllipsoid();
  SetMasked(S.IsMasked());
}
//..............................................................................
int TSAtom::_SortBondsByLengthAsc(const TSBond* b1, const TSBond* b2)  {
  return olx_cmp(b1->QLength(), b2->QLength());
}
//..............................................................................
int TSAtom::_SortBondsByLengthDsc(const TSBond* b1, const TSBond* b2)  {
  return olx_cmp(b2->QLength(), b1->QLength());
}
//..............................................................................
void TSAtom::Assign(const TSAtom& S)  {
  TSObject<TNetwork>::Assign(S);
  FEllipsoid = S.GetEllipsoid();
  FCenter    = S.crd();
  FCCenter   = S.ccrd();
  FCAtom     = &S.CAtom();
  Matrices.Assign(S.Matrices);
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
    return rv << '.' << TSymmParser::MatrixToSymmCode(
    Network->GetLattice().GetUnitCell().GetSymSpace(), *Matrices[0]);
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
  item.AddField("crd", PersUtil::VecToStr(FCenter));
  item.AddField("flags", Flags);
  if( Network->GetTag() >= 0 )  {
    IndexRange::Builder rb;
    for( size_t i=0; i < Nodes.Count(); i++ )  {
      if( Nodes[i]->IsDeleted() )  continue;
      rb << Nodes[i]->GetTag();
    }
    item.AddField("node_range", rb.GetString(true));
    for( size_t i=0; i < Bonds.Count(); i++ )  {
      if( Bonds[i]->IsDeleted() )  continue;
      rb << Bonds[i]->GetTag();
    }
    item.AddField("bond_range", rb.GetString(true));
  }
  item.AddField("atom_id", FCAtom->GetTag());
  TDataItem& matrices = item.AddItem("Matrices");
  for( size_t i=0; i < Matrices.Count(); i++ )
    matrices.AddField("matr_id", Matrices[i]->GetId());
}
//..............................................................................
void TSAtom::FromDataItem(const TDataItem& item, TLattice& parent) {
  const size_t net_id = item.GetRequiredField("net_id").ToInt();
  Network = &parent.GetFragment(net_id);
  if( net_id != InvalidIndex )  {
    ASObjectProvider& objects = parent.GetObjects();
    const TDataItem* _nodes = item.FindItem("Nodes");
    if( _nodes != NULL )  {
      const TDataItem& nodes = *_nodes;
      Nodes.SetCapacity(nodes.FieldCount());
      for( size_t i=0; i < nodes.FieldCount(); i++ )
        Nodes.Add(objects.atoms[nodes.GetField(i).ToSizeT()]);
      const TDataItem& bonds = item.FindRequiredItem("Bonds");
      Bonds.SetCapacity(bonds.FieldCount());
      for( size_t i=0; i < bonds.FieldCount(); i++ )
        Bonds.Add(objects.bonds[bonds.GetField(i).ToSizeT()]);
    }
    else  {  // index range then
      IndexRange::RangeItr ai(item.GetRequiredField("node_range"));
      Nodes.SetCapacity(ai.CalcSize());
      while( ai.HasNext() )
        Nodes.Add(objects.atoms[ai.Next()]);
      IndexRange::RangeItr bi(item.GetRequiredField("bond_range"));
      Bonds.SetCapacity(bi.CalcSize());
      while( bi.HasNext() )
        Bonds.Add(objects.bonds[bi.Next()]);
    }
  }
  TLattice& latt = Network->GetLattice();
  const size_t ca_id = item.GetRequiredField("atom_id").ToSizeT();
  CAtom(latt.GetAsymmUnit().GetAtom(ca_id));
  const TDataItem& matrices = item.FindRequiredItem("Matrices");
  Matrices.SetCapacity(matrices.FieldCount());
  for( size_t i=0; i < matrices.FieldCount(); i++ )  {
    const size_t mi = matrices.GetField(i).ToSizeT();
    Matrices.Add(latt.GetMatrix(mi));
  }
  FCCenter = *Matrices[0] * FCCenter;
  FCenter = PersUtil::FloatVecFromStr(item.GetRequiredField("crd"));
  Flags = item.GetRequiredField("flags").ToInt();
  //latt.GetAsymmUnit().CellToCartesian(FCCenter, FCenter);
}
//..............................................................................
