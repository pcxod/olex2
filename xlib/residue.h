/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_residue_H
#define __olx_residue_H
#include "asymmunit.h"

BeginXlibNamespace()

/* number and alias are unique numbers */
class TResidue : public IOlxObject {
public:
  enum {
    NoResidue = -100000
  };
  static olxch NoChainId() {
    return '~';
  }
private:
  TAsymmUnit& Parent;
  uint32_t Id;
  olxch ChainId;
  olxstr ClassName;
  int Number, Alias;
  TCAtomPList Atoms;
protected:
  // removes an atom and resets the ResiId, this is used internally from Add
  void Remove(TCAtom& ca) {
    if (Atoms.Remove(ca)) {
      ca.SetResiId(~0);
    }
  }
  void SetId(uint32_t id) {
    Id = id;
    for (size_t i = 0; i < Atoms.Count(); i++) {
      Atoms[i]->SetResiId(id);
    }
  }
  /* adds an atom to this residue without trying to remove from the owning
 residue (if there is any!)
 */
  void _Add(TCAtom& ca) {
    Atoms.Add(ca);
    ca.SetResiId(Id);
  }
public:
  TResidue(TAsymmUnit& parent, uint32_t id, const olxstr& cl,
    int number, int alias, olxch chainId)
    : Parent(parent), Id(id), ChainId(chainId),
    ClassName(cl), Number(number), Alias(alias)
  {}
  //
  DefPropC(olxstr, ClassName)
  DefPropC(int, Alias)
  DefPropP(int, Number)
  DefPropP(olxch, ChainId)
  bool HasChainId() const { return ChainId != NoChainId(); }
  bool HasAlias() const { return Number != Alias; }
  int Compare(const TResidue &r) const {
    return olx_cmp(GetNumber(), r.GetNumber());
  }
  size_t GetId() const { return Id; }
  TCAtomPList& GetAtomList() { return Atoms; }
  const TCAtomPList& GetAtomList() const { return Atoms; }
  TAsymmUnit& GetParent() { return Parent; }
  virtual TIString ToString() const {
    if (Id == 0) {
      return EmptyString();
    }
    olxstr rv("RESI ");
    rv << ClassName << ' ';
    if (HasChainId()) {
      rv << ChainId << ':';
    }
    rv << Number;
    if (HasAlias()) {
      rv << ' ' << Alias;
    }
    return rv;
  }
  // will prepend chain id if set
  olxstr GetNumberStr() const {
    if (HasChainId()) {
      return olxstr(GetChainId()) << ':' << GetNumber();
    }
    return olxstr(GetNumber());
  }
  size_t Count() const { return Atoms.Count(); }
  TCAtom& GetAtom(size_t i) const { return *Atoms[i]; }
  TCAtom& operator [] (size_t i) const { return *Atoms[i]; }
  void Clear() {
    for (size_t i = 0; i < Atoms.Count(); i++) {
      Atoms[i]->SetResiId(0);
    }
    Atoms.Clear();
  }
  size_t IndexOf(const TCAtom& ca) const {
    return Atoms.IndexOf(ca);
  }
  // removes atom from previous residue and puts into current
  void Add(TCAtom& ca) {
    if (ca.GetResiId() == Id) {
      Atoms.Add(ca);
    }
    else {
      Parent.GetResidue(ca.GetResiId()).Remove(ca);
      Atoms.Add(ca);
      ca.SetResiId(Id);
    }
  }
  void AddAll(const TCAtomPList &atoms) {
    Atoms.SetCapacity(Atoms.Count() + atoms.Count());
    for (size_t i = 0; i < atoms.Count(); i++) {
      Add(*atoms[i]);
    }
  }

  void SetCapacity(size_t c) { Atoms.SetCapacity(c); }
  bool IsEmpty() const {
    for (size_t i = 0; i < Atoms.Count(); i++) {
      if (!Atoms[i]->IsDeleted()) {
        return false;
      }
    }
    return true;
  }
  TResidue* Next() const { return Parent.NextResidue(*this); }
  TResidue* Prev() const { return Parent.PrevResidue(*this); }

  static bool IsValidNumber(const olxstr &rn) {
    size_t idx = rn.IndexOf(':');
    if (idx == InvalidIndex) {
      return rn.IsNumber();
    }
    if (idx == 0) {
      return rn.SubStringFrom(idx+1).IsNumber();
    }
    if (idx == 1) {
      return olxstr::o_isalphanumeric(rn.CharAt(0)) &&
        rn.SubStringFrom(idx + 1).IsNumber();
    }
    return false;
  }
  friend class TAsymmUnit;
};

typedef TPtrList<TResidue> ResiPList;

EndXlibNamespace()

#endif
