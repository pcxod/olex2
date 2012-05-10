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

TSimpleRestraint::TSimpleRestraint(TSRestraintList& parent, size_t id,
  const short listType)
  : Parent(parent), Id(id), ListType(listType), Atoms(parent.GetRM())
{
  Value = 0;
  Esd = Esd1 = 0;
  AllNonHAtoms = false;
  VarRef = NULL;
}
//..............................................................................
void TSimpleRestraint::AddAtoms(const TCAtomGroup& atoms)  {
  for( size_t i=0; i < atoms.Count(); i++ )
    Atoms.AddExplicit(*atoms[i].GetAtom(), atoms[i].GetMatrix());
}
//..............................................................................
void TSimpleRestraint::AtomsFromExpression(const olxstr& e, const olxstr &resi)  {
  Atoms.Build(e, resi);
}
//..............................................................................
TSimpleRestraint &TSimpleRestraint::AddAtom(TCAtom& aa, const smatd* ma)  {
  if( aa.GetParent() != &Parent.GetRM().aunit ) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "mismatching asymmetric unit");
  }
  Atoms.AddExplicit(aa, ma);
  return *this;
}
//..............................................................................
void TSimpleRestraint::Delete()  {
  Atoms.Clear();
}
//..............................................................................
TSimpleRestraint &TSimpleRestraint::Validate() {
  Atoms.Validate();
  return *this;
}
//..............................................................................
void TSimpleRestraint::Assign(const TSimpleRestraint& sr)  {
  Atoms.Assign(sr.Atoms);
  ListType = sr.GetListType();
  Value = sr.Value;
  Esd = sr.Esd;
  Esd1 = sr.Esd1;
  AllNonHAtoms = sr.AllNonHAtoms;
}
//..............................................................................
void TSimpleRestraint::ToDataItem(TDataItem& item) const {
  item.AddField("allNonH", AllNonHAtoms);
  item.AddField("esd", Esd);
  item.AddField("esd1", Esd1);
  item.AddField("val", Value);
  Atoms.ToDataItem(item.AddItem("AtomList"));
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSimpleRestraint::PyExport(TPtrList<PyObject>& atoms,
  TPtrList<PyObject>& equiv)
{
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "allNonH", Py_BuildValue("b", AllNonHAtoms));
  PythonExt::SetDictItem(main, "esd1", Py_BuildValue("d", Esd));
  PythonExt::SetDictItem(main, "esd2", Py_BuildValue("d", Esd1));
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));
  TTypeList<ExplicitCAtomRef> ats = Atoms.ExpandList(Parent.GetRM());
  PyObject* involved = PyTuple_New(ats.Count());
  for( size_t i=0; i < ats.Count(); i++ )  {
    PyObject* eq;
    if( ats[i].GetMatrix() == NULL )
      eq = Py_None;
    else
      eq = equiv[ats[i].GetMatrix()->GetId()];
    Py_INCREF(eq);
    PyTuple_SetItem(involved, i, 
      Py_BuildValue("OO", Py_BuildValue("i", ats[i].GetAtom().GetTag()), eq));
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
  TDataItem* atoms = item.FindItem("atoms");
  if ( atoms != NULL) {
    for( size_t i=0; i < atoms->ItemCount(); i++ )  {
      TDataItem& ai = atoms->GetItem(i);
      size_t aid = ai.GetRequiredField("atom_id").ToSizeT();
      uint32_t eid = ai.GetRequiredField("eqiv_id").ToUInt();
      AddAtom(Parent.GetRM().aunit.GetAtom(aid),
        olx_is_valid_index(eid) ? &Parent.GetRM().GetUsedSymm(eid) : NULL);
    }
  }
  else {
    Atoms.FromDataItem(item.FindRequiredItem("AtomList"));
  }
}
//..............................................................................
IXVarReferencerContainer& TSimpleRestraint::GetParentContainer() const
{
  return Parent;
}
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
TSimpleRestraint& TSRestraintList::Release(size_t i)  {
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
    if( !Restraints[i].IsAllNonHAtoms() || Restraints[i].Validate().IsEmpty() )
      continue;
    Restraints[i].ToDataItem(item.AddItem(rs_id++));
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSRestraintList::PyExport(TPtrList<PyObject>& atoms,
  TPtrList<PyObject>& equiv)
{
  size_t rs_cnt = 0;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() || Restraints[i].Validate().IsEmpty() )
      continue;
    rs_cnt++;
  }

  PyObject* main = PyTuple_New( rs_cnt );
  rs_cnt = 0;
  for( size_t i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() || Restraints[i].Validate().IsEmpty() )
      continue;
    PyTuple_SetItem(main, rs_cnt++, Restraints[i].PyExport(atoms, equiv));
  }
  return main;
}
#endif
//..............................................................................
void TSRestraintList::FromDataItem(const TDataItem& item) {
  for( size_t i=0; i < item.ItemCount(); i++ )
    AddNew().FromDataItem(item.GetItem(i));
}
//..............................................................................
TSimpleRestraint& TSRestraintList::AddNew()  {
  TSimpleRestraint& r = Restraints.Add(
    new TSimpleRestraint(*this, Restraints.Count(), RestraintListType));
  return RefMod.SetRestraintDefaults(r);
}
//..............................................................................
