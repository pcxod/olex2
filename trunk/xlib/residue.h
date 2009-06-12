#ifndef __olx_residue_H
#define __olx_residue_H
#include "asymmunit.h"

BeginXlibNamespace()

class TResidue : public IEObject  {
  olxstr ClassName, Alias;
  int Number, Id;
  TCAtomPList Atoms;
  TAsymmUnit& Parent;
protected:
  // removes an atom and resets the ResiId, this is used internally from Add
  inline void Remove(TCAtom& ca)           {
    int i = Atoms.IndexOf(ca);
    if( i != -1 )  {
      ca.SetResiId(-1);
      Atoms.Delete(i);
    }
  }
  // adds an atom to this residue without trying to remove from the owning residue (if there is any!)
  inline void _Add(TCAtom& ca) {
    Atoms.Add(ca);
    ca.SetResiId(Id);
  }
public:
  TResidue(TAsymmUnit& parent, int id, const olxstr& cl=EmptyString, int number = 0, const olxstr& alias=EmptyString) : 
      Parent(parent), 
      Id(id), 
      ClassName(cl), 
      Number(number), 
      Alias(alias)  {  }
  //
  DefPropC(olxstr, ClassName)
  DefPropC(olxstr, Alias)
  DefPropP(int, Number)
  inline int GetId()          const {  return Id;  }
  TCAtomPList& GetAtomList()        {  return Atoms;  }
  const TCAtomPList& GetAtomList() const {  return Atoms;  }
  inline TAsymmUnit& GetParent()    {  return Parent;  }
  virtual TIString ToString() const {
    if( Id == -1 )  return EmptyString;
    olxstr rv("RESI ");
    rv << ClassName;
    if( Number != 0 )  rv << ' ' << Number;
    return (rv << (Alias.IsEmpty() ? EmptyString : (olxstr(' ') << Alias)));
  }
  inline int Count()                 const {  return Atoms.Count();  }  
  inline TCAtom& GetAtom(int i)      const {  return *Atoms[i];  }
  inline TCAtom& operator [] (int i) const {  return *Atoms[i]; }
  inline void Clear()                      {  
    for( int i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetResiId(-1);
    Atoms.Clear();  
  } 
  int IndexOf(const TCAtom& ca) const {  return Atoms.IndexOf(ca);  }
  // removes atom from previous residue and puts into current
  inline void Add(TCAtom& ca) {
    Parent.GetResidue(ca.GetResiId()).Remove(ca);
    Atoms.Add(ca);
    ca.SetResiId(Id);
  }
  inline void SetCapacity(int c)  {  Atoms.SetCapacity(c);  }
  bool IsEmpty() const {
    for( int i=0; i < Atoms.Count(); i++ )
      if( !Atoms[i]->IsDeleted() )  return false;
    return true;
  }
  inline TResidue* Next() const {  return Parent.NextResidue(*this);  }
  inline TResidue* Prev() const {  return Parent.PrevResidue(*this);  }
  friend class TAsymmUnit;
};

typedef TPtrList<TResidue> ResiPList;

EndXlibNamespace()

#endif

