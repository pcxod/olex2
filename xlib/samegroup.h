/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_same_group_h
#define __olx_xl_same_group_h
#include "catomlist.h"

BeginXlibNamespace()

class TSameGroupList;
class TSameGroup : public ACollectionItem {
  TPtrList<TSameGroup> Dependent;  // pointers borrowed from Parent
  AtomRefList Atoms;
  uint16_t Id;
  TSameGroupList& Parent;
  TSameGroup* ParentGroup;
protected:
  void SetId(uint16_t id) { SetAtomIds(Id = id); }
  // it does not clear the ParentGroup, to make Restore work...
  bool RemoveDependent(TSameGroup& sg) { return Dependent.Remove(sg); }
  void SetAtomIds(uint16_t);
  // on release
  void ClearAtomIds() { SetAtomIds(~0); }
public:
  TSameGroup(uint16_t id, TSameGroupList& parent);
  ~TSameGroup() { Clear(); }

  const TSameGroupList& GetParent() const { return Parent; }
  TSameGroupList& GetParent() { return Parent; }
  TSameGroup* GetParentGroup() const { return ParentGroup; }
  uint16_t GetId() const { return Id; }

  void Assign(const TSameGroup& sg);
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void EndAUSort();

  void Clear() {
    SetAtomIds(~0);
    Atoms.Clear();
    Dependent.Clear();
  }
  // does not reset the Atom's SameId
  void ReleasedClear() {
    Atoms.Clear();
    Dependent.Clear();
  }

  bool IsReference() const { return !Dependent.IsEmpty(); }

  // will invalidate previously assigned group
  TCAtom& Add(TCAtom& ca);

  void AddDependent(TSameGroup& sg) {
    Dependent.Add(sg);
    sg.ParentGroup = this;
  }

  // compares pointer addresses only!
  bool operator == (const TSameGroup& sr) const { return this == &sr; }

  const AtomRefList &GetAtoms() const { return Atoms; }
  AtomRefList &GetAtoms() { return Atoms; }

  size_t DependentCount() const { return Dependent.Count(); }
  TSameGroup& GetDependent(size_t i) { return *Dependent[i]; }
  const TSameGroup& GetDependent(size_t i) const { return *Dependent[i]; }

  bool IsValidForSave() const;
  // returns true if contains only unique atoms
  bool AreAllAtomsUnique() const;

  bool DoOverlap(const TSameGroup &g) const;

  double Esd12, Esd13;

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(PyObject* main, TPtrList<PyObject>& allGroups,
    TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
  // for releasing/restoring items SetId must be called
  friend class TSameGroupList;
};

class TSameGroupList {
  TTypeList<TSameGroup> Groups;
public:

  class RefinementModel& RM;

  TSameGroupList(RefinementModel& parent) : RM(parent) {}
  TSameGroup& New() {
    return Groups.Add(new TSameGroup((uint16_t)Groups.Count(), *this));
  }
  TSameGroup& Build(const olxstr &exp, const olxstr &resi=EmptyString());
  TSameGroup& NewDependent(TSameGroup& on) {
    TSameGroup& rv = Groups.Add(
      new TSameGroup((uint16_t)Groups.Count(), *this));
    on.AddDependent(rv);
    return rv;
  }
  void FixIds();
  TSameGroup& operator [] (size_t i) { return Groups[i]; }
  const TSameGroup& operator [] (size_t i) const { return Groups[i]; }
  size_t Count() const { return Groups.Count(); }
  // searches a group by the content
  TSameGroup *Find(const TSameGroup &g) const;
  void Clear() { Groups.Clear(); }
  void Assign(const TSameGroupList& sl);
  void Release(TSameGroup& sg);
  void Restore(TSameGroup& sg);
  void Delete(const TPtrList <TSameGroup> &groups);
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void SortGroupContent();
  void EndAUSort();

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
