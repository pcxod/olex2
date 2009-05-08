#include "conninfo.h"
#include "atomref.h"
#include "xapp.h"

void ConnInfo::ProcessFree(const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  int aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(TExceptionBase& ex)  {
    throw TFunctionFailedException(__OlxSourceInfo, ex, "Failed to locate atoms");
  }
  if( ag.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "Two atoms are expected for FREE");
  if( ag[0].GetMatrix() != NULL && ag[1].GetMatrix() != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "At maximum one equivalent position is expectd for FREE");
  // validate
  if( ag[0].GetAtom()->GetId() == ag[1].GetAtom()->GetId() )  {
    if( (ag[0].GetMatrix() != NULL && ag[0].GetMatrix()->r.IsI() && ag[0].GetMatrix()->t.IsNull()) || 
        (ag[1].GetMatrix() != NULL && ag[1].GetMatrix()->r.IsI() && ag[1].GetMatrix()->t.IsNull()) || 
        ag[0].GetMatrix() == NULL && ag[1].GetMatrix() == NULL )  
    {
      TBasicApp::GetLog().Error(  olxstr("Skipping: FREE ") << ar.GetExpression() );
      return;
    }
  }
  RemBond( *ag[0].GetAtom(), *ag[1].GetAtom(), ag[0].GetMatrix(), ag[1].GetMatrix(), true );
}
//........................................................................
void ConnInfo::ProcessBind(const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  int aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(TExceptionBase& ex)  {
    throw TFunctionFailedException(__OlxSourceInfo, ex, "Failed to locate atoms");
  }
  if( ag.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "Two atoms are expected for BIND");
  if( ag[0].GetMatrix() != NULL && ag[1].GetMatrix() != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "At maximum one equivalent position is expectd for BIND");
  // validate
  if( ag[0].GetAtom()->GetId() == ag[1].GetAtom()->GetId() )  {
    if( (ag[0].GetMatrix() != NULL && ag[0].GetMatrix()->r.IsI() && ag[0].GetMatrix()->t.IsNull()) || 
        (ag[1].GetMatrix() != NULL && ag[1].GetMatrix()->r.IsI() && ag[1].GetMatrix()->t.IsNull()) || 
        ag[0].GetMatrix() == NULL && ag[1].GetMatrix() == NULL )  
    {
      TBasicApp::GetLog().Error(  olxstr("Skipping: BIND ") << ar.GetExpression() );
      return;
    }
  }
  AddBond( *ag[0].GetAtom(), *ag[1].GetAtom(), ag[0].GetMatrix(), ag[1].GetMatrix(), true );
}
//........................................................................
void ConnInfo::ProcessConn(TStrList& ins)  {
  short maxB = def_max_bonds;
  double r = -1;
  TIntList num_indexes;
  for( int i=0; i < ins.Count(); i++ )  {
    if( ins[i].IsNumber() )
      num_indexes.Add(i);
  }
  if( num_indexes.Count() == 2 )  {
    maxB = ins[num_indexes[0]].ToInt();
    r = ins[num_indexes[1]].ToDouble();
  }
  else if( num_indexes.Count() == 1 )  {
    if( ins[num_indexes[0]].IndexOf('.') != -1 )
      r = ins[num_indexes[0]].ToDouble();
    else
      maxB = ins[num_indexes[0]].ToInt();
  }
  else  // invalid argument set - reset any existing conn info
    ;
  // remove numbers to leave atom names/types only
  for( int i=num_indexes.Count()-1; i >=0; i-- )
    ins.Delete(num_indexes[i]);
  // extract and remove atom types
  for( int i=0; i < ins.Count(); i++ )  {
    if( ins[i].CharAt(0) == '$' )  {
      TBasicAtomInfo* bai = TAtomsInfo::GetInstance().FindAtomInfoBySymbol(ins[i].SubStringFrom(1));
      if( bai == NULL )  {
        TBasicApp::GetLog().Error(olxstr("Undefined atom type in CONN: ") << ins[i].SubStringFrom(1));
        ins.Delete(i--);
        continue;
      }
      TypeConnInfo& ci = TypeInfo.Add(bai, TypeConnInfo(*bai) );
      ci.maxBonds = maxB;
      ci.r = r;
      ins.Delete(i--);
    }
  }
  if( !ins.IsEmpty() )  {
    TSAtomPList atoms;
    TXApp::GetInstance().FindSAtoms(ins.Text(' '), atoms);
    for( int i=0; i < atoms.Count(); i++ )  {
      AtomConnInfo& ai = AtomInfo.Add(&atoms[i]->CAtom(), AtomConnInfo(atoms[i]->CAtom()));
      ai.maxBonds = maxB;
      ai.r = r;
    }
  }
}
//........................................................................
void ConnInfo::ToInsList(TStrList& ins) const {
  // put the type specific info first
  for( int i=0; i < TypeInfo.Count(); i++ )  {
    const TypeConnInfo& tci = TypeInfo.GetValue(i);
    if( tci.maxBonds == def_max_bonds && tci.r == -1 )
      continue;
    olxstr& str = ins.Add("CONN ");
    if( tci.maxBonds != def_max_bonds )
      str << tci.maxBonds << ' ';
    if( tci.r != -1 )
      str << tci.r << ' ';
    str << '$' << tci.atomInfo->GetSymbol();
  }
  // specialisation for particular atoms to follow the generic type info
  for( int i=0; i < AtomInfo.Count(); i++ )  {
    const AtomConnInfo& aci = AtomInfo.GetValue(i);
    if( aci.atom->IsDeleted() )
      continue;
    if( aci.r != -1 || aci.maxBonds != def_max_bonds )  {
      olxstr& str = ins.Add("CONN ");
      if( aci.maxBonds != def_max_bonds )
        str << aci.maxBonds << ' ';
      if( aci.r != -1 )
        str << aci.r << ' ';
      str << aci.atom->GetLabel();
    }
    for( int j=0; j < aci.BondsToCreate.Count(); j++ )  {
      const CXBondInfo& bi = aci.BondsToCreate[j];
      if( bi.to.IsDeleted() )
        continue;
      olxstr& str = ins.Add("BIND ");
      str << aci.atom->GetLabel() << ' ' << bi.to.GetLabel();
      if( bi.matr != NULL )  {
        int si = rm.UsedSymmIndex(*bi.matr);
        if( si == -1 )
          throw TFunctionFailedException(__OlxSourceInfo, "Undefined EQIV in BIND");
        str << "_$" << (si+1);
      }
    }
    for( int j=0; j < aci.BondsToRemove.Count(); j++ )  {
      const CXBondInfo& bi = aci.BondsToRemove[j];
      if( bi.to.IsDeleted() )
        continue;
      olxstr& str = ins.Add("FREE ");
      str << aci.atom->GetLabel() << ' ' << bi.to.GetLabel();
      if( bi.matr != NULL )  {
        int si = rm.UsedSymmIndex(*bi.matr);
        if( si == -1 )
          throw TFunctionFailedException(__OlxSourceInfo, "Undefined EQIV in BIND");
        str << "_$" << (si+1);
      }
    }
  }
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(const TCAtom& ca) const {
  CXConnInfo& ci = *(new CXConnInfo);  
  int ai_ind = AtomInfo.IndexOf(&ca), ti_ind;
  if( ai_ind != -1 )  {
    const AtomConnInfo& aci = AtomInfo.GetValue(ai_ind);
    ci.r = aci.r < 0 ? ca.GetAtomInfo().GetRad1() : aci.r;
    ci.maxBonds = aci.maxBonds;
    ci.BondsToCreate.AddListC(aci.BondsToCreate);
    ci.BondsToRemove.AddListC(aci.BondsToRemove);
  } 
  else if( (ti_ind = TypeInfo.IndexOf(&ca.GetAtomInfo())) != -1 )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? ca.GetAtomInfo().GetRad1() : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  else  {
    ci.r = ca.GetAtomInfo().GetRad1();
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(TBasicAtomInfo& bai) const {
  CXConnInfo& ci = *(new CXConnInfo);
  int ti_ind = TypeInfo.IndexOf(&bai);
  if( ti_ind != -1 )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? bai.GetRad1() : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  else  {
    ci.r = bai.GetRad1();
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
void ConnInfo::Assign(const ConnInfo& ci)  {
  Clear();
  for( int i=0; i < ci.AtomInfo.Count(); i++ )  {
    const AtomConnInfo& _aci = ci.AtomInfo.GetValue(i);
    if( _aci.atom->IsDeleted() )
      continue;
    TCAtom* ca = rm.aunit.FindCAtomById(_aci.atom->GetId());
    if( ca == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Asymmetric units mismatch");
    AtomConnInfo& aci = AtomInfo.Add(ca);
    aci.atom = ca;
    (CXConnInfoBase&)aci = _aci;
    for( int j=0; j < _aci.BondsToCreate.Count(); j++ )  {
      ca = rm.aunit.FindCAtomById(_aci.BondsToCreate[j].to.GetId());
      if( ca == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "Asymmetric units mismatch");
      if( ca->IsDeleted() )
        continue;
      const smatd* sm = _aci.BondsToCreate[j].matr == NULL ? NULL :
                        &rm.AddUsedSymm(*_aci.BondsToCreate[j].matr);
      aci.BondsToCreate.Add( new CXBondInfo(*ca, sm) );
    }
    for( int j=0; j < _aci.BondsToRemove.Count(); j++ )  {
      ca = rm.aunit.FindCAtomById(_aci.BondsToRemove[j].to.GetId());
      if( ca == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "Asymmetric units mismatch");
      if( ca->IsDeleted() )
        continue;
      const smatd* sm = _aci.BondsToRemove[j].matr == NULL ? NULL :
                        &rm.AddUsedSymm(*_aci.BondsToRemove[j].matr);
      aci.BondsToRemove.Add( new CXBondInfo(*ca, sm) );
    }
  }
  for( int i=0; i < ci.TypeInfo.Count(); i++ ) 
    TypeInfo.Add( ci.TypeInfo.GetKey(i), ci.TypeInfo.GetValue(i) );
}
//........................................................................
void ConnInfo::ToDataItem(TDataItem& item)  {
  TDataItem& ti_item = item.AddItem("TYPE");
  for( int i=0; i < TypeInfo.Count(); i++ )
    TypeInfo.GetValue(i).ToDataItem(ti_item.AddItem(TypeInfo.GetValue(i).atomInfo->GetSymbol()));
  TDataItem& ai_item = item.AddItem("ATOM");
  for( int i=0; i < AtomInfo.Count(); i++ ) 
    AtomInfo.GetValue(i).ToDataItem(ai_item.AddItem(AtomInfo.GetValue(i).atom->GetTag()));
}
//........................................................................
void ConnInfo::FromDataItem(TDataItem& item)  {
  TAtomsInfo& ai = TAtomsInfo::GetInstance();
  TDataItem& ti_item = item.FindRequiredItem("TYPE");
  for( int i=0; i < ti_item.ItemCount(); i++ )  {
    TBasicAtomInfo* bai = ai.FindAtomInfoBySymbol(ti_item.GetItem(i).GetName());
    if( bai == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown symbol: ") << ti_item.GetItem(i).GetName());
    TypeInfo.Add(bai).FromDataItem(ti_item.GetItem(i), bai);

  }
  TDataItem& ai_item = item.FindRequiredItem("ATOM");
  for( int i=0; i < ai_item.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom(ai_item.GetItem(i).GetName().ToInt());
    AtomInfo.Add(&ca).FromDataItem(ai_item.GetItem(i), rm, ca);
  }
}
//........................................................................
//........................................................................
//........................................................................
void ConnInfo::TypeConnInfo::ToDataItem(TDataItem& item)  {
  item.AddField("r", r);
  item.AddField("b", maxBonds);
}
void ConnInfo::TypeConnInfo::FromDataItem(TDataItem& item, TBasicAtomInfo* bai)  {
  atomInfo = bai;
  r = item.GetRequiredField("r").ToDouble();
  maxBonds = item.GetRequiredField("b").ToInt();
}
//........................................................................
int ConnInfo::FindBondIndex(const BondInfoList& list, TCAtom* key, TCAtom& a1, TCAtom& a2, const smatd* eqiv) const {
  if( key != &a1 && key != &a2 )
    return -1;
  if( key == &a1 )  {
    for( int i=0; i < list.Count(); i++ )  {
      if( list[i].to == a2 )  {
        if( list[i].matr == NULL && eqiv == NULL )
          return i;
        if( list[i].matr != NULL && eqiv != NULL && *list[i].matr == *eqiv )  
          return i;
      }
    }
    return -1;
  }
  else if( eqiv == NULL )  {
    for( int i=0; i < list.Count(); i++ )  {
      if( list[i].to == a1 )
        return i;
    }
    return -1;
  }
  else  {
    smatd mat(*eqiv);
    mat.r = eqiv->r.Inverse();
    mat.t *= -1;
    for( int i=0; i < list.Count(); i++ )  {
      if( list[i].to == a1 && list[i].matr != NULL && *list[i].matr == mat )  
        return i;
    }
    return -1;
  }
}
//........................................................................
const smatd* ConnInfo::GetCorrectMatrix(const smatd* eqiv1, const smatd* eqiv2, bool release) const {
  if( eqiv1 == NULL || (eqiv1->r.IsI() && eqiv1->t.IsNull()) )  {
    if( release && eqiv1 != NULL )
      rm.RemUsedSymm(*eqiv1);
    return eqiv2;
  }
  if( eqiv2 == NULL || (eqiv2->r.IsI() && eqiv2->t.IsNull()) )  {
    smatd mat(*eqiv1);
    mat.r = eqiv1->r.Inverse();
    mat.t *= -1;
    if( release )  {
      rm.RemUsedSymm(*eqiv1);
      if( eqiv2 != NULL )
        rm.RemUsedSymm(*eqiv2);
    }
    return &rm.AddUsedSymm(mat);
  }
  throw TFunctionFailedException(__OlxSourceInfo, "both symops are not identity");
}
//........................................................................
void ConnInfo::AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv)  {
  const smatd* eqiv = NULL;
  try {  eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);  }
  catch( ... )  {
    TBasicApp::GetLog().Error( olxstr("Failed to add bond: only one EQIV is expected"));
    return;
  }
  int ind = -1;
  for( int i=0; i < AtomInfo.Count(); i++ )  {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate, AtomInfo.GetKey(i), a1, a2, eqiv); 
    if( ind != -1 )
      break;
  }
  if( ind == -1 )  {
    // validate the bonds to delete
    bool exists = false;
    for( int i=0; i < AtomInfo.Count(); i++ )  {
      ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i), a1, a2, eqiv);
      if( ind != -1 )  {
        AtomInfo.GetValue(i).BondsToRemove.Delete(ind);
        exists = true;
        break;
      }
    }
    if( ! exists )  {  // if was deleted - may be already exists?
      AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
      ai.BondsToCreate.Add( new CXBondInfo(a2, eqiv) );
    }
  }
}
//........................................................................
void ConnInfo::RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv)  {
  const smatd* eqiv = NULL;
  try {  eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);  }
  catch( ... )  {
    TBasicApp::GetLog().Error( olxstr("Failed to delete bond: only one EQIV is expected"));
    return;
  }
  int ind = -1;
  for( int i=0; i < AtomInfo.Count(); i++ )  {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i), a1, a2, eqiv);
    if( ind != -1 )
      break;
  }
  if( ind == -1 )  {
    // validate the bonds to create
    bool exists = false;
    for( int i=0; i < AtomInfo.Count(); i++ )  {
      ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate, AtomInfo.GetKey(i), a1, a2, eqiv);
      if( ind != -1 )  {
        AtomInfo.GetValue(i).BondsToCreate.Delete(ind);
        exists = true;
        break;
      }
    }
    if( !exists )  {  // if was added - then it might not exist?
      AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
      ai.BondsToRemove.Add( new CXBondInfo(a2, eqiv) );
    }
  }
}
//........................................................................
void ConnInfo::AtomConnInfo::ToDataItem(TDataItem& item)  {
  item.AddField("r", r);
  item.AddField("b", maxBonds);
  TDataItem& ab = item.AddItem("ADDBOND");
  for( int i=0; i < BondsToCreate.Count(); i++ )  {
    if( BondsToCreate[i].to.IsDeleted() )
      continue;
    TDataItem& bi = ab.AddItem("bi");
    bi.AddField("to", BondsToCreate[i].to.GetTag());
    if( BondsToCreate[i].matr != NULL )
      bi.AddField("eqiv", BondsToCreate[i].matr->GetTag());
  }
  TDataItem& db = item.AddItem("DELBOND");
  for( int i=0; i < BondsToRemove.Count(); i++ )  {
    if( BondsToRemove[i].to.IsDeleted() )
      continue;
    TDataItem& bi = ab.AddItem("bi");
    bi.AddField("to", BondsToRemove[i].to.GetTag());
    if( BondsToCreate[i].matr != NULL )
      bi.AddField("eqiv", BondsToRemove[i].matr->GetTag());
  }
}
void ConnInfo::AtomConnInfo::FromDataItem(TDataItem& item, RefinementModel& rm, TCAtom& a)  {
  atom = &a;
  r = item.GetRequiredField("r").ToDouble();
  maxBonds = item.GetRequiredField("b").ToInt();
  TDataItem& ab = item.FindRequiredItem("ADDBOND");
  for( int i=0; i < ab.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom( ab.GetItem(i).GetRequiredField("to").ToInt() );
    const olxstr& eq = ab.GetItem(i).GetFieldValue("eqiv");
    smatd const* eqiv = NULL;
    if( !eq.IsEmpty() )  { 
      eqiv = &rm.GetUsedSymm( eq.ToInt() );
      rm.AddUsedSymm(*eqiv);  // persist
    }
    BondsToCreate.Add( new CXBondInfo(ca, eqiv) );
  }
  TDataItem& db = item.FindRequiredItem("DELBOND");
  for( int i=0; i < db.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom( db.GetItem(i).GetRequiredField("to").ToInt() );
    const olxstr& eq = db.GetItem(i).GetFieldValue("eqiv");
    smatd const* eqiv = NULL;
    if( !eq.IsEmpty() )  { 
      eqiv = &rm.GetUsedSymm( eq.ToInt() );
      rm.AddUsedSymm(*eqiv);  // persist
    }
    BondsToRemove.Add( new CXBondInfo(ca, eqiv) );
  }
}
//........................................................................
