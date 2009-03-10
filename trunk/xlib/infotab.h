/* implementation of the HTAB and RTAB for management of deleted atoms etc */
#ifndef _olx_info_tab_H
#define _olx_info_tab_H

#include "refmodel.h"

BeginXlibNamespace()

const short 
  infotab_htab = 1,
  infotab_rtab = 2;

class InfoTab : public IEObject {  // need to cast to delete
  olxstr ResiName, ParamName;
  TCAtomGroup atoms;
  RefinementModel& RM;
  short Type;
public:
  InfoTab(RefinementModel& rm, short type, const olxstr& paramName=EmptyString, const olxstr& resiName=EmptyString) : 
      RM(rm), Type(type), ParamName(paramName), ResiName(resiName) {  }
  
  InfoTab(RefinementModel& rm, const InfoTab& it) : RM(rm)  {  
    this->operator = (it);
  }
  virtual ~InfoTab()  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].GetMatrix() != NULL ) 
        RM.RemUsedSymm(*atoms[i].GetMatrix());
    }
  }
      
  bool operator == (const InfoTab& it) const {
    if( ResiName != it.ResiName )  return false;
    if( Type != it.Type )  return false;
    if( atoms.Count() != it.atoms.Count() )  return false;
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].GetAtom()->GetId() != it.atoms[i].GetAtom()->GetId() )
        return false;
      if( atoms[i].GetMatrix() != NULL && it.atoms[i].GetMatrix() != NULL )  {
        if( !(*atoms[i].GetMatrix() == *it.atoms[i].GetMatrix()) )
          return false;
      }
      else if( atoms[i].GetMatrix() == NULL && it.atoms[i].GetMatrix() == NULL )
        ;
      else
        return false;
    }
    return true;
  }

  InfoTab& operator = (const InfoTab& it)  {
    ResiName = it.ResiName;
    ParamName = it.ParamName;
    Type = it.Type;
    for( int i=0; i < it.atoms.Count(); i++ )  {
      if( it.atoms[i].GetMatrix() != NULL )
        atoms.Add( new TGroupCAtom(&RM.aunit.GetAtom(it.atoms[i].GetAtom()->GetId()), &RM.AddUsedSymm(*it.atoms[i].GetMatrix()) ) );
      else
        atoms.Add( new TGroupCAtom(&RM.aunit.GetAtom( it.atoms[i].GetAtom()->GetId()) ) );
    }
    return *this;
  }

  void AssignAtoms( const TCAtomGroup& ag )  {
    atoms.Clear();
    for( int i=0; i < ag.Count(); i++ )
      atoms.AddNew( ag[i] );
  }
  void AddAtom(TCAtom* ca, const smatd* sm)  {
    atoms.Add( new TGroupCAtom(ca, (sm == NULL ? NULL : &RM.AddUsedSymm(*sm))) );
  }

  int Count() const {  return atoms.Count();  }
  TGroupCAtom& GetAtom(int i) {  return atoms[i];  }
  const TGroupCAtom& GetAtom(int i) const {  return atoms[i];  }

  short GetType() const {  return Type;  }

  bool HasDeletedAtom() const {
    for( int i=0; i < atoms.Count(); i++ )
      if( atoms[i].GetAtom()->IsDeleted() )
        return true;
    return false;
  }
  bool IsValid() const {
    int ac = 0;
    for( int i=0; i < atoms.Count(); i++ )
      if( !atoms[i].GetAtom()->IsDeleted() )
        ac++;
    if( Type == infotab_htab && ac == 2 )  return true;
    if( Type == infotab_rtab && ac >= 2 && ac <= 4 && !ParamName.IsEmpty() )  return true;
    return false;
  }

  olxstr InsStr() const {
    olxstr rv = (Type == infotab_htab ? "HTAB" : "RTAB");
    if( !ResiName.IsEmpty() )
      rv << '_' << ResiName;
    rv << ' ' << ParamName;
    for( int i=0; i < atoms.Count(); i++ )  {
      if( !atoms[i].GetAtom()->IsDeleted() )
        rv << ' ' << atoms[i].GetFullLabel(RM);
    }
    return rv;
  }
};
EndXlibNamespace()

#endif
