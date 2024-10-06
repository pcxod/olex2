/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
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
#include "tagholder.h"
#include "ins.h"

TSameGroup::TSameGroup(uint16_t id, TSameGroupList& parent)
  : Atoms(parent.RM), Id(id), Parent(parent),
  ParentGroup(0), Esd12(0.02), Esd13(0.04)
{}
//.............................................................................
TSameGroup::~TSameGroup() {
  Clear();
  // manage in batch when needed onstead
  //if (ParentGroup != 0) {
  //  ParentGroup->RemoveDependent(*this);
  //}
}
void TSameGroup::SetAtomIds(uint16_t id) {
  if (!Atoms.IsExplicit()) {
    return;
  }
  TAtomRefList atoms = Atoms.ExpandList(Parent.RM);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].GetAtom().SetSameId(id);
  }
}
//.............................................................................
void TSameGroup::Assign(const TSameGroup& sg) {
  Clear();
  if (sg.Atoms.IsEmpty()) {
    return;
  }
  Atoms.Assign(sg.Atoms);
  SetAtomIds(GetId());
  Esd12 = sg.Esd12;
  Esd13 = sg.Esd13;
  for (size_t i = 0; i < sg.Dependent.Count(); i++) {
    Dependent.Add(Parent[sg.Dependent[i]->Id]);
  }
  if (sg.GetParentGroup() != 0) {
    ParentGroup = &Parent[sg.GetParentGroup()->Id];
  }
}
//.............................................................................
TCAtom& TSameGroup::Add(TCAtom& ca) {
  ca.SetSameId(Id);
  Atoms.AddExplicit(ca);
  return ca;
}
//.............................................................................
void TSameGroup::ToDataItem(TDataItem& item) const {
  item.AddField("ID", GetId()); // for the user reference only!
  if (!IsReference()) {
    item.AddField("esd12", Esd12)
      .AddField("esd13", Esd13);
  }
  Atoms.ToDataItem(item.AddItem("Atoms"));
  IndexRange::Builder irb;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (Dependent[i]->IsValidForSave()) {
      irb << Dependent[i]->GetTag();
    }
  }
  if (IsReference()) {
    item.AddField("dependent", irb.GetString());
  }
  else if (ParentGroup !=0 ) { // implicit?
    item.AddField("parent", ParentGroup->GetTag());
  }
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
  for (size_t i = 0; i < atom_list.Count(); i++) {
    PyObject* eq;
    if (atom_list[i].GetMatrix() == 0) {
      eq = Py_None;
    }
    else {
      eq = equiv[atom_list[i].GetMatrix()->GetId()];
    }
    Py_INCREF(eq);
    PyTuple_SetItem(atoms, i,
      Py_BuildValue("OO", Py_BuildValue("i", atom_list[i].GetAtom().GetTag()), eq));
  }
  PythonExt::SetDictItem(main, "atoms", atoms);
  PyObject* dependent = PyTuple_New(Dependent.Count());
  for (size_t i = 0; i < Dependent.Count(); i++) {
    PyTuple_SetItem(dependent, i, Py_BuildValue("i", Dependent[i]->GetTag()));
  }
  PythonExt::SetDictItem(main, "dependent", dependent);
  if (ParentGroup != 0) {
    PythonExt::SetDictItem(main, "parent", Py_BuildValue("i",
      ParentGroup->GetTag()));
  }
  return main;
}
#endif
//.............................................................................
void TSameGroup::FromDataItem(TDataItem& item) {
  Clear();
  Esd12 = item.FindField("esd12", "0").ToDouble();
  Esd13 = item.FindField("esd13", "0").ToDouble();
  TDataItem* atoms = item.FindItem("Atoms");
  if (atoms != 0 || item.FieldExists("AtomList")) {
    if (atoms != 0) {
      Atoms.FromDataItem(*atoms);
    }
    else { // buggy version - no residue is saved...
      Atoms.Build(item.GetFieldByName("AtomList"));
    }
    olxstr dep_l = item.FindField("dependent");
    if (!dep_l.IsEmpty()) {
      IndexRange::RangeItr di(dep_l);
      while (di.HasNext()) {
        AddDependent(Parent[di.Next()]);
      }
    }
  }
  else { // backwards compatibility
    TAsymmUnit& au = Parent.RM.aunit;
    const TDataItem* _atoms = item.FindItem("atoms");
    if (_atoms != 0) {
      for (size_t i = 0; i < _atoms->ItemCount(); i++) {
        Add(au.GetAtom(_atoms->GetItemByIndex(i).GetValue().ToSizeT()));
      }
    }
    else {  // index range then
      IndexRange::RangeItr ai(item.GetFieldByName("atom_range"));
      while (ai.HasNext()) {
        Add(au.GetAtom(ai.Next()));
      }
    }
    TDataItem& dep = item.GetItemByName("dependent");
    for (size_t i = 0; i < dep.ItemCount(); i++) {
      AddDependent(Parent[dep.GetItemByIndex(i).GetValue().ToInt()]);
    }
  }
  const olxstr p_id = item.FindField("parent");
  if (!p_id.IsEmpty()) {
    ParentGroup = &Parent[p_id.ToInt()];
  }
}
//.............................................................................
bool TSameGroup::DoOverlap(const TSameGroup& g) const {
  TAtomRefList ar1 = Atoms.ExpandList(Parent.RM);
  TAtomRefList ar2 = g.Atoms.ExpandList(g.Parent.RM);
  for (size_t i = 0; i < ar1.Count(); i++) {
    ar1[i].GetAtom().SetTag(0);
  }
  for (size_t i = 0; i < ar2.Count(); i++) {
    ar2[i].GetAtom().SetTag(1);
  }
  for (size_t i = 0; i < ar1.Count(); i++) {
    if (ar1[i].GetAtom().GetTag() != 0) {
      return true;
    }
  }
  return false;
}
//.............................................................................
int TSameGroup::DoOverlapEx(const TSameGroup& g) const {
  TAtomRefList ar1 = Atoms.ExpandList(Parent.RM);
  TAtomRefList ar2 = g.Atoms.ExpandList(g.Parent.RM);
  return ACollectionItem::AnalyseOverlap(ar1, ar2,
    FunctionAccessor::MakeConst(&ExplicitCAtomRef::GetAtom));
}
//.............................................................................
bool TSameGroup::IsSubgroupOf(const TSameGroup& g) const {
  TAtomRefList ar1 = Atoms.ExpandList(Parent.RM);
  TAtomRefList ar2 = g.Atoms.ExpandList(g.Parent.RM);
  for (size_t i = 0; i < ar1.Count(); i++) {
    ar1[i].GetAtom().SetTag(0);
  }
  for (size_t i = 0; i < ar2.Count(); i++) {
    ar2[i].GetAtom().SetTag(1);
  }
  for (size_t i = 0; i < ar1.Count(); i++) {
    if (ar1[i].GetAtom().GetTag() != 1) {
      return false;
    }
  }
  return true;
}
//.............................................................................
bool TSameGroup::IsValidForSave() const {
  return !Atoms.IsEmpty() && Atoms.IsValid();
}
//.............................................................................
bool TSameGroup::AreAllAtomsUnique() const {
  TAtomRefList atoms = Atoms.ExpandList(Parent.RM);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].GetAtom().SetTag(i);
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    if ((size_t)atoms[i].GetAtom().GetTag() != i) {
      return false;
    }
  }
  return true;
}
//.............................................................................
void TSameGroup::OnAUUpdate() {
  Atoms.OnAUUpdate();
}
//.............................................................................
void TSameGroup::BeginAUSort() {
  if (!IsReference()) {
    Atoms.BeginAUSort();
  }
}
//.............................................................................
void TSameGroup::EndAUSort() {
  if (!IsReference()) {
    Atoms.EndAUSort(true);
  }
}
//.............................................................................
int TSameGroup::Compare(const TSameGroup& g) const {
  if (GetParentGroup() != 0) {
    if (g.GetParentGroup() == 0) {
      return 1;
    }
  }
  else {
    if (g.GetParentGroup() == 0) {
      return -1;
    }
  }
  // equivalent from point of view of ParentGroup, check first atom Id
  if (!Atoms.IsExplicit() || !g.Atoms.IsExplicit()) {
    return 0;
  }
  TAtomRefList this_atoms = Atoms.ExpandList(Parent.RM);
  TAtomRefList that_atoms = g.Atoms.ExpandList(g.Parent.RM);
  if (this_atoms.IsEmpty()) {
    if (that_atoms.IsEmpty()) {
      return 0;
    }
    return -1;
  }
  else if (that_atoms.IsEmpty()) {
    return 1;
  }
  return olx_cmp(this_atoms[0].GetAtom().GetId(), that_atoms[0].GetAtom().GetId());
}
//.............................................................................
olx_object_ptr<DistanceGenerator> TSameGroup::get_generator() const {
  TAtomRefList ar = GetAtoms().ExpandList(Parent.RM);
  TArrayList<TAtomRefList> di_atoms(DependentCount());
  for (size_t di = 0; di < DependentCount(); di++) {
    di_atoms[di] = GetDependent(di).GetAtoms().ExpandList(Parent.RM);
    if (ar.Count() != di_atoms[di].Count()) {
      throw TInvalidArgumentException(__OlxSrcInfo, "atoms list sizes");
    }
  }
  const olx_capacity_t cap = olx_reserve(ar.Count() * DependentCount());
  DistanceGenerator::atom_set_t atom_set(cap),
    inc_set(cap);
  DistanceGenerator::atom_map_N_t atom_map(cap);
  for (size_t i = 0; i < ar.Count(); i++) {
    const  TCAtom& a = ar[i].GetAtom();
    atom_set.Add(a.GetId());
    TSizeList& idl = atom_map.Add(a.GetId());
    for (size_t j = 0; j < di_atoms.Count(); j++) {
      size_t aid = di_atoms[j][i].GetAtom().GetId();
      idl.Add(aid);
      inc_set.Add(aid);
    }
  }
  inc_set.AddAll(atom_set);
  olx_object_ptr<DistanceGenerator> d = new DistanceGenerator();
  d->atom_map_N = new DistanceGenerator::atom_map_N_t();
  d->atom_map_N->TakeOver(atom_map);
  d->Generate(Parent.RM.aunit, atom_set, true, true, inc_set);
  return d;
}
//.............................................................................
TStrList::const_list_type TSameGroup::GenerateList() const {
  TStrList rv;
  olx_object_ptr<DistanceGenerator> d = get_generator();
  rv.AddAll(d->GenerateSADIList(Parent.RM.aunit, *d->atom_map_N, Esd12, Esd13));
  return rv;
}
//.............................................................................
TStrList::const_list_type TSameGroup::Analyse(bool report,
  TPtrList<const TSameGroup>* offending) const
{
  TStrList log;
  if (!IsReference()) {
    return log;
  }
  olxset< const TSameGroup*, TPointerComparator> conflicted;
  TAtomRefList this_atoms = Atoms.ExpandList(Parent.RM);
  for (size_t i = 0; i < Dependent.Count(); i++) {
    const TSameGroup& dg = *Dependent[i];
    TAtomRefList that_atoms = Dependent[i]->Atoms.ExpandList(dg.Parent.RM);
    if (this_atoms.Count() != that_atoms.Count()) {
      if (report) {
        log.Add("Mismatching atom number for ") << this_atoms.Count()
          << " vs " << that_atoms.Count();
        log.Add(Atoms.GetExpression());
        log.Add(dg.Atoms.GetExpression());
      }
      conflicted.Add(&dg);
    }
    olxstr mixed;
    for (size_t ai = 0; ai < olx_min(this_atoms.Count(), that_atoms.Count()); ai++) {
      if (this_atoms[ai].GetAtom().GetType() !=
        that_atoms[ai].GetAtom().GetType())
      {
        if (report) {
          mixed << ", [" << this_atoms[ai].GetAtom().GetLabel() << " - "
            << that_atoms[ai].GetAtom().GetLabel() << ']';
        }
      }
    }
    if (!mixed.IsEmpty()) {
      if (report) {
        log.Add("Mixed atom types: ") << mixed.SubStringFrom(2);
        if (!conflicted.Contains(&dg)) {
          log.Add(Atoms.GetExpression());
          log.Add(dg.Atoms.GetExpression());
        }
      }
      conflicted.Add(&dg);
    }
  }
  if (offending != 0) {
    offending->AddAll(conflicted);
  }
  return log;
}
//.............................................................................
void TSameGroup::Reorder() {
  if (!IsReference()) {
    return;
  }
  Atoms.SortExplicitRefs();
  for (size_t i = 0; i < Dependent.Count(); i++) {
    Dependent[i]->Atoms.ConvertToExplicit();
    Dependent[i]->Atoms.SortExplicitRefs();
    Dependent[i]->Atoms.EndAUSort(true);
  }
}
//.............................................................................
void TSameGroup::GetRestrainedDistances(
  TTypeList<DistanceGenerator::pair_list_t>& d12,
  TTypeList<DistanceGenerator::pair_list_t>& d13) const
{
  if (!IsReference() || !Atoms.IsExplicit()) {
    return;
  }
  olx_object_ptr<DistanceGenerator> d = get_generator();
  TTypeList<DistanceGenerator::pair_list_t> l12 =
    DistanceGenerator::GeneratePairList(d->distances_12, Parent.RM.aunit, *d->atom_map_N);
  d12.AddAll(l12);
  l12.ReleaseAll();
  TTypeList<DistanceGenerator::pair_list_t> l13 =
    DistanceGenerator::GeneratePairList(d->distances_13, Parent.RM.aunit, *d->atom_map_N);
  d13.AddAll(l13);
  l13.ReleaseAll();
}
//.............................................................................
void TSameGroup::Expand(TStrList* log) const {
  if (!IsReference() || !Atoms.IsExplicit()) {
    return;
  }
  olx_object_ptr<DistanceGenerator> d = get_generator();
  d->GenerateSADI(Parent.RM, d->atom_map_N, Esd12, Esd13, log);
}
//.............................................................................
//.............................................................................
//.............................................................................
void TSameGroupList::Release(TSameGroup& sg) {
  if (&sg.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "SAME group parent differs");
  }
  if (Groups.Count() <= sg.GetId() || &Groups[sg.GetId()] != &sg) {
#ifdef _DEBUG
    throw TFunctionFailedException(__OlxSrcInfo, "assert");
#endif
    return;
  }
  Groups.Release(sg.GetId());
  if (sg.GetParentGroup() != 0) {
    sg.GetParentGroup()->RemoveDependent(sg);
  }
  sg.ClearAtomIds();
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].SetId((uint16_t)i);
  }
  FixIds();
}
//.............................................................................
void TSameGroupList::Restore(TSameGroup& sg) {
  if (&sg.GetParent() != this) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "SAME group parent differs");
  }
  sg.SetId((uint16_t)Groups.Count());
  Groups.Add(sg);
  if (sg.GetParentGroup() != 0) {
    sg.GetParentGroup()->AddDependent(sg);
    // make sure that atoms have parent group Id
    if (Groups.Contains(sg.GetParentGroup())) {
      sg.GetParentGroup()->SetId(sg.GetParentGroup()->GetId());
    }
  }
}
//.............................................................................
void TSameGroupList::Delete(const TPtrList <TSameGroup>& groups_) {
  Groups.ForEach(ACollectionItem::TagSetter(0));
  TPtrList <TSameGroup> groups = groups_;
  ACollectionItem::Unify(groups);
  // implicit recursion
  for (size_t i = 0; i < groups.Count(); i++) {
    for (size_t j = 0; j < groups[i]->DependentCount(); j++) {
      groups.Add(groups[i]->GetDependent(j));
    }
  }
  for (size_t i = 0; i < groups.Count(); i++) {
    if (&groups[i]->GetParent() != this) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "SAME group parent differs");
    }
    groups[i]->SetTag(1);
  }
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].PackDependent(1);
    if (Groups[i].GetParentGroup() == 0 && Groups[i].DependentCount() == 0) {
      Groups[i].SetTag(1);
    }
  }
  Groups.Pack(ACollectionItem::TagAnalyser(1));
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].SetId((uint16_t)i);
  }
  FixIds();
}
//.............................................................................
void TSameGroupList::Sort() {
  QuickSorter::Sort(Groups);
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].SetId((uint16_t)i);
  }
  FixIds();
}
//.............................................................................
void TSameGroupList::ToDataItem(TDataItem& item) const {
  size_t cnt = 0;
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsValidForSave()) {
      Groups[i].SetTag(cnt++);
    }
  }
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsValidForSave()) {
      Groups[i].ToDataItem(item.AddItem("group"));
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
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsValidForSave()) {
      Groups[i].SetTag(id++);
    }
  }
  if (id == 0) {
    return PythonExt::PyNone();
  }
  PyObject* main = PyTuple_New(id);
  TPtrList<PyObject> allGroups;
  for (size_t i = 0; i < id; i++) {
    PyTuple_SetItem(main, i, allGroups.Add(PyDict_New()));
  }
  id = 0;
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (Groups[i].IsValidForSave()) {
      Groups[i].PyExport(allGroups[id++], allGroups, _atoms, equiv);
    }
  }
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
  for (size_t i = 0; i < n; i++) {
    New();
  }
  for (size_t i = 0; i < n; i++) {
    Groups[i].FromDataItem(item.GetItemByIndex(i));
  }
}
//.............................................................................
void TSameGroupList::FixIds() {
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (!Groups[i].IsReference()) {
      continue;
    }
    Groups[i].SetAtomIds(Groups[i].GetId());
  }
}
void TSameGroupList::Assign(const TSameGroupList& sl) {
  Clear();
  Groups.SetCapacity(sl.Groups.Count());
  for (size_t i = 0; i < sl.Groups.Count(); i++) {
    New();
  }
  for (size_t i = 0; i < sl.Groups.Count(); i++) {
    Groups[i].Assign(sl.Groups[i]);
  }
  FixIds();
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
    if (Groups[i].GetParentGroup() == 0) {
      TPtrList<AtomRefList> deps;
      for (size_t j = 0; j < Groups[i].DependentCount(); j++) {
        deps.Add(Groups[i].GetDependent(j).GetAtoms());
      }
      Groups[i].GetAtoms().SortByTag(deps);
    }
  }
}
//.............................................................................
TSameGroup* TSameGroupList::Find(const TSameGroup& g) const {
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
  return 0;
}
//.............................................................................
TSameGroup& TSameGroupList::Build(const olxstr& exp, const olxstr& resi) {
  TSameGroup& rv = Groups.Add(
    new TSameGroup((uint16_t)Groups.Count(), *this));
  rv.GetAtoms().Build(exp, resi);
  rv.SetId(rv.GetId());
  return rv;
}
//.............................................................................
void TSameGroupList::Analyse() {
  TPtrList< TSameGroup> refs = Groups.ptr().Filter(
    FunctionAccessorAnalyser::Make(
      FunctionAccessor::MakeConst(&TSameGroup::IsReference)));
  TStrList log;
  for (size_t i = 0; i < refs.Count(); i++) {
    if (refs[i] == 0) {
      continue;
    }
    const TSameGroup& sg1 = *refs[i];
    log.AddAll(sg1.Analyse(true));
    for (size_t j = i + 1; j < refs.Count(); j++) {
      if (refs[j] == 0) {
        continue;
      }
      const TSameGroup& sg2 = *refs[j];
      int ov = sg2.DoOverlapEx(sg1);
      if (ov == OVERLAP_NONE) {
        continue;
      }
      if (ov == OVERLAP_OVERLAP) {
        log.Add("Olex2 cannot use this kind of SAME - overlapping SAME do not share all of the atoms");
        log.Add("Use 'same -fix' to expand unsuported SAME into SADI or 'same -e' to expand all of SAME into SADI");
      }
    }
  }
  if (!log.IsEmpty()) {
    TBasicApp::NewLogEntry() << log;
  }
}
//.............................................................................
void TSameGroupList::FixOverlapping() {
  TPtrList< TSameGroup> refs = Groups.ptr().Filter(
    FunctionAccessorAnalyser::Make(
      FunctionAccessor::MakeConst(&TSameGroup::IsReference)));
  TStrList log;
  TPtrList<TSameGroup> to_del;
  for (size_t i = 0; i < refs.Count(); i++) {
    if (refs[i] == 0) {
      continue;
    }
    TSameGroup& sg1 = *refs[i];
    for (size_t j = i + 1; j < refs.Count(); j++) {
      if (refs[j] == 0) {
        continue;
      }
      TSameGroup& sg2 = *refs[j];
      int ov = sg2.DoOverlapEx(sg1);
      if (ov == OVERLAP_NONE) {
        continue;
      }
      if (ov == OVERLAP_OVERLAP) {
        to_del.Add(&sg2);
      }
    }
  }
  ACollectionItem::Unify(to_del);
  for (size_t i = 0; i < to_del.Count(); i++) {
    to_del[i]->Expand(&log);
  }
  RM.MergeSADI();
  Delete(to_del);
  if (!log.IsEmpty()) {
    TBasicApp::NewLogEntry() << log;
  }
}
//.............................................................................
void TSameGroupList::PrepareSave() {
  TIndexList tags = TIndexList::FromList(RM.aunit.GetAtoms(), TCAtom::TagAccessor());
  typedef TPtrList<TSameGroup> ref_l_t;
  ref_l_t refs = Groups.ptr().Filter(
    FunctionAccessorAnalyser::Make(
      FunctionAccessor::MakeConst(&TSameGroup::IsReference)));
  refs.ForEach(ACollectionItem::TagSetter(0));
  olxdict<const TSameGroup*, TAtomRefList, TPointerComparator>
    sg_atoms;
  for (size_t i = 0; i < Groups.Count(); i++) {
    sg_atoms.Add(&Groups[i], Groups[i].GetAtoms().ExpandList(RM));
  }
  /* Several complex scenario to be considered:
   - check if any of the atom in dependent groups come from a refence group
   - dependent groups belongs to multiple reference groups
  */
  size_t merge_cnt = 0;
  FunctionAccessor::ConstFunctionAccessorR_<TCAtom, ExplicitCAtomRef> acc =
    FunctionAccessor::MakeConst(&ExplicitCAtomRef::GetAtom);
  // merge reference groups
  Groups.ForEach(ACollectionItem::TagSetter(0));
  for (size_t i = 0; i < refs.Count(); i++) {
    TSameGroup& sg1 = *refs[i];
    if (sg1.GetTag() != 0) {
      continue;
    }
    TAtomRefList& sg1_a = sg_atoms[&sg1];
    for (size_t j = i + 1; j < refs.Count(); j++) {
      TSameGroup& sg2 = *refs[j];
      if (sg2.GetTag() != 0) {
        continue;
      }
      int ov = ACollectionItem::AnalyseOverlap(sg1_a, sg_atoms[&sg2], acc);
      if (ov == OVERLAP_NONE) {
        continue;
      }
      sg2.SetTag(1);
      if (ov == OVERLAP_SAME) {
        size_t sg1_dc = sg1.DependentCount();
        for (size_t di = 0; di < sg2.DependentCount(); di++) {
          bool uniq = true;
          TSameGroup& dep = sg2.GetDependent(di);
          for (size_t di1 = 0; di1 < sg1_dc; di1++) {
            if (ACollectionItem::IsTheSame(
              sg_atoms[&dep],
              sg_atoms[&sg1.GetDependent(di1)], acc))
            {
              dep.SetTag(2);
              dep.ReleasedClear();
              uniq = false;
              break;
            }
          }
          if (uniq) {
            sg1.AddDependent(sg2.GetDependent(di));
          }
        }
        TAtomRefList& arl = sg_atoms[&sg2];
        for (size_t ai = 0; ai < arl.Count(); ai++) {
          arl[i].GetAtom().SetSameId(sg1.GetId());
        }
        sg2.SetTag(2);
        sg2.ReleasedClear();
        merge_cnt++;
      }
    }
  }
  if (merge_cnt > 0) {
    refs.Pack(ACollectionItem::TagAnalyser(2));
    Groups.Pack(ACollectionItem::TagAnalyser(2));
  }
  // make sure subgroups survive!
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].SetTag(sg_atoms[&Groups[i]].Count());
  }
  QuickSorter::Sort(Groups,
    ReverseComparator::Make(ACollectionItem::TagComparator()));
  for (size_t i = 0; i < Groups.Count(); i++) {
    Groups[i].SetId((uint16_t)i);
    Groups[i].SetTag(sg_atoms[&Groups[i]].Count());
  }
  FixIds();
  /* Find overlaps between any ref groups
  */
  refs.ForEach(ACollectionItem::TagSetter(0));
  for (size_t i = 0; i < refs.Count(); i++) {
    TSameGroup& sg1 = *refs[i];
    if (sg1.GetTag() != 0) {
      continue;
    }
    ref_l_t overlapping;
    overlapping << sg1;
    // implicit recursion
    for (size_t oi = 0; oi < overlapping.Count(); oi++) {
      TAtomRefList& sg1_a = sg_atoms[overlapping[oi]];
      for (size_t j = i + 1; j < refs.Count(); j++) {
        TSameGroup& sg2 = *refs[j];
        if (sg2.GetTag() != 0) {
          continue;
        }
        int ov = ACollectionItem::AnalyseOverlap(sg1_a, sg_atoms[&sg2], acc);
        if (ov != OVERLAP_NONE) {
          sg2.SetTag(1);
          overlapping << sg2;
        }
      }
    }
    if (overlapping.Count() > 1) {
      SortSupergroups(overlapping, sg_atoms);
      { /* continous numbering reference groups within the supergroup
        */
        // mask supergroup with -1
        TAtomRefList& sg_ar = sg_atoms[overlapping[0]];
        sg_ar.ForEach(ACollectionItem::TagSetter(acc, -1));
        // mask reference groups with group_id
        for (size_t gi = 1; gi < overlapping.Count(); gi++) {
          TAtomRefList& ar = sg_atoms[overlapping[gi]];
          for (size_t ai = 0; ai < ar.Count(); ai++) {
            ar[ai].GetAtom().SetTag(overlapping[gi]->GetId());
          }
        }
        olx_pdict<index_t, index_t> gs;
        index_t tag = 0;
        for (size_t ai = 0; ai < sg_ar.Count(); ai++) {
          TCAtom& ca = sg_ar[ai].GetAtom();
          if (ca.GetTag() == -1) {
            ca.SetTag(tag++);
          }
          else {
            olx_pair_t<size_t, bool> ip = gs.AddEx(ca.GetTag());
            if (ip.b) {
              gs.GetValue(ip.a) = tag;
              tag += Groups[ca.GetTag()].Atoms.RefCount() + 1;
            }
            ca.SetTag(++gs.GetValue(ip.a));
          }
        }
      }

      for (size_t gi = 0; gi < overlapping.Count(); gi++) {
        TSameGroup* sg = overlapping[gi];
        TAtomRefList& ar1 = sg_atoms[sg];
        for (size_t di = 0; di < sg->DependentCount(); di++) {
          TAtomRefList& ar2 = sg_atoms[&sg->GetDependent(di)];
          if (ar1.Count() != ar2.Count()) {
            throw TInvalidArgumentException(__OlxSrcInfo, "atoms list sizes");
          }
          for (size_t ai = 0; ai < ar1.Count(); ai++) {
            ar2[ai].GetAtom().SetTag(ar1[ai].GetAtom().GetTag());
          }
          sg->GetDependent(di).Atoms.SortExplicitRefs();
          QuickSorter::Sort(ar2, FunctionComparator::Make(&AtomRefList::RefTagCmp));
        }
        QuickSorter::Sort(ar1, FunctionComparator::Make(&AtomRefList::RefTagCmp));
        sg->Atoms.SortExplicitRefs();
      }
    }
  }
  for (size_t i = 0; i < RM.aunit.AtomCount(); i++) {
    RM.aunit.GetAtom(i).SetTag(tags[i]);
  }
}
//..............................................................................
TPtrList<TSameGroup>::const_list_type
TSameGroupList::FindSupergroups(const TSameGroup& sg,
  const olxdict<const TSameGroup*, TAtomRefList, TPointerComparator>* sg_atoms) const
{
  TPtrList<TSameGroup> rv;
  if (!sg.IsReference()) {
    return rv;
  }
  typedef olxdict<const TSameGroup*, TAtomRefList, TPointerComparator> dict_t;
  olx_object_ptr<dict_t> sg_atoms_p;
  if (sg_atoms == 0) {
    sg_atoms_p = new dict_t(olx_reserve(Groups.Count()));
    sg_atoms = &sg_atoms_p;
    for (size_t i = 0; i < Groups.Count(); i++) {
      if (!Groups[i].IsReference()) {
        continue;
      }
      sg_atoms_p->Add(&Groups[i], Groups[i].GetAtoms().ExpandList(RM));
    }
  }
  ItemTagHolder ith;
  ExplicitCAtomRef::AtomAccessor acc = ExplicitCAtomRef::AtomAccessor();
  for (size_t i = 0; i < sg_atoms->Count(); i++) {
    ith.store(sg_atoms->GetValue(i), acc);
  }
  TAtomRefList& ar1 = (*sg_atoms)[&sg];
  for (size_t i = 0; i < Groups.Count(); i++) {
    if (!Groups[i].IsReference() || sg.GetId() == Groups[i].GetId()) {
      continue;
    }
    TAtomRefList ar2 = (*sg_atoms)[&Groups[i]];
    int ov = ACollectionItem::AnalyseOverlap(ar2, ar1, acc);
    if (ov == OVERLAP_SUPERGROUP) {
      rv.Add(Groups[i]);
    }
  }
  SortSupergroups(rv, *sg_atoms);
  return rv;
}
//..............................................................................
void TSameGroupList::SortSupergroups(
  TPtrList<TSameGroup>& groups,
  const olxdict<const TSameGroup*, TAtomRefList, TPointerComparator>& sg_atoms)
{
  if (groups.Count() < 2) {
    return;
  }
  else if (groups.Count() == 2) {
    if (ACollectionItem::AnalyseOverlap(
      sg_atoms[groups[0]], sg_atoms[groups[1]],
      ExplicitCAtomRef::AtomAccessor()) == OVERLAP_SUBGROUP)
    {
      groups.Swap(0, 1);
    }
    return;
  }
  bool changes = true;
  while (changes) {
    changes = false;
    for (size_t gi = 0; gi < groups.Count() - 1; gi++) {
      int ov = ACollectionItem::AnalyseOverlap(
        sg_atoms[groups[gi]], sg_atoms[groups[gi + 1]],
        ExplicitCAtomRef::AtomAccessor());
      if (ov == OVERLAP_SUBGROUP) {
        changes = true;
        groups.Swap(gi, gi + 1);
      }
    }
  }
}
//..............................................................................
TStrList::const_list_type TSameGroupList::GenerateList() const {
  TStrList rv;
  typedef TTypeList<DistanceGenerator::pair_list_t> lsts_t;
  olxdict<double, lsts_t, TPrimitiveComparator> lists;
  for (size_t gi = 0; gi < Groups.Count(); gi++) {
    const TSameGroup& sg = Groups[gi];
    if (!sg.IsReference() || !sg.GetAtoms().IsExplicit()) {
      continue;
    }
    size_t idx12 = lists.IndexOf(sg.Esd12);
    if (idx12 == InvalidIndex) {
      idx12 = lists.AddEx(sg.Esd12).a;
    }
    size_t idx13 = lists.IndexOf(sg.Esd13);
    if (idx13 == InvalidIndex) {
      idx13 = lists.AddEx(sg.Esd13).a;
    }
    sg.GetRestrainedDistances(lists.GetValue(idx12),
      lists.GetValue(idx13));
  }
  for (size_t i = 0; i < lists.Count(); i++) {
    lsts_t m = DistanceGenerator::Merge(lists.GetValue(i));
    if (m.IsEmpty()) {
      continue;
    }
    for (size_t j = 0; j < m.Count(); j++) {
      if (m[j].Count() < 2) {
        continue;
      }
      olxstr str = olxstr("SADI ") << lists.GetKey(i);
      for (size_t k = 0; k < m[j].Count(); k++) {
        str << ' ' << RM.aunit.GetAtom(m[j][k].a).GetResiLabel()
          << ' ' << RM.aunit.GetAtom(m[j][k].b).GetResiLabel();
      }
      TIns::HyphenateIns(str, rv);
    }
  }
  return rv;
}
//..............................................................................
void TSameGroupList::Expand() {
  for (size_t gi = 0; gi < Groups.Count(); gi++) {
    const TSameGroup& sg = Groups[gi];
    if (!sg.IsReference() || !sg.GetAtoms().IsExplicit()) {
      continue;
    }
    sg.Expand();
  }
  Clear();
}
//..............................................................................
