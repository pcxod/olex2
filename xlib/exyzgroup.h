#ifndef __olx_exyz_group_H
#define __olx_exyz_group_H
#include "catom.h"
BeginXlibNamespace()

class TExyzGroup {
  int Id;
  TCAtomPList Atoms;
  TExyzGroups& Parent;
public:
  TExyzGroup(TExyzGroups& parent, int id) : Parent(parent), Id(id) {  }
  ~TExyzGroup()  { 
    for( int i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetExyzGroup(NULL);
  }
  DefPropP(int, Id)
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
  TCAtom& operator [] (int i) {  return *Atoms[i];  }
  const TCAtom& operator [] (int i) const {  return *Atoms[i];  }
  int Count() const {  return Atoms.Count();  }
  bool IsEmpty() const {
    int ac = 0;
    for( int i=0; i < Atoms.Count(); i++ )
      if( !Atoms[i]->IsDeleted() ) ac++;
    return (ac < 2);
  }
  void Assign(const TExyzGroup& ags);
  void Clear();
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};
//....................................................................................
class TExyzGroups {
  TTypeList<TExyzGroup> Groups;
public:
  
  class RefinementModel& RM;

  TExyzGroups(RefinementModel& parent) : RM(parent) {}

  TExyzGroup& New() {  return Groups.Add( new TExyzGroup(*this, Groups.Count()) );  }
  void Clear() {  Groups.Clear();  }
  int Count() const {  return Groups.Count();  }
  TExyzGroup& operator [] (int i) {  return Groups[i];  }
  const TExyzGroup& operator [] (int i) const {  return Groups[i];  }
  void Delete(int i)  {
    Groups.Delete(i);
    for( int j=i; j < Groups.Count(); j++ )
      Groups[j].SetId(j);
  }

  void Assign(const TExyzGroups& ags)  {
    Clear();
    for( int i=0; i < ags.Count(); i++ )  {
      if( ags[i].IsEmpty() )  continue;
      Groups.Add( new TExyzGroup(*this, Groups.Count()) ).Assign(ags[i]);
    }
  }
  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
