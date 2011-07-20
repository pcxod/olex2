/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "srestraint.h"
#include "refmodel.h"

TSimpleRestraint::TSimpleRestraint(TSRestraintList& parent, size_t id, const short listType) : 
  Parent(parent), Id(id), ListType(listType)
{
  Value = 0;
  Esd = Esd1 = 0;
  AllNonHAtoms = false;
  VarRef = NULL;
}
//..............................................................................
void TSimpleRestraint::Clear()  {  InvolvedAtoms.Clear();  }
//..............................................................................
void TSimpleRestraint::AddAtoms(const TCAtomGroup& atoms)  {
  InvolvedAtoms.SetCapacity(InvolvedAtoms.Count() + atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )
    InvolvedAtoms.AddNew(atoms[i].GetAtom(), atoms[i].GetMatrix());
}
//..............................................................................
TSimpleRestraint &TSimpleRestraint::AddAtom(TCAtom& aa, const smatd* ma)  {
  if( aa.GetParent() != &Parent.GetRM().aunit  )
    throw TInvalidArgumentException(__OlxSourceInfo, "mismatching asymmetric unit");
  const smatd* tm = NULL;
  if( ma != NULL )  {
    if( !ma->r.IsI() || ma->t.QLength() != 0 ) 
      tm = &Parent.GetRM().AddUsedSymm(*ma);
  }
  InvolvedAtoms.AddNew(&aa, tm);
  return *this;
}
//..............................................................................
void TSimpleRestraint::Validate()  {
  size_t group_cnt = 1;      
  if( ListType == rltGroup2 )
    group_cnt = 2;
  else if( ListType == rltGroup3 )
    group_cnt = 3;
  else if( ListType == rltGroup4 )
    group_cnt = 4;
  else if( ListType == rltGroup )
    group_cnt = InvolvedAtoms.Count();
  
  for( size_t i=0; i < InvolvedAtoms.Count(); i+=group_cnt )  {
    bool valid = true;
    for( size_t j=i; j < group_cnt; j++ )  {
      if( j >= InvolvedAtoms.Count() || InvolvedAtoms.IsNull(j) ||
          InvolvedAtoms[j].GetAtom()->IsDeleted() )
      {
        valid = false;
        break;
      }
    }
    if( !valid )  {
      for( size_t j=i; j < group_cnt; j++ )  {
        if( j >= InvolvedAtoms.Count() )  break;
        if( InvolvedAtoms.IsNull(j) )  continue;
        if( InvolvedAtoms[j].GetMatrix() != NULL )
          Parent.GetRM().RemUsedSymm(*InvolvedAtoms[j].GetMatrix());
        InvolvedAtoms.NullItem(j);
      }
    }
  }
  InvolvedAtoms.Pack();
}
//..............................................................................
//void TSimpleRestraint::RemoveAtom(int i)  {
//  if( InvolvedAtoms[i].GetMatrix() != NULL )
//    Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
//  InvolvedAtoms.Delete(i);
//}
//..............................................................................
void TSimpleRestraint::Delete()  {
  for( size_t i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetMatrix() != NULL )
      Parent.GetRM().RemUsedSymm(*InvolvedAtoms[i].GetMatrix());
  }
  InvolvedAtoms.Clear();
}
//..............................................................................
/*
const TSimpleRestraint& TSimpleRestraint::operator = ( const TSimpleRestraint& sr)  {
  Clear();
  for( size_t i=0; i < sr.InvolvedAtoms.Count(); i+=2 )  {
    AddAtomPair( sr.InvolvedAtoms[i].GetAtom(), sr.InvolvedAtoms[i].GetMatrix(),
                 sr.InvolvedAtoms[i+1].GetAtom(), sr.InvolvedAtoms[i+1].GetMatrix() );
  }
  Value = sr.Value;
  Esd = sr.Esd;
  return sr;
}
*/
//..............................................................................
void TSimpleRestraint::Assign(const TSimpleRestraint& sr)  {
  Clear();
  ListType = sr.GetListType();
  Value = sr.Value;
  Esd = sr.Esd;
  Esd1 = sr.Esd1;
  AllNonHAtoms = sr.AllNonHAtoms;

  if( sr.AtomCount() == 0 )  return;

  TAsymmUnit& au = sr.Parent.GetRM().aunit;
  TAsymmUnit& tau = Parent.GetRM().aunit;

  if( &au == &tau )  {
    for( size_t i=0; i < sr.InvolvedAtoms.Count(); i++ )
      AddAtom(sr.InvolvedAtoms[i]);
  }
  else  {
    for( size_t i=0; i < sr.InvolvedAtoms.Count(); i++ )  {
      TCAtom* aa = tau.FindCAtomById(sr.InvolvedAtoms[i].GetAtom()->GetId());
      if( aa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
      AddAtom(*aa, sr.InvolvedAtoms[i].GetMatrix());
    }
  }
}
//..............................................................................
void TSimpleRestraint::Subtract(TSimpleRestraint& sr)  {
  if( sr.GetListType() != ListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  if( ListType == rltAtoms )  { // do not subtract groups though: #312..
    size_t del_count = 0;
    for( size_t i=0; i < InvolvedAtoms.Count(); i++ )  {
      for( size_t j=0; j < sr.InvolvedAtoms.Count(); j++ )  {
        if( AtomsEqual(InvolvedAtoms[i], sr.InvolvedAtoms[j]) )  {
          if( InvolvedAtoms[i].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm(*InvolvedAtoms[i].GetMatrix());
          InvolvedAtoms.Delete(i--);
          del_count++;
          break;
        }
      }
    }
    if( ListType == rltAtoms && del_count > 0 )  {  // should merge then... #199
      for( size_t i=0; i < InvolvedAtoms.Count(); i++ )
        sr.AddAtom(*InvolvedAtoms[i].GetAtom(), InvolvedAtoms[i].GetMatrix());
      Delete();
    }
  }
  else if( ListType == rltGroup2 )  {
    for( size_t i=0; i < InvolvedAtoms.Count(); i+=2 )  {
      for( size_t j=0; j < sr.InvolvedAtoms.Count(); j+=2 )  {
        if( (AtomsEqual(InvolvedAtoms[i], sr.InvolvedAtoms[j]) &&
             AtomsEqual(InvolvedAtoms[i+1], sr.InvolvedAtoms[j+1])) ||
            (AtomsEqual(InvolvedAtoms[i], sr.InvolvedAtoms[j+1]) &&
             AtomsEqual(InvolvedAtoms[i+1], sr.InvolvedAtoms[j])) )
        {
          if( InvolvedAtoms[i].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm(*InvolvedAtoms[i].GetMatrix());
          if( InvolvedAtoms[i+1].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm(*InvolvedAtoms[i+1].GetMatrix());
          InvolvedAtoms.Delete(i);
          InvolvedAtoms.Delete(i);
          i -= 2;
          break;
        }
      }
    }
  }
}
//..............................................................................
void TSimpleRestraint::OnCAtomCrdChange(TCAtom* ca, const smatd& matr)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TSimpleRestraint::ContainsAtom(TCAtom& ca) const {
  for( size_t i=0; i < InvolvedAtoms.Count(); i++ )
    if( InvolvedAtoms[i].GetAtom() == &ca )
      return true;
  return false;
}
//..............................................................................
void TSimpleRestraint::ToDataItem(TDataItem& item) const {
  item.AddField("allNonH", AllNonHAtoms);
  item.AddField("esd", Esd);
  item.AddField("esd1", Esd1);
  item.AddField("val", Value);
  TDataItem& atoms = item.AddItem("atoms");
  size_t atom_id=0;
  for( size_t i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    TDataItem& atom = atoms.AddItem(atom_id++);
    atom.AddField("atom_id", InvolvedAtoms[i].GetAtom()->GetTag());
    atom.AddField("eqiv_id",
      InvolvedAtoms[i].GetMatrix() == NULL ? InvalidIndex : InvolvedAtoms[i].GetMatrix()->GetId());
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSimpleRestraint::PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv)  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "allNonH", Py_BuildValue("b", AllNonHAtoms));
  PythonExt::SetDictItem(main, "esd1", Py_BuildValue("d", Esd));
  PythonExt::SetDictItem(main, "esd2", Py_BuildValue("d", Esd1));
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));

  size_t atom_cnt=0;
  for( size_t i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    atom_cnt++;
  }

  PyObject* involved = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for( size_t i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    PyObject* eq;
    if( InvolvedAtoms[i].GetMatrix() == NULL )
      eq = Py_None;
    else
      eq = equiv[InvolvedAtoms[i].GetMatrix()->GetId()];
    Py_INCREF(eq);
    PyTuple_SetItem(involved, atom_cnt++, 
      Py_BuildValue("OO", Py_BuildValue("i", InvolvedAtoms[i].GetAtom()->GetTag()), eq));
  }
  PythonExt::SetDictItem(main, "atoms", involved);
  return main;
}
#endif//..............................................................................
void TSimpleRestraint::FromDataItem(const TDataItem& item) {
  AllNonHAtoms = item.GetRequiredField("allNonH").ToBool();
  Esd = item.GetRequiredField("esd").ToDouble();
  Esd1 = item.GetRequiredField("esd1").ToDouble();
  Value = item.GetRequiredField("val").ToDouble();
  TDataItem& atoms = item.FindRequiredItem("atoms");
  for( size_t i=0; i < atoms.ItemCount(); i++ )  {
    TDataItem& ai = atoms.GetItem(i);
    size_t aid = ai.GetRequiredField("atom_id").ToSizeT();
    uint32_t eid = ai.GetRequiredField("eqiv_id").ToUInt();
    AddAtom(Parent.GetRM().aunit.GetAtom(aid), olx_is_valid_index(eid) ? &Parent.GetRM().GetUsedSymm(eid) : NULL);
  }
}
//..............................................................................
IXVarReferencerContainer& TSimpleRestraint::GetParentContainer() const {  return Parent;  }
//..............................................................................
olxstr TSimpleRestraint::GetIdName() const {  return Parent.GetIdName();  }
//..............................................................................
olxstr TSimpleRestraint::GetVarName(size_t var_index) const {  
  const static olxstr vm("1");
  if( var_index != 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "var index");
  return vm;  
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
void TSRestraintList::Assign(const TSRestraintList& rl)  {
  if( rl.GetRestraintListType() != RestraintListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");

  Clear();
  for( size_t i=0; i < rl.Count(); i++)  {
    AddNew().Assign(rl.Restraints[i]);
  }
}
//..............................................................................
void TSRestraintList::ValidateRestraint(TSimpleRestraint& sr)  {
  if( sr.GetListType() != RestraintListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  size_t AllAtomsInd = InvalidIndex;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( Restraints[i].IsAllNonHAtoms() )  {
      AllAtomsInd = i;
      break;
    }
  }
  if( AllAtomsInd != InvalidIndex )  {
    for( size_t i=0; i < Restraints.Count(); i++ )
      if( i != AllAtomsInd )
        Restraints[i].Delete();
  }
  else  {
    for( size_t i=0; i < Restraints.Count(); i++ )  {
      if( &Restraints[i] == &sr )
        continue;
      Restraints[i].Subtract(sr);
    }
  }
}
//..............................................................................
void TSRestraintList::Clear()  {  
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( Restraints[i].GetVarRef(0) != NULL )
      delete RefMod.Vars.ReleaseRef(Restraints[i], 0);
  }
  Restraints.Clear();  
}
//..............................................................................
TSimpleRestraint& TSRestraintList::Release(size_t i)    {  
  if( Restraints[i].GetVarRef(0) != NULL )
    RefMod.Vars.ReleaseRef(Restraints[i], 0);
  return Restraints.Release(i);  
}
//..............................................................................
void TSRestraintList::Restore(TSimpleRestraint& sr)  {  
  if( &sr.GetParent() != this )
    throw TInvalidArgumentException(__OlxSourceInfo, "restraint parent differs");
  Restraints.Add(sr);  
  if( sr.GetVarRef(0) != NULL )
    RefMod.Vars.RestoreRef(sr, 0, sr.GetVarRef(0));
}
//..............................................................................
void TSRestraintList::Release(TSimpleRestraint& sr)  {
  size_t ind = Restraints.IndexOf(sr);
  if( ind == InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "restraint");
  Release(ind);
}
//..............................................................................
void TSRestraintList::ToDataItem(TDataItem& item) const {
  size_t rs_id = 0;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    Restraints[i].ToDataItem(item.AddItem(rs_id++));
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSRestraintList::PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv)  {
  size_t rs_cnt = 0;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    rs_cnt++;
  }

  PyObject* main = PyTuple_New( rs_cnt );
  rs_cnt = 0;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    PyTuple_SetItem(main, rs_cnt++, Restraints[i].PyExport(atoms, equiv) );
  }
  return main;
}
#endif//..............................................................................
void TSRestraintList::FromDataItem(const TDataItem& item) {
  for( size_t i=0; i < item.ItemCount(); i++ )
    AddNew().FromDataItem(item.GetItem(i));
}
//..............................................................................
