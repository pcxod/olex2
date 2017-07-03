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
  short listType)
  : Parent(parent), Id(id), ListType(listType), Atoms(parent.GetRM())
{
  Value = 0;
  Esd = Esd1 = 0;
  AllNonHAtoms = false;
  VarRef = 0;
}
//..............................................................................
void TSimpleRestraint::AddAtoms(const TCAtomGroup& atoms) {
  for (size_t i = 0; i < atoms.Count(); i++) {
    Atoms.AddExplicit(*atoms[i].GetAtom(), atoms[i].GetMatrix());
  }
}
//..............................................................................
void TSimpleRestraint::AtomsFromExpression(const olxstr& e, const olxstr &resi) {
  Atoms.Build(e, resi);
}
//..............................................................................
TSimpleRestraint &TSimpleRestraint::AddAtom(TCAtom& aa, const smatd* ma) {
  if (aa.GetParent() != &Parent.GetRM().aunit) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "mismatching asymmetric unit");
  }
  if (ma != 0) {
    for (size_t i = 0; i < aa.EquivCount(); i++) {
      if (aa.GetEquiv(i).GetId() == ma->GetId()) {
        ma = 0;
        break;
      }
    }
  }
  Atoms.AddExplicit(aa, ma);
  return *this;
}
//..............................................................................
void TSimpleRestraint::Delete() {
  Atoms.Clear();
}
//..............................................................................
TSimpleRestraint &TSimpleRestraint::Validate() {
  size_t gc = InvalidIndex;
  switch (ListType) {
  case rltGroup2: gc = 2; break;
  case rltGroup3: gc = 3; break;
  case rltGroup4: gc = 4; break;
  }
  Atoms.Validate(gc);
  return *this;
}
//..............................................................................
void TSimpleRestraint::Assign(const TSimpleRestraint& sr) {
  Atoms.Assign(sr.Atoms);
  ListType = sr.GetListType();
  Value = sr.Value;
  Esd = sr.Esd;
  Esd1 = sr.Esd1;
  AllNonHAtoms = sr.AllNonHAtoms;
}
//..............................................................................
void TSimpleRestraint::EndAUSort() {
  Atoms.EndAUSort(ListType == rltAtoms && !Parent.GetIdName().Equalsi("RIGU"));
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
#ifdef _PYTHON
ConstPtrList<PyObject> TSimpleRestraint::PyExport(TPtrList<PyObject>& atoms,
  TPtrList<PyObject>& equiv)
{
  TTypeList<TAtomRefList> ats = Atoms.Expand(Parent.GetRM());
  TPtrList<PyObject> rv(ats.Count());
  for (size_t i=0; i < ats.Count(); i++) {
    rv[i] = PyDict_New();
    PythonExt::SetDictItem(rv[i], "allNonH", Py_BuildValue("b", AllNonHAtoms));
    PythonExt::SetDictItem(rv[i], "esd1", Py_BuildValue("d", Esd));
    PythonExt::SetDictItem(rv[i], "esd2", Py_BuildValue("d", Esd1));
    PythonExt::SetDictItem(rv[i], "value", Py_BuildValue("d", Value));
    PyObject* involved = PyTuple_New(ats[i].Count());
    for( size_t j=0; j < ats[i].Count(); j++ )  {
      PyObject* eq;
      if (ats[i][j].GetMatrix() == 0) {
        eq = Py_None;
      }
      else {
        eq = equiv[ats[i][j].GetMatrix()->GetId()];
      }
      Py_INCREF(eq);
      PyTuple_SetItem(involved, j,
        Py_BuildValue("OO", Py_BuildValue("i", ats[i][j].GetAtom().GetTag()), eq));
    }
    PythonExt::SetDictItem(rv[i], "atoms", involved);
  }
  return rv;
}
#endif
//..............................................................................
void TSimpleRestraint::FromDataItem(const TDataItem& item) {
  AllNonHAtoms = item.GetFieldByName("allNonH").ToBool();
  Esd = item.GetFieldByName("esd").ToDouble();
  Esd1 = item.GetFieldByName("esd1").ToDouble();
  Value = item.GetFieldByName("val").ToDouble();
  TDataItem* atoms = item.FindItem("atoms");
  if (atoms != 0) {
    for( size_t i=0; i < atoms->ItemCount(); i++ )  {
      TDataItem& ai = atoms->GetItemByIndex(i);
      size_t aid = ai.GetFieldByName("atom_id").ToSizeT();
      uint32_t eid = ai.GetFieldByName("eqiv_id").ToUInt();
      AddAtom(Parent.GetRM().aunit.GetAtom(aid),
        olx_is_valid_index(eid) ? &Parent.GetRM().GetUsedSymm(eid) : 0);
    }
  }
  else {
    Atoms.FromDataItem(item.GetItemByName("AtomList"));
  }
}
//..............................................................................
IXVarReferencerContainer& TSimpleRestraint::GetParentContainer() const {
  return Parent;
}
//..............................................................................
olxstr TSimpleRestraint::GetIdName() const {
  return Parent.GetIdName();
}
//..............................................................................
olxstr TSimpleRestraint::GetVarName(size_t var_index) const {
  const static olxstr vm("1");
  if (var_index != 0) {
    throw TInvalidArgumentException(__OlxSourceInfo, "var index");
  }
  return vm;
}
//..............................................................................
TIString TSimpleRestraint::ToString() const {
  olxstr rv = Parent.GetIdName();
  if (!Atoms.GetResi().IsEmpty()) {
    rv << '_' << Atoms.GetResi();
  }
  if ((Parent.GetParameters() & rptValue) != 0) {
    rv << ' ' << Value;
  }
  if ((Parent.GetParameters() & rptEsd) != 0) {
    rv << ' ' << Esd;
  }
  if ((Parent.GetParameters() & rptEsd1) != 0) {
    rv << ' ' << Esd1;
  }
  if ((Parent.GetParameters() & rptValue1) != 0) {
    rv << ' ' << Value;
  }
  return (rv << ' ' << Atoms.GetExpression());
}
//..............................................................................
void TSimpleRestraint::OnAUUpdate() {
  Atoms.OnAUUpdate();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TSRestraintList::Assign(const TSRestraintList& rl)  {
  if (rl.GetRestraintListType() != RestraintListType) {
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  }

  Clear();
  for (size_t i=0; i < rl.Count(); i++) {
    AddNew().Assign(rl.Restraints[i]);
  }
}
//..............................................................................
void TSRestraintList::ValidateRestraint(TSimpleRestraint& sr)  {

  if (sr.GetListType() != RestraintListType) {
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  }
  // check if there is a restraint for all non-H atoms and locate the sr
  size_t ri = InvalidIndex;
  {
    size_t AllAtomsInd = InvalidIndex;
    for (size_t i = 0; i < Restraints.Count(); i++) {
      if (Restraints[i].IsAllNonHAtoms()) {
        AllAtomsInd = i;
      }
      if (&Restraints[i] == &sr) {
        ri = i;
      }
      if (AllAtomsInd != InvalidIndex && ri != InvalidIndex) {
        break;
      }
    }
    if (AllAtomsInd != InvalidIndex && Restraints.Count() > 1) {
      for (size_t i = 0; i < Restraints.Count(); i++) {
        if (i != AllAtomsInd) {
          Restraints[i].Delete();
        }
      }
      TBasicApp::NewLogEntry(logWarning) <<
        "There is a restraint for all non-H atoms, removing any others for "
        << GetIdName() << " list";
      return;
    }
  }
  // check uniqueness
  {
    olxstr exp = olxstr(sr.GetAtoms().GetResi()) << ": "
      << sr.GetAtomExpression();
    size_t dc = 0;
    for (size_t i = 0; i < Restraints.Count(); i++) {
      if (i == ri) {
        continue;
      }
      olxstr exp1 = Restraints[i].GetAtoms().GetResi();
      exp1 << ": " << Restraints[i].GetAtomExpression();
      if (exp1 == exp) {
        Restraints[i].Delete();
        dc++;
      }
    }
    if (dc != 0) {
      TBasicApp::NewLogEntry(logWarning) << "Duplicate restraints removed in "
        << GetIdName() << " list: " << exp;
    }
  }
}
//..............................................................................
void TSRestraintList::Clear() {
  for (size_t i = 0; i < Restraints.Count(); i++) {
    if (Restraints[i].GetVarRef(0) != 0) {
      delete RefMod.Vars.ReleaseRef(Restraints[i], 0);
    }
  }
  Restraints.Clear();
}
//..............................................................................
TSimpleRestraint& TSRestraintList::Release(size_t i) {
  if (Restraints[i].GetVarRef(0) != 0) {
    RefMod.Vars.ReleaseRef(Restraints[i], 0);
  }
  return Restraints.Release(i);
}
//..............................................................................
void TSRestraintList::Restore(TSimpleRestraint& sr)  {
  if (&sr.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "restraint parent differs");
  }
  Restraints.Add(sr);
  if (sr.GetVarRef(0) != 0) {
    RefMod.Vars.RestoreRef(sr, 0, sr.GetVarRef(0));
  }
}
//..............................................................................
void TSRestraintList::Release(TSimpleRestraint& sr)  {
  size_t ind = Restraints.IndexOf(sr);
  if (ind == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "restraint");
  }
  Release(ind);
}
//..............................................................................
void TSRestraintList::ToDataItem(TDataItem& item) const {
  size_t rs_id = 0;
  for (size_t i = 0; i < Restraints.Count(); i++) {
    if (!Restraints[i].IsAllNonHAtoms() && Restraints[i].Validate().IsEmpty()) {
      continue;
    }
    Restraints[i].ToDataItem(item.AddItem("item"));
  }
}
//..............................................................................
#ifdef _PYTHON
PyObject* TSRestraintList::PyExport(TPtrList<PyObject>& atoms,
  TPtrList<PyObject>& equiv)
{
  TPtrList<PyObject> all;
  for (size_t i = 0; i < Restraints.Count(); i++) {
    if (!Restraints[i].IsAllNonHAtoms() && Restraints[i].Validate().IsEmpty()) {
      continue;
    }
    all << Restraints[i].PyExport(atoms, equiv);
  }
  PyObject* main = PyTuple_New(all.Count());
  for (size_t i = 0; i < all.Count(); i++) {
    PyTuple_SetItem(main, i, all[i]);
  }
  return main;
}
#endif
//..............................................................................
void TSRestraintList::FromDataItem(const TDataItem& item) {
  for (size_t i=0; i < item.ItemCount(); i++) {
    try {
      AddNew().FromDataItem(item.GetItemByIndex(i));
    }
    catch (const TExceptionBase &) {
      TBasicApp::NewLogEntry() << "One of the " << GetIdName() << " was not "
        "saved correctly and was removed";
      Restraints.Delete(Restraints.Count()-1);
    }
  }
}
//..............................................................................
TSimpleRestraint& TSRestraintList::AddNew()  {
  TSimpleRestraint& r = Restraints.Add(
    new TSimpleRestraint(*this, Restraints.Count(), RestraintListType));
  return RefMod.SetRestraintDefaults(r);
}
//..............................................................................
void TSRestraintList::OnAUUpdate() {
  if (!AllowSymm) {
    return;
  }
  for (size_t i = 0; i < Restraints.Count(); i++) {
    Restraints[i].OnAUUpdate();
  }
}
//..............................................................................
void TSRestraintList::BeginAUSort() {
  for (size_t i = 0; i < Restraints.Count(); i++) {
    Restraints[i].BeginAUSort();
  }
}
//..............................................................................
void TSRestraintList::EndAUSort() {
  for (size_t i = 0; i < Restraints.Count(); i++) {
    Restraints[i].EndAUSort();
  }
}
//..............................................................................
void TSRestraintList::SortAtomsByTags() {
  for (size_t i = 0; i < Restraints.Count(); i++) {
    Restraints[i].Sort();
  }
}
//..............................................................................
void TSRestraintList::UpdateResi() {
  for (size_t i = 0; i < Restraints.Count(); i++) {
    Restraints[i].UpdateResi();
  }
}
//..............................................................................
