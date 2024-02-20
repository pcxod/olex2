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

TSameGroup::TSameGroup(uint16_t id, TSameGroupList& parent)
: Atoms(parent.RM), Id(id), Parent(parent),
  ParentGroup(0), Esd12(0.02), Esd13(0.02)
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
  item.AddField("esd12", Esd12)
    .AddField("esd13", Esd13)
    .AddField("AtomList", Atoms.GetExpression());
  IndexRange::Builder irb;
  for (size_t i = 0; i < Dependent.Count(); i++) {
    if (Dependent[i]->IsValidForSave()) {
      irb << Dependent[i]->GetTag();
    }
  }
  item.AddField("dependent", irb.GetString());
  if (ParentGroup != 0) {
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
  for (size_t i=0; i < atom_list.Count(); i++) {
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
bool TSameGroup::DoOverlap(const TSameGroup &g) const {
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
TStrList::const_list_type TSameGroup::Analyse(bool report,
  TPtrList<const TSameGroup>* offending) const
{
  TStrList log;
  if (!IsReference()) {
    return log;
  }
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
      if (offending != 0) {
        offending->Add(dg);
      }
      continue;
    }
    olxstr mixed;
    for (size_t ai = 0; ai < this_atoms.Count(); ai++) {
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
        log.Add(Atoms.GetExpression());
        log.Add(dg.Atoms.GetExpression());
      }
      if (offending != 0) {
        offending->Add(dg);
      }
    }
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
void TSameGroupList::Restore(TSameGroup& sg)  {
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
void TSameGroupList::Delete(const TPtrList <TSameGroup> &groups_) {
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
  for (size_t i=0; i < Groups.Count(); i++) {
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
  for (size_t i=0; i < sl.Groups.Count(); i++) {
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
  return 0;
}
//.............................................................................
TSameGroup& TSameGroupList::Build(const olxstr &exp, const olxstr &resi) {
  TSameGroup& rv = Groups.Add(
    new TSameGroup((uint16_t)Groups.Count(), *this));
  rv.GetAtoms().Build(exp, resi);
  rv.SetId(rv.GetId());
  return rv;
}
//.............................................................................
void TSameGroupList::Analyse() {
  TPtrList<const TSameGroup> refs = Groups.ptr().Filter(
    FunctionAccessorAnalyser::Make(
      FunctionAccessor::MakeConst(&TSameGroup::IsReference)));
  TStrList log;
  for (size_t i = 0; i < refs.Count(); i++) {
    const TSameGroup& sg1 = *refs[i];
    log.AddAll(sg1.Analyse(true));
    for (size_t j = i + 1; j < refs.Count(); j++) {
      const TSameGroup& sg2 = *refs[j];
      int ov = sg2.DoOverlapEx(sg1);
      if (ov == OVERLAP_NONE) {
        continue;
      }
      if ((ov & OVERLAP_SAME) != 0) {
        log.Add("");
      }
      if (ov == OVERLAP_OVERLAP) {
        log.Add("Invalid SAME - overlapping SAME do not share all of the atoms");
      }
    }
  }
  if (!log.IsEmpty()) {
    TBasicApp::NewLogEntry() << log;
  }
}
//.............................................................................
void TSameGroupList::PrepareSave() {
  TIndexList tags = TIndexList::FromList(RM.aunit.GetAtoms(), TCAtom::TagAccessor());
  this->BeginAUSort();
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
  /* check if any of the atom in dependent groups come from a refence group
  * If they do - rearrange 
  */
  for (size_t i = 0; i < refs.Count(); i++) {
    TSameGroup& sg = *refs[i];
    for (size_t j = 0; j < sg.DependentCount(); j++) {

    }
  }
  size_t merge_cnt = 0;
  FunctionAccessor::ConstFunctionAccessorR_<TCAtom, ExplicitCAtomRef> acc =
    FunctionAccessor::MakeConst(&ExplicitCAtomRef::GetAtom);
  // merge refrence groups first - meging will mess up Atom's Same Gropu ids!
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
    Groups.Pack(ACollectionItem::TagAnalyser(2));
    for (size_t i = 0; i < Groups.Count(); i++) {
      Groups[i].SetId((uint16_t)i);
    }
    FixIds();
  }
  /* Find overlaps between any same group with reference groups to
  sort out atom's order
  */
  Groups.ForEach(ACollectionItem::TagSetter(0));
  for (size_t i = 0; i < Groups.Count(); i++) {
    TSameGroup& sg1 = Groups[i];
    if (sg1.GetTag() != 0 || !sg1.IsReference()) {
      continue;
    }
    ref_l_t overlapping;
    overlapping << sg1;
    // implicit recursion
    for (size_t oi = 0; oi < overlapping.Count(); oi++) {
      TAtomRefList& sg1_a = sg_atoms[overlapping[oi]];
      for (size_t j = i + 1; j < Groups.Count(); j++) {
        TSameGroup& sg2 = Groups[j];
        if (sg2.GetTag() != 0 ||
          !(overlapping[oi]->IsReference() || sg2.IsReference()))
        {
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
      bool changes = true;
      // sort - supergroups first
      while (changes) {
        changes = false;
        for (size_t gi = 0; gi < overlapping.Count() - 1; gi++) {
          int ov = ACollectionItem::AnalyseOverlap(
            sg_atoms[overlapping[gi]], sg_atoms[overlapping[gi + 1]], acc);
          if (ov == OVERLAP_SUBGROUP) {
            changes = true;
            overlapping.Swap(gi, gi + 1);
          }
        }
      }
      { /* continous numbering for reference groups within the supergroup but
        anchored to the first atom within the SG
        */
        // mask supergroup with -1
        TAtomRefList& sg_ar = sg_atoms[overlapping[0]];
        sg_ar.ForEach(ACollectionItem::TagSetter(acc, -1));
        // mask reference groups with group_id
        for (size_t gi = 1; gi < overlapping.Count(); gi++) {
          if (!overlapping[gi]->IsReference()) {
            continue;
          }
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
        if (!sg->IsReference()) {
          sg = sg->GetParentGroup();
          TAtomRefList& ar2 = sg_atoms[sg];
          if (ar1.Count() != ar2.Count()) {
            throw TInvalidArgumentException(__OlxSrcInfo, "atoms list sizes");
          }
          for (size_t ai = 0; ai < ar1.Count(); ai++) {
            ar2[ai].GetAtom().SetTag(ar1[ai].GetAtom().GetTag());
          }
          QuickSorter::Sort(ar2, FunctionComparator::Make(&AtomRefList::RefTagCmp));
        }
        for (size_t di = 0; di < sg->DependentCount(); di++) {
          TAtomRefList& ar2 = sg_atoms[&sg->GetDependent(di)];
          if (&ar1 == &ar2) {
            sg->GetDependent(di).Atoms.SortExplicitRefs();
            continue;
          }
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
  RM.aunit.SetNonHAtomTags_();
  this->EndAUSort();
  for (size_t i = 0; i < RM.aunit.AtomCount(); i++) {
    RM.aunit.GetAtom(i).SetTag(tags[i]);
  }
}
//..............................................................................
