/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_same_group_h
#define __olx_xl_same_group_h
#include "catomlist.h"
#include "releasable.h"

BeginXlibNamespace()

class TSameGroupList;

class TSameGroup : public ACollectionItem, public AReleasable {
  TPtrList<TSameGroup> Dependent;  // pointers borrowed from Parent
  AtomRefList Atoms;
  TSameGroup* ParentGroup;
protected:
  // it does not clear the ParentGroup, to make Restore work...
  bool RemoveDependent(TSameGroup& sg) { return Dependent.Remove(sg); }
  void ClearDependent(bool clear_dep = true);
  void SetAtomIds(size_t);
  // on release
  void ClearAtomIds() { SetAtomIds(~0); }
  // re-orders this groups and dependent groups atoms according to tags
  void Reorder();
  // initialises atom_map_N
  olx_object_ptr<DistanceGenerator> get_generator() const;
public:
  TSameGroup(TSameGroupList& parent, bool tmp=false);
  ~TSameGroup();

  TSameGroupList& GetParent() const;
  TSameGroup* GetParentGroup() const { return ParentGroup; }

  void Assign(const TSameGroup& sg);
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void EndAUSort();

  TStrList::const_list_type Analyse(bool log,
    TPtrList<const TSameGroup> *offending=0) const;

  void Clear() {
    SetAtomIds(~0);
    Atoms.Clear();
    Dependent.Clear();
  }
  // does not reset the Atom's SameId
  void OnReleasedDelete() {
    Atoms.Clear();
    Dependent.Clear();
  }

  bool IsReference() const { return !Dependent.IsEmpty(); }
  size_t GetId() const { return this->GetReleasableId(); }
  void SetId(size_t id, bool set_atom_ids=false) {
    this->SetReleasableId(id);
    if (set_atom_ids) {
      SetAtomIds(id);
    }
  }

  // will invalidate previously assigned group
  TCAtom& Add(TCAtom& ca);

  void AddDependent(TSameGroup& sg) {
    Dependent.Add(sg);
    sg.ParentGroup = this;
  }

  // compares pointer addresses only!
  bool operator == (const TSameGroup& sr) const { return this == &sr; }

  int Compare(const TSameGroup& g) const;

  const AtomRefList &GetAtoms() const { return Atoms; }
  AtomRefList &GetAtoms() { return Atoms; }

  size_t DependentCount() const { return Dependent.Count(); }
  TSameGroup& GetDependent(size_t i) { return *Dependent[i]; }
  const TSameGroup& GetDependent(size_t i) const { return *Dependent[i]; }
  void PackDependent(index_t tag) {
    Dependent.Pack(ACollectionItem::TagAnalyser(tag));
  }
  bool IsValidForSave() const;
  // returns true if contains only unique atoms
  bool AreAllAtomsUnique() const;

  bool DoOverlap(const TSameGroup &g) const;

  // returns OVERLAP_ bit mask
  int DoOverlapEx(const TSameGroup& g) const;

  bool IsSubgroupOf(const TSameGroup& g) const;

  bool IsSupergroupOf(const TSameGroup& g) const {
    return g.IsSubgroupOf(*this);
  }

  // could return null, the returned value owned by the parent
  TSameGroup* ToExplicit(TTypeList<TSameGroup> *dest=0) const;

  // generates SADI list
  TStrList::const_list_type GenerateList() const;

  // a list of restrained distances for atom indices in the AU
  DistanceGenerator::pair_list_t::const_list_type GetRestrainedDistances() const {
    TTypeList<DistanceGenerator::pair_list_t> rv1;
    GetRestrainedDistances(rv1, rv1);
    // GCC cannot deduce return type??? cannot use FunctionAccessor::MakeConst directly
    size_t (DistanceGenerator::pair_list_t:: * f)() const = &DistanceGenerator::pair_list_t::Count;
    size_t sz = olx_sum(rv1, FunctionAccessor::MakeConst(f));
    DistanceGenerator::pair_list_t rv(olx_reserve(sz));
    for (size_t i = 0; i < rv1.Count(); i++) {
      rv.AddCopyAll(rv1[i]);
    }
    return rv;
  }

  void GetRestrainedDistances(TTypeList<DistanceGenerator::pair_list_t> &d12,
    TTypeList<DistanceGenerator::pair_list_t>& d13) const;

  void GetRestrainedDistances(
    olx_pdict<double, TTypeList<DistanceGenerator::pair_list_t> >& ds) const;

  // expands SAME into SADI lists of the parent RefinementModel
  void Expand(TStrList* log=0) const;

  double Esd12, Esd13;

  void ToDataItem(TDataItem& item) const;
  void ToDataItem_HRF(TDataItem& item) const;
  olxstr ToInsString() const;
#ifdef _PYTHON
  PyObject* PyExport(PyObject* main, TPtrList<PyObject>& allGroups,
    TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
  void FromDataItem_HRF(TDataItem& item);
  // for releasing/restoring items SetId must be called
  friend class TSameGroupList;
};

class TSameGroupList : public AReleasableContainer<TSameGroup> {
protected:
  void OnRestore(TSameGroup& item);
  void OnRelease(TSameGroup& item);
public:
  class RefinementModel& RM;

  TSameGroupList(RefinementModel& parent)
    : RM(parent)
  {}
  // if the dest is not specified - this objects keeps ownership
  TSameGroup& New(TSameGroup* ref = 0, TTypeList<TSameGroup>* dest = 0);

  TSameGroup& Build(const olxstr &exp, const olxstr &resi=EmptyString());
  void FixIds();
  const TSameGroup& operator [] (size_t i) const {
#ifdef OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, items.Count());
#endif
    return (const TSameGroup&)items[i];
  }
  TSameGroup& operator [] (size_t i) {
#ifdef OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, items.Count());
#endif
    return (TSameGroup&)items[i];
  }
  const TSameGroup& GetItem(size_t i) const {
#ifdef OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, items.Count());
#endif
    return (const TSameGroup&)items[i];
  }
  TSameGroup& GetItem(size_t i) {
#ifdef OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, i, 0, items.Count());
#endif
    return (TSameGroup&)items[i];
  }
  size_t Count() const { return items.Count(); }
  bool IsEmpty() const { return items.IsEmpty(); }
  // searches a group by the content
  TSameGroup *Find(const TSameGroup &g) const;
  void Clear() { AReleasableContainer::Clear(); }
  void Assign(const TSameGroupList& sl);
  void Delete(const TPtrList <TSameGroup> &groups);
  void Delete(TSameGroup& g) {
    Delete(TPtrList<TSameGroup>() << g);
  }
  void Sort();
  void Analyse();
  void FixOverlapping();
  void PrepareSave();
  // works on Reference groups only
  TPtrList<TSameGroup>::const_list_type
    FindSupergroups(const TSameGroup& sg,
      const olxdict<const TSameGroup*, TAtomRefList, TPointerComparator>* sg_atoms=0) const;
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void SortGroupContent();
  void EndAUSort();

  TStrList::const_list_type GenerateList() const;
  // expands all SAME into SADI lists
  void Expand(TStrList *log=0);

  void ToDataItem(TDataItem& item) const;
  // fo INS header - easy to read and update
  void ToDataItem_HRF(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
  TPtrList<PyObject>::const_list_type PyExportAsSADI(
    TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  // auto selects which format to use based on 'n' field
  void FromDataItem(TDataItem& item, bool clear=true);
private:
  void FromDataItem_HRF(TDataItem& item);
  void FromDataItem_(TDataItem& item, size_t n);
  // supergroups first
  static void SortSupergroups(TPtrList<TSameGroup>& groups,
    const olxdict<const TSameGroup*, TAtomRefList, TPointerComparator>& sg_atoms);

  // used when extra groups are generated by ToExplicit
  struct GroupTrimmer {
    TTypeList<TSameGroup> &groups;
    size_t sz;
    GroupTrimmer(TTypeList<TSameGroup>& groups)
      : groups(groups),
      sz(groups.Count())
    {}
    ~GroupTrimmer() {
      while (groups.Count() > sz) {
        groups.GetLast().OnReleasedDelete();
        groups.Delete(groups.Count() - 1);
      }
    }
  };
};

EndXlibNamespace()
#endif
