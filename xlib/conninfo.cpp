#include "conninfo.h"
#include "atomref.h"
#include "xapp.h"
#include "unitcell.h"

void ConnInfo::ProcessFree(const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  size_t aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(const TExceptionBase&)  {
    TBasicApp::GetLog().Error(olxstr("Could not locate atoms for FREE ") << ar.GetExpression());
    return;
  }
  if( ag.Count() == 0 || (ag.Count()%2) != 0 )  {
    TBasicApp::GetLog().Error(olxstr("Even number of atoms is expected for FREE ") <<
      ar.GetExpression() << ' ' << ag.Count() << " is provided");
    return;
  }
  for( size_t i=0; i < ag.Count(); i += 2 )  {
    if( ag[i].GetMatrix() != NULL && ag[i+1].GetMatrix() != NULL )  {
      TBasicApp::GetLog().Error(olxstr("Only one eqivalent is expected in FREE, skipping ") << 
        ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm) );
      continue;
    }
    // validate
    if( ag[i].GetAtom()->GetId() == ag[i+1].GetAtom()->GetId() )  {
      if( (ag[i].GetMatrix() != NULL && ag[i].GetMatrix()->r.IsI() && ag[i].GetMatrix()->t.IsNull()) || 
        (ag[i+1].GetMatrix() != NULL && ag[i+1].GetMatrix()->r.IsI() && ag[i+1].GetMatrix()->t.IsNull()) || 
        ag[i].GetMatrix() == NULL && ag[i+1].GetMatrix() == NULL )  
      {
        TBasicApp::GetLog().Error(olxstr("Dummy  FREE, skipping ") <<
          ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm) );
        continue;
      }
    }
    RemBond(*ag[i].GetAtom(), *ag[i+1].GetAtom(), ag[i].GetMatrix(), ag[i+1].GetMatrix(), true );
  }
}
//........................................................................
void ConnInfo::ProcessBind(const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  size_t aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(const TExceptionBase&)  {
    TBasicApp::GetLog().Error(olxstr("Could not locate atoms for BIND ") << ar.GetExpression());
    return;
  }
  if( ag.Count() == 0 || (ag.Count()%2) != 0 )  {
    TBasicApp::GetLog().Error(olxstr("Even number of atoms is expected for FREE ") <<
      ar.GetExpression() << ' ' << ag.Count() << " is provided");
    return;
  }
  for( size_t i=0; i < ag.Count(); i += 2 )  {
    if( ag[i].GetMatrix() != NULL && ag[i+1].GetMatrix() != NULL )  {
      TBasicApp::GetLog().Error(olxstr("Only one eqivalent is expected in BIND, skipping ") << 
        ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm) );
      continue;
    }
    // validate
    if( ag[i].GetAtom()->GetId() == ag[i+1].GetAtom()->GetId() )  {
      if( (ag[i].GetMatrix() != NULL && ag[i].GetMatrix()->r.IsI() && ag[i].GetMatrix()->t.IsNull()) || 
        (ag[i+1].GetMatrix() != NULL && ag[i+1].GetMatrix()->r.IsI() && ag[i+1].GetMatrix()->t.IsNull()) || 
        ag[i].GetMatrix() == NULL && ag[i+1].GetMatrix() == NULL )  
      {
        TBasicApp::GetLog().Error(olxstr("Dummy  BIND, skipping ") <<
          ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm) );
        continue;
      }
    }
    AddBond(*ag[i].GetAtom(), *ag[i+1].GetAtom(), ag[i].GetMatrix(), ag[i+1].GetMatrix(), true );
  }
}
//........................................................................
void ConnInfo::Disconnect(TCAtom& ca)  {
  const size_t i = AtomInfo.IndexOf(&ca);
  if( i == InvalidIndex )  {
    AtomConnInfo& ai = AtomInfo.Add(&ca, AtomConnInfo(ca));
    ai.maxBonds = 0;
    ai.r = -1;
  }
  else  {
    AtomInfo.GetValue(i).maxBonds = 0;
  }
  ca.SetConnInfo(GetConnInfo(ca));
}
//........................................................................
void ConnInfo::ProcessConn(TStrList& ins)  {
  short maxB = def_max_bonds;
  double r = -1;
  TSizeList num_indexes;
  for( size_t i=0; i < ins.Count(); i++ )  {
    if( ins[i].IsNumber() )
      num_indexes.Add(i);
  }
  if( num_indexes.Count() == 2 )  {
    maxB = ins[num_indexes[0]].ToInt();
    r = ins[num_indexes[1]].ToDouble();
  }
  else if( num_indexes.Count() == 1 )  {
    if( ins[num_indexes[0]].IndexOf('.') != InvalidIndex )
      r = ins[num_indexes[0]].ToDouble();
    else
      maxB = ins[num_indexes[0]].ToInt();
  }
  else  // invalid argument set - reset any existing conn info
    ;
  // remove numbers to leave atom names/types only
  for( size_t i=num_indexes.Count(); i > 0; i-- )
    ins.Delete(num_indexes[i-1]);
  // extract and remove atom types
  for( size_t i=0; i < ins.Count(); i++ )  {
    if( ins[i].CharAt(0) == '$' )  {
      cm_Element* elm = XElementLib::FindBySymbol(ins[i].SubStringFrom(1));
      if( elm == NULL )  {
        TBasicApp::GetLog().Error(olxstr("Undefined atom type in CONN: ") << ins[i].SubStringFrom(1));
        ins.Delete(i--);
        continue;
      }
      TypeConnInfo& ci = TypeInfo.Add(elm, TypeConnInfo(*elm) );
      ci.maxBonds = maxB;
      ci.r = r;
      ins.Delete(i--);
    }
  }
  if( !ins.IsEmpty() )  {
    TAtomReference ar(ins.Text(' '));
    TCAtomGroup ag;
    size_t aag;
    try  {  ar.Expand(rm, ag, EmptyString, aag);  }
    catch(const TExceptionBase& )  {  }
    if( ag.IsEmpty() )  {
      TBasicApp::GetLog().Error( olxstr("Undefined atom in CONN: ") << ins.Text(' ') );
      return;
    }
    for( size_t i=0; i < ag.Count(); i++ )  {
      if( maxB == 12 && r == -1 )  {  // reset to default?
        size_t ai = AtomInfo.IndexOf(ag[i].GetAtom());
        if( ai != InvalidIndex )
          AtomInfo.Delete(ai);
      }
      else  {
        AtomConnInfo& ai = AtomInfo.Add(ag[i].GetAtom(), AtomConnInfo(*ag[i].GetAtom()));
        ai.maxBonds = maxB;
        ai.r = r;
      }
    }
  }
}
//........................................................................
void ConnInfo::ToInsList(TStrList& ins) const {
  // put the type specific info first
  for( size_t i=0; i < TypeInfo.Count(); i++ )  {
    const TypeConnInfo& tci = TypeInfo.GetValue(i);
    if( tci.maxBonds == def_max_bonds && tci.r == -1 )
      continue;
    olxstr& str = ins.Add("CONN ");
    if( tci.maxBonds != def_max_bonds )
      str << tci.maxBonds << ' ';
    if( tci.r != -1 )
      str << tci.r << ' ';
    str << '$' << tci.atomType->symbol;
  }
  // specialisation for particular atoms to follow the generic type info
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    const AtomConnInfo& aci = AtomInfo.GetValue(i);
    if( aci.atom->IsDeleted() )
      continue;
    if( aci.r != -1 || aci.maxBonds != def_max_bonds )  {
      olxstr& str = ins.Add("CONN ");
      if( aci.maxBonds != def_max_bonds || aci.r != -1 )
        str << aci.maxBonds << ' ';
      if( aci.r != -1 )
        str << aci.r << ' ';
      str << aci.atom->GetLabel();
    }
    for( size_t j=0; j < aci.BondsToCreate.Count(); j++ )  {
      const CXBondInfo& bi = aci.BondsToCreate[j];
      if( bi.to.IsDeleted() )
        continue;
      olxstr& str = ins.Add("BIND ");
      str << aci.atom->GetLabel() << ' ' << bi.to.GetLabel();
      if( bi.matr != NULL )  {
        size_t si = rm.UsedSymmIndex(*bi.matr);
        if( si == InvalidIndex )
          throw TFunctionFailedException(__OlxSourceInfo, "Undefined EQIV in BIND");
        str << "_$" << (si+1);
      }
    }
    for( size_t j=0; j < aci.BondsToRemove.Count(); j++ )  {
      const CXBondInfo& bi = aci.BondsToRemove[j];
      if( bi.to.IsDeleted() )
        continue;
      olxstr& str = ins.Add("FREE ");
      str << aci.atom->GetLabel() << ' ' << bi.to.GetLabel();
      if( bi.matr != NULL )  {
        size_t si = rm.UsedSymmIndex(*bi.matr);
        if( si == InvalidIndex )
          throw TFunctionFailedException(__OlxSourceInfo, "Undefined EQIV in BIND");
        str << "_$" << (si+1);
      }
    }
  }
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(const TCAtom& ca) const {
  CXConnInfo& ci = *(new CXConnInfo);  
  size_t ai_ind, ti_ind;
  if( (ti_ind = TypeInfo.IndexOf(&ca.GetType())) != InvalidIndex )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? ca.GetType().r_bonding : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  // specialise the connectivity info...
  if( (ai_ind = AtomInfo.IndexOf(&ca)) != InvalidIndex )  {
    const AtomConnInfo& aci = AtomInfo.GetValue(ai_ind);
    ci.r = aci.r < 0 ? ca.GetType().r_bonding : aci.r;
    ci.maxBonds = aci.maxBonds;
    ci.BondsToCreate.AddListC(aci.BondsToCreate);
    ci.BondsToRemove.AddListC(aci.BondsToRemove);
  } 
  // use defaults then
  if( ai_ind == InvalidIndex && ti_ind == InvalidIndex )  {
    ci.r = ca.GetType().r_bonding;
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(const cm_Element& elm) const {
  CXConnInfo& ci = *(new CXConnInfo);
  size_t ti_ind = TypeInfo.IndexOf(&elm);
  if( ti_ind != InvalidIndex )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? elm.r_bonding : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  else  {
    ci.r = elm.r_bonding;
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
void ConnInfo::Assign(const ConnInfo& ci)  {
  Clear();
  for( size_t i=0; i < ci.AtomInfo.Count(); i++ )  {
    const AtomConnInfo& _aci = ci.AtomInfo.GetValue(i);
    if( _aci.atom->IsDeleted() )
      continue;
    TCAtom* ca = rm.aunit.FindCAtomById(_aci.atom->GetId());
    if( ca == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Asymmetric units mismatch");
    AtomConnInfo& aci = AtomInfo.Add(ca);
    aci.atom = ca;
    (CXConnInfoBase&)aci = _aci;
    for( size_t j=0; j < _aci.BondsToCreate.Count(); j++ )  {
      ca = rm.aunit.FindCAtomById(_aci.BondsToCreate[j].to.GetId());
      if( ca == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "Asymmetric units mismatch");
      if( ca->IsDeleted() )
        continue;
      const smatd* sm = _aci.BondsToCreate[j].matr == NULL ? NULL :
                        &rm.AddUsedSymm(*_aci.BondsToCreate[j].matr);
      aci.BondsToCreate.Add( new CXBondInfo(*ca, sm) );
    }
    for( size_t j=0; j < _aci.BondsToRemove.Count(); j++ )  {
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
  for( size_t i=0; i < ci.TypeInfo.Count(); i++ ) 
    TypeInfo.Add( ci.TypeInfo.GetKey(i), ci.TypeInfo.GetValue(i) );
}
//........................................................................
void ConnInfo::ToDataItem(TDataItem& item) const {
  TDataItem& ti_item = item.AddItem("TYPE");
  for( size_t i=0; i < TypeInfo.Count(); i++ )
    TypeInfo.GetValue(i).ToDataItem(ti_item.AddItem(TypeInfo.GetValue(i).atomType->symbol));
  TDataItem& ai_item = item.AddItem("ATOM");
  for( size_t i=0; i < AtomInfo.Count(); i++ ) 
    AtomInfo.GetValue(i).ToDataItem(ai_item.AddItem(AtomInfo.GetValue(i).atom->GetTag()));
}
//........................................................................
void ConnInfo::FromDataItem(const TDataItem& item)  {
  TDataItem& ti_item = item.FindRequiredItem("TYPE");
  for( size_t i=0; i < ti_item.ItemCount(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbol(ti_item.GetItem(i).GetName());
    if( elm == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown symbol: ") << ti_item.GetItem(i).GetName());
    TypeInfo.Add(elm).FromDataItem(ti_item.GetItem(i), elm);

  }
  TDataItem& ai_item = item.FindRequiredItem("ATOM");
  for( size_t i=0; i < ai_item.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom(ai_item.GetItem(i).GetName().ToInt());
    AtomInfo.Add(&ca).FromDataItem(ai_item.GetItem(i), rm, ca);
  }
}
#ifndef _NO_PYTHON
PyObject* ConnInfo::PyExport()  {
  PyObject* main = PyDict_New(),
    *type = PyDict_New(),
    *atom = PyDict_New();
  for( size_t i=0; i < TypeInfo.Count(); i++ )  {
    PythonExt::SetDictItem(type, TypeInfo.GetValue(i).atomType->symbol,
      TypeInfo.GetValue(i).PyExport());
  }
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    PythonExt::SetDictItem(atom, Py_BuildValue("i", AtomInfo.GetValue(i).atom->GetTag()),
      AtomInfo.GetValue(i).PyExport());
  }
  PythonExt::SetDictItem(main, "type", type);
  PythonExt::SetDictItem(main, "atom", atom);
  return main;
}
#endif
//........................................................................
//........................................................................
//........................................................................
void ConnInfo::TypeConnInfo::ToDataItem(TDataItem& item) const {
  item.AddField("r", r);
  item.AddField("b", maxBonds);
}
void ConnInfo::TypeConnInfo::FromDataItem(const TDataItem& item, const cm_Element* elm)  {
  atomType = elm;
  r = item.GetRequiredField("r").ToDouble();
  maxBonds = item.GetRequiredField("b").ToInt();
}
//........................................................................
#ifndef _NO_PYTHON
PyObject* ConnInfo::TypeConnInfo::PyExport()  {
  return Py_BuildValue("{s:d,s:i}", "radius", r, "bonds", maxBonds);
}
#endif
//........................................................................
size_t ConnInfo::FindBondIndex(const BondInfoList& list, TCAtom* key, TCAtom& a1, TCAtom& a2, const smatd* eqiv) {
  if( key != &a1 && key != &a2 || list.IsEmpty() )
    return InvalidIndex;
  if( key == &a1 )  {
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i].to == a2 )  {
        if( list[i].matr == NULL ) // this is special case - generic bond suppression... && eqiv == NULL )
          return i;
        if( list[i].matr != NULL && eqiv != NULL && *list[i].matr == *eqiv )  
          return i;
      }
    }
    return InvalidIndex;
  }
  else if( eqiv == NULL )  {
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i].to == a1 )
        return i;
    }
    return InvalidIndex;
  }
  else  {
    smatd mat = eqiv->Inverse();
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i].to == a1 && list[i].matr != NULL && *list[i].matr == mat )  
        return i;
    }
    return InvalidIndex;
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
    smatd mat(eqiv1->Inverse());
    if( release )  {
      rm.RemUsedSymm(*eqiv1);
      if( eqiv2 != NULL )
        rm.RemUsedSymm(*eqiv2);
    }
    return &rm.AddUsedSymm(mat);
  }
  //((*eqiv2)*eqiv1->Inverse());
  const TUnitCell& uc = rm.aunit.GetLattice().GetUnitCell();
  smatd mat = uc.MulMatrix(*eqiv2, uc.InvMatrix(*eqiv1));
  if( release )  {
    rm.RemUsedSymm(*eqiv1);
    rm.RemUsedSymm(*eqiv2);
  }
  return &rm.AddUsedSymm(mat);
}
//........................................................................
void ConnInfo::AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv)  {
  const smatd* eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);
  size_t ind = InvalidIndex;
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate, AtomInfo.GetKey(i), a1, a2, eqiv); 
    if( ind != InvalidIndex )
      break;
  }
  if( ind == InvalidIndex )  {
    // validate the bonds to delete
    bool exists = false;
    for( size_t i=0; i < AtomInfo.Count(); i++ )  {
      ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i), a1, a2, eqiv);
      if( ind != InvalidIndex )  {
        AtomInfo.GetValue(i).BondsToRemove.Delete(ind);
        exists = true;
        break;
      }
    }
    if( !exists )  {  // if was deleted - may be already exists?
      AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
      if( eqiv == NULL )  // these to be processed first
        ai.BondsToCreate.Insert(0, new CXBondInfo(a2, eqiv));
      else
        ai.BondsToCreate.Add(new CXBondInfo(a2, eqiv));
    }
  }
}
//........................................................................
void ConnInfo::RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv)  {
  const smatd* eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);
  size_t ind = InvalidIndex;
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i), a1, a2, eqiv);
    if( ind != InvalidIndex )
      break;
  }
  if( ind == InvalidIndex )  {
    // validate the bonds to create
    bool exists = false;
    for( size_t i=0; i < AtomInfo.Count(); i++ )  {
      ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate, AtomInfo.GetKey(i), a1, a2, eqiv);
      if( ind != InvalidIndex )  {
        AtomInfo.GetValue(i).BondsToCreate.Delete(ind);
        exists = true;
        break;
      }
    }
    if( !exists )  {  // if was added - then it might not exist?
      AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
      if( eqiv == NULL )  // these to be processed first
        ai.BondsToRemove.Insert(0, new CXBondInfo(a2, eqiv));
      else
        ai.BondsToRemove.Add(new CXBondInfo(a2, eqiv));
    }
  }
}
//........................................................................
void ConnInfo::Compile(const TCAtom& a, BondInfoList& toCreate, BondInfoList& toDelete, smatd_list& ml)  {
  const TAsymmUnit& au = *a.GetParent();
  const TUnitCell& uc = a.GetParent()->GetLattice().GetUnitCell();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() )  continue;
    CXConnInfo& ci = ca.GetConnInfo();
    if( ca != a )  {
      for( size_t j=0; j < ci.BondsToRemove.Count(); j++ )  {
        if( ci.BondsToRemove[j].to == a )  {
          if( ci.BondsToRemove[j].matr == NULL )
            toDelete.AddCCopy(ci.BondsToRemove[j]);
          else  {
            const smatd matr = uc.InvMatrix(*ci.BondsToRemove[j].matr);
            bool uniq = true;
            for( size_t k=0; k < toDelete.Count(); k++ )  {
              if( toDelete[k].matr != NULL && toDelete[k].to == a && toDelete[k].matr->GetId() == matr.GetId() )  {
                uniq = false;
                break;
              }
            }
            if( uniq )
              toDelete.Add(new CXBondInfo(ca, &ml.AddCCopy(matr)));
          }
        }
      }
      for( size_t j=0; j < ci.BondsToCreate.Count(); j++ )  {
        if( ci.BondsToCreate[j].to == a )  {
          if( ci.BondsToCreate[j].matr == NULL )
            toCreate.AddCCopy(ci.BondsToCreate[j]);
          else  {
            const smatd matr = uc.InvMatrix(*ci.BondsToCreate[j].matr);
            bool uniq = true;
            for( size_t k=0; k < toCreate.Count(); k++ )  {
              if( toCreate[k].matr != NULL && toCreate[k].to == a && toCreate[k].matr->GetId() == matr.GetId() )  {
                uniq = false;
                break;
              }
            }
            if( uniq )
              toCreate.Add(new CXBondInfo(ca, &ml.AddCCopy(matr)));
          }
        }
      }
    }
    else  {  // own connectivity
      for( size_t i=0; i < ci.BondsToCreate.Count(); i++ )
        toCreate.Add(new CXBondInfo(ci.BondsToCreate[i].to,
          ci.BondsToCreate[i].matr == NULL ? NULL : &ml.AddCCopy(*ci.BondsToCreate[i].matr)));
      for( size_t i=0; i < ci.BondsToRemove.Count(); i++ )
        toDelete.Add(new CXBondInfo(ci.BondsToRemove[i].to,
          ci.BondsToRemove[i].matr == NULL ? NULL : &ml.AddCCopy(*ci.BondsToRemove[i].matr)));
    }
  }
  //for( size_t i=0; i < ml.Count(); i++ )
  //  uc.InitMatrixId(ml[i]);
}
//........................................................................
void ConnInfo::AtomConnInfo::ToDataItem(TDataItem& item) const {
  item.AddField("r", r);
  item.AddField("b", maxBonds);
  TDataItem& ab = item.AddItem("ADDBOND");
  for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
    if( BondsToCreate[i].to.IsDeleted() )
      continue;
    TDataItem& bi = ab.AddItem("bi");
    bi.AddField("to", BondsToCreate[i].to.GetTag());
    if( BondsToCreate[i].matr != NULL )
      bi.AddField("eqiv", BondsToCreate[i].matr->GetId());
  }
  TDataItem& db = item.AddItem("DELBOND");
  for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
    if( BondsToRemove[i].to.IsDeleted() )
      continue;
    TDataItem& bi = ab.AddItem("bi");
    bi.AddField("to", BondsToRemove[i].to.GetTag());
    if( BondsToRemove[i].matr != NULL )
      bi.AddField("eqiv", BondsToRemove[i].matr->GetId());
  }
}
void ConnInfo::AtomConnInfo::FromDataItem(const TDataItem& item, RefinementModel& rm, TCAtom& a)  {
  atom = &a;
  r = item.GetRequiredField("r").ToDouble();
  maxBonds = item.GetRequiredField("b").ToInt();
  TDataItem& ab = item.FindRequiredItem("ADDBOND");
  for( size_t i=0; i < ab.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom( ab.GetItem(i).GetRequiredField("to").ToInt() );
    const olxstr& eq = ab.GetItem(i).GetFieldValue("eqiv");
    smatd const* eqiv = NULL;
    if( !eq.IsEmpty() )  { 
      eqiv = &rm.GetUsedSymm(eq.ToInt());
      rm.AddUsedSymm(*eqiv);  // persist
    }
    BondsToCreate.Add(new CXBondInfo(ca, eqiv));
  }
  TDataItem& db = item.FindRequiredItem("DELBOND");
  for( size_t i=0; i < db.ItemCount(); i++ )  {
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
#ifndef _NO_PYTHON
PyObject* ConnInfo::AtomConnInfo::PyExport()  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "radius", Py_BuildValue("f", r));
  PythonExt::SetDictItem(main, "max_bonds", Py_BuildValue("i", maxBonds));
  size_t bc = 0;
  for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
    if( BondsToCreate[i].to.IsDeleted() )  continue;
    bc++;
  }
  if( bc > 0 )  {
    PyObject* btc = PyTuple_New(bc);
    bc = 0;
    for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
      if( BondsToCreate[i].to.IsDeleted() )  continue;
      PyTuple_SetItem(btc, bc++,
        Py_BuildValue("{s:i,s:i}", "to", BondsToCreate[i].to.GetTag(), "eqiv",
          BondsToCreate[i].matr == NULL ? -1 : BondsToCreate[i].matr->GetId()));
    }
    PythonExt::SetDictItem(main, "create", btc);
  }
  bc = 0;
  for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
    if( BondsToRemove[i].to.IsDeleted() )  continue;
    bc++;
  }
  if( bc > 0 )  {
    PyObject* btd = PyTuple_New(bc);
    bc = 0;
    for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
      if( BondsToRemove[i].to.IsDeleted() )  continue;
      PyTuple_SetItem(btd, bc++,
        Py_BuildValue("{s:i,s:i}", "to", BondsToRemove[i].to.GetTag(), "eqiv",
          BondsToRemove[i].matr == NULL ? -1 : BondsToRemove[i].matr->GetId()));
    }
    PythonExt::SetDictItem(main, "delete", btd);
  }
  return main;
}
#endif
