/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "samegroup.h"
#include "asymmunit.h"
#include "refmodel.h"
#include "index_range.h"

TSameGroup::TSameGroup(uint16_t id, TSameGroupList& parent)
: Atoms(parent.RM), Id(id), Parent(parent),
  ParentGroup(NULL), Esd12(0.02), Esd13(0.02)
{}
//.............................................................................
void TSameGroup::SetAtomIds(uint16_t id) {
  if (!Atoms.IsExplicit()) return;
  TAtomRefList atoms = Atoms.ExpandList(Parent.RM);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].GetAtom().SetSameId(id);
  }
}
//.............................................................................
void TSameGroup::Assign(const TSameGroup& sg)  {
  Clear();
  if (sg.Atoms.IsEmpty()) return;
  Atoms.Assign(sg.Atoms);
  SetAtomIds(GetId());
  Esd12 = sg.Esd12;
  Esd13 = sg.Esd13;
  for (size_t i=0; i < sg.Dependent.Count(); i++)
    Dependent.Add(Parent[sg.Dependent[i]->Id]);
  if (sg.GetParentGroup() != NULL)
    ParentGroup = &Parent[sg.GetParentGroup()->Id];
}
//.............................................................................
TCAtom& TSameGroup::Add(TCAtom& ca)  {
  ca.SetSameId(Id);
  Atoms.AddExplicit(ca);
  return ca;
}
//.............................................................................
void TSameGroup::ToDataItem(TDataItem& item) const {
  item.AddField("esd12", Esd12)
    .AddField("esd13", Esd13)
    .AddField("AtomList", Atoms.GetExpression());
  IndexRange::Builder irb;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    irb << Dependent[i]->GetId();
  }
  item.AddField("dependent", irb.GetString());
  if (ParentGroup != NULL)
    item.AddField("parent", ParentGroup->GetId());
}
//..............................................................................
#ifdef _PYTHON
PyObject* TSameGroup::PyExport(PyObject* main, TPtrList<PyObject>& allGroups,
  TPtrList<PyObject>& _atoms, TPtrList<PyObject>& equiv)
{
  PythonExt::SetDictItem(main, "esd12", Py_BuildValue("d", Esd12));
  PythonExt::SetDictItem(main, "esd13", Py_BuildValue("d", Esd13));
  TAtomRefList atom_list = Atoms.ExpandList(Parent.RM);
  PyObject* atoms = PyTuple_New(atom_list.Count());
  for (size_t i=0; i < atom_list.Count(); i++) {
    PyObject* eq;
    if (atom_list[i].GetMatrix() == NULL)
      eq = Py_None;
    else
      eq = equiv[atom_list[i].GetMatrix()->GetId()];
    Py_INCREF(eq);
    PyTuple_SetItem(atoms, i,
      Py_BuildValue("OO", Py_BuildValue("i", atom_list[i].GetAtom().GetTag()), eq));
  }
  PythonExt::SetDictItem(main, "atoms", atoms);
  PyObject* dependent = PyTuple_New(Dependent.Count());
  for (size_t i=0; i < Dependent.Count(); i++)
    PyTuple_SetItem(dependent, i, Py_BuildValue("i", Dependent[i]->GetTag()));
  PythonExt::SetDictItem(main, "dependent", dependent);
  if (ParentGroup != NULL) {
    PythonExt::SetDictItem(main, "parent", Py_BuildValue("i",
      ParentGroup->GetTag()));
  }
  return main;
}
#endif
//.............................................................................
void TSameGroup::FromDataItem(TDataItem& item) {
  Clear();
  Esd12 = item.GetFieldByName("esd12").ToDouble();
  Esd13 = item.GetFieldByName("esd13").ToDouble();

  if (item.FieldExists("AtomList")) {
    Atoms.Build(item.GetFieldByName("AtomList"));
    IndexRange::RangeItr di(item.GetFieldByName("dependent"));
    while (di.HasNext()) {
      AddDependent(Parent[di.Next()]);
    }
  }
  else {
    TAsymmUnit& au = Parent.RM.aunit;
    const TDataItem* _atoms = item.FindItem("atoms");
    if (_atoms != NULL) {
      for (size_t i = 0; i < _atoms->ItemCount(); i++)
        Add(au.GetAtom(_atoms->GetItemByIndex(i).GetValue().ToSizeT()));
    }
    else  {  // index range then
      IndexRange::RangeItr ai(item.GetFieldByName("atom_range"));
      while (ai.HasNext())
        Add(au.GetAtom(ai.Next()));
    }
    TDataItem& dep = item.GetItemByName("dependent");
    for (size_t i = 0; i < dep.ItemCount(); i++)
      AddDependent(Parent[dep.GetItemByIndex(i).GetValue().ToInt()]);
  }
  const olxstr p_id = item.FindField("parent");
  if( !p_id.IsEmpty() )
    ParentGroup = &Parent[p_id.ToInt()];
}
//.............................................................................
bool TSameGroup::DoOverlap(const TSameGroup &g) const {
  TAtomRefList ar1 = Atoms.ExpandList(Parent.RM);
  TAtomRefList ar2 = g.Atoms.ExpandList(g.Parent.RM);
  for (size_t i = 0; i < ar1.Count(); i++)
    ar1[i].GetAtom().SetTag(0);
  for (size_t i=0; i < ar2.Count(); i++)
    ar2[i].GetAtom().SetTag(1);
  for (size_t i=0; i < ar1.Count(); i++)
    if (ar1[i].GetAtom().GetTag() != 0)
      return true;
  return false;
}
//.............................................................................
bool TSameGroup::IsValidForSave() const {
  if (Atoms.IsEmpty()) return false;
  return true;
}
//.............................................................................
bool TSameGroup::AreAllAtomsUnique() const {
  TAtomRefList atoms = Atoms.ExpandList(Parent.RM);
  for (size_t i=0; i < atoms.Count(); i++)
    atoms[i].GetAtom().SetTag(i);
  for (size_t i=0; i < atoms.Count(); i++)
    if( (size_t)atoms[i].GetAtom().GetTag() != i)
      return false;
  return true;
}
//.............................................................................
//.............................................................................
//.............................................................................
void TSameGroupList::Release(TSameGroup& sg)  {
  if (&sg.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "SAME group parent differs");
  }
  if (Groups.Count() <= sg.GetId() || &Groups[sg.GetId()] != &sg)
    return;
  Groups.Release(sg.GetId());
  if (sg.GetParentGroup() != NULL)
    sg.GetParentGroup()->RemoveDependent(sg);
  sg.ClearAtomIds();
  for (size_t i=0; i < Groups.Count(); i++)
    Groups[i].SetId((uint16_t)i);
}
//.............................................................................
void TSameGroupList::Restore(TSameGroup& sg)  {
  if (&sg.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "SAME group parent differs");
  }
  sg.SetId((uint16_t)Groups.Count());
  Groups.Add(sg);
  if (sg.GetParentGroup() != NULL) {
    sg.GetParentGroup()->AddDependent(sg);
    // make sure that atoms have parent group Id
    if (Groups.Contains(sg.GetParentGroup())) {
      sg.GetParentGroup()->SetId(sg.GetParentGroup()->GetId());
    }
  }
}
//.............................................................................
void TSameGroupList::Delete(const TPtrList <TSameGroup> &groups_) {
  Groups.ForEach(ACollectionItem::TagSetter(0));
  TPtrList <TSameGroup> groups = groups_;
  for (size_t i = 0; i < groups.Count(); i++) {
    for (size_t j = 0; j < groups[i]->DependentCount(); j++) {
      groups.AddUnique(groups[i]->GetDependent(j));
    }
  }
  for (size_t i = 0; i < groups.Count(); i++) {
    if (&groups[i]->GetParent() != this) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "SAME group parent differs");
    }
    groups[i]->SetTag(1);
  }
  Groups.Pack(ACollectionItem::TagAnalyser(1));
  for (size_t i = 0; i < Groups.Count(); i++)
    Groups[i].SetId((uint16_t)i);
}
//.............................................................................
void TSameGroupList::ToDataItem(TDataItem& item) const {
  size_t cnt=0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsValidForSave() )  {
      Groups[i].ToDataItem(item.AddItem("group"));
      cnt++;
    }
  }
  item.AddField("n", cnt);
}
//..............................................................................
#ifdef _PYTHON
PyObject* TSameGroupList::PyExport(TPtrList<PyObject>& _atoms,
  TPtrList<PyObject>& equiv)
{
  size_t id = 0;
  for (size_t i=0; i < Groups.Count(); i++) {
    if (Groups[i].IsValidForSave())
      Groups[i].SetTag(id++);
  }
  if (id == 0)
    return PythonExt::PyNone();
  PyObject* main = PyTuple_New(id);
  TPtrList<PyObject> allGroups;
  for (size_t i=0; i < id; i++)
    PyTuple_SetItem(main, i, allGroups.Add(PyDict_New()));
  id = 0;
  for (size_t i=0; i < Groups.Count(); i++)
    if (Groups[i].IsValidForSave())
      Groups[i].PyExport(allGroups[id++], allGroups, _atoms, equiv);
  return main;
}
#endif
//.............................................................................
void TSameGroupList::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetFieldByName("n").ToSizeT();
  if (n != item.ItemCount()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "number of groups does not match the number of items");
  }
  for (size_t i=0; i < n; i++)
    New();
  for( size_t i=0; i < n; i++ )
    Groups[i].FromDataItem(item.GetItemByIndex(i));
}
//.............................................................................
void TSameGroupList::Assign(const TSameGroupList& sl) {
  Clear();
  for (size_t i=0; i < sl.Groups.Count(); i++)
    New().SetTag(0);
  for (size_t i=0; i < sl.Groups.Count(); i++) {
    // dependent first, to override shared atoms SameId
    for (size_t j=0; j < sl.Groups[i].DependentCount(); j++) {
      size_t id = sl.Groups[i].GetDependent(j).GetId();
      if (Groups[id].GetTag() != 0) continue;
      Groups[id].Assign(sl.Groups[i].GetDependent(j));
      Groups[id].SetTag(1);
    }
    if (Groups[i].GetTag() != 0) continue;
    Groups[i].Assign(sl.Groups[i]);
    Groups[i].SetTag(1);
  }
}
//.............................................................................
void TSameGroupList::OnAUUpdate() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].OnAUUpdate();
  }
}
//.............................................................................
void TSameGroupList::BeginAUSort() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].BeginAUSort();
  }
}
//.............................................................................
void TSameGroupList::EndAUSort() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].EndAUSort();
  }
}
//.............................................................................
void TSameGroupList::SortGroupContent() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].GetParentGroup() == NULL) {
      TPtrList<AtomRefList> deps;
      for (size_t j = 0; j < Groups[i].DependentCount(); j++) {
        deps.Add(Groups[i].GetDependent(j).GetAtoms());
      }
      Groups[i].GetAtoms().SortByTag(deps);
    }
  }
}
//.............................................................................
TSameGroup *TSameGroupList::Find(const TSameGroup &g) const {
  TAtomRefList ar1 = g.GetAtoms().ExpandList(RM);
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (&Groups[i] == &g || !Groups[i].GetAtoms().IsExplicit()) {
      continue;
    }
    TAtomRefList ar2 = Groups[i].GetAtoms().ExpandList(RM);
    if (ar1.Count() != ar2.Count()) {
      continue;
    }
    bool eq = true;
    for (size_t j = 0; j < ar1.Count(); j++) {
      if (ar1[j].GetAtom().GetId() != ar2[j].GetAtom().GetId()) {
        eq = false;
        break;
      }
    }
    if (eq) {
      return &Groups[i];
    }
  }
  return NULL;
}
//.............................................................................
