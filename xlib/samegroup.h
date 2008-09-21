#ifndef __same_group_h
#define __same_group_h

#include "catom.h"

BeginXlibNamespace()

class TSameGroup {
  TPtrList<TSameGroup> Dependent;  // pointers borrowed from Parent
  TCAtomPList Atoms;
  int Id;
  class TSameGroupList& Parent;
public:
  TSameGroup(int id, TSameGroupList& parent) : Id(id), Parent(parent)  { 
    Esd12 = 0.02;
    Esd13 = 0.04;
  }
  ~TSameGroup()  {  Clear();  }

  void Assign(class TAsymmUnit& tau, const TSameGroup& sg);
  
  void Clear()  {
    for( int i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetSameId(-1);
    Atoms.Clear();
    Dependent.Clear();
  }
  
  bool IsReference() const {  return !Dependent.IsEmpty();  }
  
  TCAtom& Add(TCAtom& ca)  {  
    ca.SetSameId(Id);
    Atoms.Add(&ca);
    return ca;
  }
  
  void AddDependent(TSameGroup& sg)  {
    Dependent.Add( &sg );
  }
  
  int Count() const {  return Atoms.Count();  }
  TCAtom& operator [] (int i) {  return *Atoms[i];  }
  const TCAtom& operator [] (int i) const {  return *Atoms[i];  }

  int DependentCount() const {  return Dependent.Count();  }
  TSameGroup& GetDependent(int i) {  return *Dependent[i];  }
  const TSameGroup& GetDependent(int i) const {  return *Dependent[i];  }

  double Esd12, Esd13;
};

class TSameGroupList  {
  TTypeList<TSameGroup> Groups;
public:
  TSameGroup& New() {  return Groups.Add(new TSameGroup(Groups.Count(), *this));  }
  TSameGroup& NewDependent(TSameGroup& on) {  
    TSameGroup& rv = Groups.Add( new TSameGroup(Groups.Count(), *this) ); 
    on.AddDependent(rv);
    return rv;
  }
  TSameGroup& operator [] (int i)  {  return Groups[i];  }
  const TSameGroup& operator [] (int i) const {  return Groups[i];  }
  int Count() const {  return Groups.Count();  }
  void Clear()  {  Groups.Clear();  }
  void Assign(TAsymmUnit& tau, const TSameGroupList& sl)  {
    Clear();
    for( int i=0; i < sl.Groups.Count(); i++ )
      New();
    for( int i=0; i < sl.Groups.Count(); i++ )
      Groups[i].Assign( tau, sl.Groups[i] );
  }
};

EndXlibNamespace()
#endif
