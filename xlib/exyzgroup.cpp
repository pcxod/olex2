/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exyzgroup.h"
#include "refmodel.h"
#include "index_range.h"

void TExyzGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TExyzGroup::Assign(const TExyzGroup& ag) {
  for (size_t i = 0; i < ag.Atoms.Count(); i++) {
    Atoms.Add(Parent.RM.aunit.FindCAtomById(ag.Atoms[i]->GetId()));
    if (Atoms.GetLast() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    }
    Atoms.GetLast()->SetExyzGroup(this);
  }
}
//..............................................................................
olxstr TExyzGroup::ToString() const {
  AtomRefList atoms(Parent.RM);
  for (size_t i = 0; i < Atoms.Count(); i++) {
    if (Atoms[i]->IsDeleted()) {
      continue;
    }
    atoms.AddExplicit(*Atoms[i]);
  }
  if (atoms.IsEmpty()) {
    return EmptyString();
  }
  atoms.UpdateResi();
  olxstr_buf rv = olxstr("EXYZ");
  if (!atoms.GetResi().IsEmpty()) {
    rv << '_' << atoms.GetResi();
  }
  return olxstr(rv << ' ' << atoms.GetExpression());
}
//..............................................................................
void TExyzGroup::ToDataItem(TDataItem& item) const {
  IndexRange::Builder rb;
  for (size_t i=0; i < Atoms.Count(); i++) {
    if (!Atoms[i]->IsDeleted()) {
      rb << Atoms[i]->GetTag();
    }
  }
  item.AddField("atom_range", rb.GetString());
}
//..............................................................................
#ifdef _PYTHON
PyObject* TExyzGroup::PyExport(TPtrList<PyObject>& atoms) {
  int atom_cnt = 0;
  for (size_t i = 0; i < Atoms.Count(); i++) {
    if (!Atoms[i]->IsDeleted()) {
      atom_cnt++;
    }
  }
  PyObject* main = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for (size_t i = 0; i < Atoms.Count(); i++) {
    if (!Atoms[i]->IsDeleted()) {
      PyTuple_SetItem(main, atom_cnt++, Py_BuildValue("i", Atoms[i]->GetTag()));
    }
  }
  return main;
}
#endif
//..............................................................................
void TExyzGroup::FromDataItem(TDataItem& item) {
  size_t ai = item.FieldIndex("atom_range");
  if(ai != InvalidIndex) {
    IndexRange::RangeItr i = item.GetFieldByIndex(ai);
    while (i.HasNext()) {
      Atoms.Add(Parent.RM.aunit.GetAtom(i.Next()))->SetExyzGroup(this);
    }
  }
  else {
    TStrStrList fields = item.GetOrderedFieldList();
    for (size_t i = 0; i < fields.Count(); i++) {
      Atoms.Add(Parent.RM.aunit.GetAtom(fields.GetObject(i).ToSizeT()));
      Atoms.GetLast()->SetExyzGroup(this);
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TExyzGroups::ToDataItem(TDataItem& item) {
  size_t group_id = 0;
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsEmpty()) {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();
  item.AddField("n", Groups.Count());
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].ToDataItem(item.AddItem("group"));
  }
}
//..............................................................................
#ifdef _PYTHON
PyObject* TExyzGroups::PyExport(TPtrList<PyObject>& atoms) {
  size_t group_id = 0;
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsEmpty()) {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();

  PyObject* main = PyTuple_New(Groups.Count());
  for (size_t i = 0; i < Groups.Count(); i++) {
    PyTuple_SetItem(main, i, Groups[i].PyExport(atoms));
  }
  return main;
}
#endif
//..............................................................................
void TExyzGroups::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetFieldByName("n").ToSizeT();
  if (n != item.ItemCount()) {
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  }
  for (size_t i=0; i < n; i++) {
    New().FromDataItem(item.GetItemByIndex(i));
  }
}
//..............................................................................
