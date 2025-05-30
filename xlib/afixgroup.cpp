/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "afixgroup.h"
#include "refmodel.h"
#include "index_range.h"

const olxstr TAfixGroup::m_names[] = {
  EmptyString(),
  "ternary CH",
  "secondary CH2",
  "Me",
  "aromatic/amide H",
  "fitted pentagon",
  "fitted hexagon",
  "fitted hexagon",
  "tetrahedral OH",
  "X=CH2",
  "Cp*",
  "naphthalene",
  "disordered Me",
  "idealised Me",
  "idealised tetrahedral OH",
  "boron cage BH",
  "acetylenic CH",
};
const olxstr TAfixGroup::n_names[] = {
  EmptyString(),
  "with everything fixed",
  "with fixed occupancy and ADP/Uiso",
  "with riding coordinates",
  "with riding coordinates and stretchable bonds",
  "is dependent atom of rigid group",
  "as free rotating group",
  "as rotating group",
  "as rotating group with stretchable bonds",
  "as free rotating sizeable group",
};
//..............................................................................
TAfixGroup::~TAfixGroup() {
  for (size_t i = 0; i < Dependent.Count(); i++) {
    Dependent[i]->SetParentAfixGroup(0);
  }
  Dependent.Clear();
  if (Pivot == 0) {
    return;
  }
  if (HasExcplicitPivot() || IsUnbound()) {
    Pivot->SetDependentAfixGroup(0);
  }
  // might happen at several places
  else if (Pivot->DependentHfixGroupCount() != 0) {
    Pivot->RemoveDependentHfixGroup(*this);
  }
}
void TAfixGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TAfixGroup::Assign(const TAfixGroup& ag) {
  D = ag.D;
  Sof = ag.Sof;
  U = ag.U;
  Afix = ag.Afix;

  Pivot = Parent.RM.aunit.FindCAtomById(ag.Pivot->GetId());
  if (Pivot == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  }
  SetPivot(*Pivot);
  for (size_t i = 0; i < ag.Dependent.Count(); i++) {
    Dependent.Add(Parent.RM.aunit.FindCAtomById(ag.Dependent[i]->GetId()));
    if (Dependent.GetLast() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    }
    Dependent.GetLast()->SetParentAfixGroup(this);
  }
}
//..............................................................................
void TAfixGroup::ToDataItem(TDataItem& item) const {
  item.AddField("afix", Afix);
  item.AddField("d", D);
  item.AddField("u", U);
  item.AddField("pivot_atom_id", Pivot->GetTag());
  IndexRange::Builder rb;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (!Dependent[i]->IsDeleted()) {
      rb << Dependent[i]->GetTag();
    }
  }
  item.AddItem("dependent_range", rb.GetString());
}
//..............................................................................
#ifdef _PYTHON
PyObject* TAfixGroup::PyExport(TPtrList<PyObject>& atoms) {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "afix", Py_BuildValue("i", Afix));
  PythonExt::SetDictItem(main, "u", Py_BuildValue("d", U));
  PythonExt::SetDictItem(main, "d", Py_BuildValue("d", D));
  PythonExt::SetDictItem(main, "pivot", Py_BuildValue("i", Pivot->GetTag()));
  size_t dep_cnt = 0;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (Dependent[i]->IsDeleted()) {
      continue;
    }
    dep_cnt++;
  }
  PyObject* dependent = PyTuple_New(dep_cnt);
  dep_cnt = 0;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (Dependent[i]->IsDeleted()) {
      continue;
    }
    PyTuple_SetItem(dependent, dep_cnt++,
      Py_BuildValue("i", Dependent[i]->GetTag()));
  }
  PythonExt::SetDictItem(main, "dependent", dependent);
  return main;
}
#endif
//..............................................................................
void TAfixGroup::FromDataItem(TDataItem& item) {
  Afix = item.GetFieldByName("afix").ToInt();
  D = item.GetFieldByName("d").ToDouble();
  U = item.GetFieldByName("u").ToDouble();
  SetPivot(Parent.RM.aunit.GetAtom(item.GetFieldByName("pivot_atom_id").ToSizeT()));
  TDataItem* dep = item.FindItem("dependent");
  if (dep != NULL) {
    TStrStrList ordered = dep->GetOrderedFieldList();
    for (size_t i = 0; i < ordered.Count(); i++) {
      Dependent.Add(Parent.RM.aunit.GetAtom(
        ordered.GetObject(i).ToInt()))->SetParentAfixGroup(this);
    }
  }
  else {
    IndexRange::RangeItr i = item.GetItemByName("dependent_range").GetValue();
    while (i.HasNext()) {
      Dependent.Add(
        Parent.RM.aunit.GetAtom(i.Next()))->SetParentAfixGroup(this);
    }
  }
}
//..............................................................................
olxstr TAfixGroup::Describe() const {
  const int n = GetN(), m = GetM();
  if (n >= 9 || m >= 16) {
    return EmptyString();
  }
  if (n == 0) {
    return m_names[m];
  }
  if (m == 0) {
    return n_names[n].SubStringFrom(n_names[n].FirstIndexOf(' ') + 1);
  }
  return olxstr(m_names[m]) << " refined " << n_names[n];
}
//..............................................................................
TIString TAfixGroup::ToString() const {
  olxstr rv;
  rv << Pivot->GetLabel() << '(';
  size_t dep_cnt = 0;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (Dependent[i]->IsDeleted()) {
      continue;
    }
    rv << Dependent[i]->GetLabel() << ',';
    dep_cnt++;
  }
  if (dep_cnt == 0) {
    return Pivot->GetLabel();
  }
  rv.SetLength(rv.Length() - 1);  // remove trailing ','
  rv << ')';
  //item.AddField("d", D);
  //item.AddField("u", U);
  return rv;
}
//..............................................................................
bool TAfixGroup::IsEmpty() const {
  if (Pivot == 0 || Pivot->IsDeleted()) {
    return true;
  }
  size_t dep_cnt = 0;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (!Dependent[i]->IsDeleted()) {
      dep_cnt++;
    }
  }
  if (IsFixedGroup() && dep_cnt != Dependent.Count()) {
    return true;
  }
  if (Afix == 1 || Afix == 2) {
    return false;
  }
  return dep_cnt == 0;
}
//..............................................................................
void TAfixGroup::Sort() {
  if (IsFittedRing()) {
    return;
  }
  QuickSorter::Sort(Dependent,
    ComplexComparator::Make(
      FunctionAccessor::MakeConst((index_t(TCAtom::*)() const) & TCAtom::GetTag),
      TPrimitiveComparator())
  );
}
//..............................................................................
bool TAfixGroup::IsUsable() const {
  if (Pivot != 0) {
    if (Pivot->IsDeleted()) {
      return false;
    }
    if (Pivot->AreAllFixed(catom_var_name_X, 3)) {
      if (!IsRotating() && !IsSizable()) {
        return false;
      }
    }
  }
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (!Dependent[i]->IsDeleted() && Dependent[i]->AreAllFixed(catom_var_name_X, 3)) {
      return false;
    }
  }
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAfixGroups::ToDataItem(TDataItem& item) {
  int group_id = 0;
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
PyObject* TAfixGroups::PyExport(TPtrList<PyObject>& atoms) {
  int group_id = 0;
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
void TAfixGroups::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetFieldByName("n").ToSizeT();
  if (n != item.ItemCount()) {
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  }
  for (size_t i = 0; i < n; i++) {
    Groups.Add(new TAfixGroup(*this)).SetId(i);
    Groups.GetLast().FromDataItem(item.GetItemByIndex(i));
  }
}
//.............................................................................
void TAfixGroups::Release(TAfixGroup& ag)  {
  if (&ag.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "AFIX group parent differs");
  }
  Groups.Release(ag.GetId());
  for (size_t i=0; i < Groups.Count(); i++)
    Groups[i].SetId((uint16_t)i);
}
//.............................................................................
void TAfixGroups::Restore(TAfixGroup& ag) {
  if (&ag.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "AFIX group parent differs");
  }
  Groups.Add(ag);
  ag.SetId((uint16_t)(Groups.Count() - 1));
}
//.............................................................................
void TAfixGroups::SortGroupContent() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].Sort();
  }
}
//.............................................................................
