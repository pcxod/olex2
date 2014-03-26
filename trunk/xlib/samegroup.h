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
#include "catom.h"

BeginXlibNamespace()

class TSameGroup : public ACollectionItem {
  TPtrList<TSameGroup> Dependent;  // pointers borrowed from Parent
  TCAtomPList Atoms;
  uint16_t Id;
  class TSameGroupList& Parent;
  TSameGroup* ParentGroup;
protected:
  void SetId(uint16_t id)  {
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetSameId(id);
    Id = id;
  }
  // it does not clear the ParentGroup, to make Restore work...
  void RemoveDependent(TSameGroup& sg)  {
    size_t ind = Dependent.IndexOf(&sg);
    if( ind != InvalidIndex )
      Dependent.Delete(ind);
  }
  // on release
  void ClearAtomIds()  {
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetSameId(~0);
  }
public:
  TSameGroup(uint16_t id, TSameGroupList& parent)
    : Id(id), Parent(parent), ParentGroup(NULL), Esd12(0.02), Esd13(0.02)
  {}
  ~TSameGroup()  {  Clear();  }

  const TSameGroupList& GetParent() const {  return Parent;  }
  TSameGroupList& GetParent() {  return Parent;  }
  TSameGroup* GetParentGroup() const {  return ParentGroup;  }
  int16_t GetId() const {  return Id;  }

  void Assign(class TAsymmUnit& tau, const TSameGroup& sg);

  void Clear()  {
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetSameId(~0);
    Atoms.Clear();
    Dependent.Clear();
  }
  // does not reset the Atom's SameId
  void ReleasedClear()  {
    Atoms.Clear();
    Dependent.Clear();
  }

  bool IsReference() const {  return !Dependent.IsEmpty();  }

  // will invalidate previously assigned group
  TCAtom& Add(TCAtom& ca);

  void AddDependent(TSameGroup& sg)  {
    Dependent.Add(sg);
    sg.ParentGroup = this;
  }

  // compares pointer addresses only!
  bool operator == (const TSameGroup& sr) const {  return this == &sr;  }

  size_t Count() const {  return Atoms.Count();  }
  TCAtom& operator [] (size_t i) {  return *Atoms[i];  }
  const TCAtom& operator [] (size_t i) const {  return *Atoms[i];  }

  size_t DependentCount() const {  return Dependent.Count();  }
  TSameGroup& GetDependent(size_t i) {  return *Dependent[i];  }
  const TSameGroup& GetDependent(size_t i) const {  return *Dependent[i];  }

  bool IsValidForSave() const;
  // returns true if contains only unique atoms
  bool AreAllAtomsUnique() const;

  bool DoOverlap(const TSameGroup &g) const;

  double Esd12, Esd13;

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(PyObject* main, TPtrList<PyObject>& allGroups,
    TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
  // for releasing/restoring items SetId must be called
  friend class TSameGroupList;
};

class TSameGroupList  {
  TTypeList<TSameGroup> Groups;
public:

  class RefinementModel& RM;

  TSameGroupList(RefinementModel& parent) : RM(parent) {}
  TSameGroup& New() {
    return Groups.Add(new TSameGroup((uint16_t)Groups.Count(), *this));
  }
  TSameGroup& NewDependent(TSameGroup& on) {
    TSameGroup& rv = Groups.Add(
      new TSameGroup((uint16_t)Groups.Count(), *this));
    on.AddDependent(rv);
    return rv;
  }
  TSameGroup& operator [] (size_t i)  {  return Groups[i];  }
  const TSameGroup& operator [] (size_t i) const {  return Groups[i];  }
  size_t Count() const {  return Groups.Count();  }
  void Clear()  {  Groups.Clear();  }
  void Assign(TAsymmUnit& tau, const TSameGroupList& sl);
  void Release(TSameGroup& sg);
  void Restore(TSameGroup& sg);

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
