/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_exyz_group_H
#define __olx_exyz_group_H
#include "catom.h"
BeginXlibNamespace()

class TExyzGroup {
  TExyzGroups& Parent;
  size_t Id;
  TCAtomPList Atoms;
public:
  TExyzGroup(TExyzGroups& parent, size_t id) : Parent(parent), Id(id) {  }
  ~TExyzGroup()  { 
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetExyzGroup(NULL);
  }
  DefPropP(size_t, Id)
  TCAtom& Add(TCAtom& ca)  {
    if( ca.GetExyzGroup() != NULL )
      ca.GetExyzGroup()->Remove(ca);
    ca.SetExyzGroup(this);
    Atoms.Add(&ca);
    return ca;
  }
  void Remove(TCAtom& ca)  {
    Atoms.Remove(&ca);
    ca.SetExyzGroup(NULL);
  }
  TCAtom& operator [] (size_t i) {  return *Atoms[i];  }
  const TCAtom& operator [] (size_t i) const {  return *Atoms[i];  }
  size_t Count() const {  return Atoms.Count();  }
  bool IsEmpty() const {
    size_t ac = 0;
    for( size_t i=0; i < Atoms.Count(); i++ )
      if( !Atoms[i]->IsDeleted() && Atoms[i]->GetExyzGroup() == this ) 
        ac++;
    return (ac < 2);
  }
  void Assign(const TExyzGroup& ags);
  void Clear();
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};
//..............................................................................
class TExyzGroups {
  TTypeList<TExyzGroup> Groups;
public:
  class RefinementModel& RM;

  TExyzGroups(RefinementModel& parent) : RM(parent) {}

  TExyzGroup& New() {
    return Groups.Add(new TExyzGroup(*this, Groups.Count()));
  }
  void Clear() {  Groups.Clear();  }
  size_t Count() const {  return Groups.Count();  }
  TExyzGroup& operator [] (size_t i) {  return Groups[i];  }
  const TExyzGroup& operator [] (size_t i) const {  return Groups[i];  }
  void Delete(size_t i)  {
    Groups.Delete(i);
    for( size_t j=i; j < Groups.Count(); j++ )
      Groups[j].SetId(j);
  }

  void Assign(const TExyzGroups& ags)  {
    Clear();
    for( size_t i=0; i < ags.Count(); i++ )  {
      if( ags[i].IsEmpty() )  continue;
      Groups.Add( new TExyzGroup(*this, Groups.Count()) ).Assign(ags[i]);
    }
  }
  void ValidateAll() {
    for( size_t i=0; i < Groups.Count(); i++ )
      if( Groups[i].IsEmpty() )
        Groups.NullItem(i);
    Groups.Pack();
  }
  void ToDataItem(TDataItem& item);
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
