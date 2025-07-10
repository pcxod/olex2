/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_srestraint_H
#define __olx_xl_srestraint_H
#include "catomlist.h"
#include "releasable.h"
#undef AddAtom
BeginXlibNamespace()

class TAsymmUnit;
class RefinementModel;
class TSRestraintList;
// restraint atom list types
const short
  rltNone    = 0, // default value for the constructor...
  rltAtoms1N = 1, // set of independent atoms
  rltAtoms2N = 2, // set of 2 or more independent atoms
  rltAtoms3N = 3, // set of 3 or more independent atoms
  rltAtoms4N = 4, // set of 4 or more independent atoms

  rltGroup2  = 5, // set of "bonds" - atom pairs
  rltGroup3  = 6, // set of "angles" - atom triplets
  rltGroup4  = 7; // dihedrals

const short
  rptNone   = 0x0000,
  rptValue  = 0x0001,
  rptEsd    = 0x0002,
  rptEsd1   = 0x0004,
  rptValue1 = 0x0008
  ;

class TSAtom;
class TSRestraintList;

class TSimpleRestraint : public IXVarReferencer, public AReleasable {
  size_t Position;
  short ListType;
  double Value, Esd, Esd1;
  XVarReference* VarRef;
  bool AllNonHAtoms;
  AtomRefList Atoms;
public:
  TSimpleRestraint(TSRestraintList& parent, short listType);
  virtual ~TSimpleRestraint() {}

  TSRestraintList& GetParent() const;
  
  void AddAtoms(const TCAtomGroup& atoms);
  TSimpleRestraint &AddAtom(TCAtom& aa, const smatd* ma);
  TSimpleRestraint &AddAtom(TGroupCAtom& a) {
    return AddAtom(*a.GetAtom(), a.GetMatrix());
  }
  TSimpleRestraint &AddAtomPair(TCAtom& aa, const smatd* ma,
    TCAtom& ab, const smatd* mb)
  {
    AddAtom(aa, ma);
    return AddAtom(ab, mb);
  }
  TSimpleRestraint& AddAtomPair(TSAtom& aa, TSAtom& ab);
  void AtomsFromExpression(const olxstr &e, const olxstr &resi = EmptyString());
  void SetAtoms(const TPtrList<TSAtom> &atoms) {
    Atoms.Build(atoms);
  }
  void ConvertToImplicit() { Atoms.ConvertToImplicit(); }

  const AtomRefList &GetAtoms() const { return Atoms; }
  // clears atoms and assiciated VarRef
  void Delete();
  bool IsEmpty() const { return Atoms.IsEmpty(); }
  olxstr GetAtomExpression() const { return Atoms.GetExpression(); }
  void UpdateResi() { Atoms.UpdateResi(); }
  // in the case the atom list becomes empty - clears VarRef
  TSimpleRestraint &Validate();
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort() { Atoms.BeginAUSort(); }
  void EndAUSort();
  void Sort() { Atoms.SortByTag(TPtrList<AtomRefList>()); }
  // copies data from a restrain, but with atoms from the thisAU
  void Assign(const TSimpleRestraint &r);

  short GetListType() const { return ListType; }

  size_t GetGroupSize() const {
    return ListType >= rltGroup2 && ListType <= rltGroup4
      ? ((ListType - rltGroup2) + 2)
      : InvalidIndex;
  }

  DefPropP(size_t, Position);
  DefPropP(double, Value);
  DefPropP(double, Esd);
  DefPropP(double, Esd1);
  DefPropBIsSet(AllNonHAtoms);

  size_t GetId() const { return this->GetReleasableId(); }

  TStrList remarks;

  // compares pointer addresses only!
  bool operator == (const TSimpleRestraint& sr) const { return this == &sr; }
  // IXVarReferencer implementation
  virtual size_t VarCount() const { return 1; }
  // for "released" restraints this must be deleted manually
  virtual XVarReference* GetVarRef(size_t var_index) const {
    if (var_index != 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    return VarRef;
  }
  virtual void OnReleasedDelete() {
    if (GetVarRef(0) != 0) {
      delete GetVarRef(0);
    }
  }

  virtual olxstr GetVarName(size_t var_index) const;

  virtual void SetVarRef(size_t var_index, XVarReference* var_ref) {
    if (var_index != 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    VarRef = var_ref;
  }

  virtual IXVarReferencerContainer& GetParentContainer() const;

  virtual double GetValue(size_t var_index) const {
    if (var_index != 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    return Value;
  }
  virtual void SetValue(size_t var_index, const double& val) {
    if (var_index != 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    Value = val;
  }
  virtual bool IsValid() const {
    return Atoms.IsEmpty() ? AllNonHAtoms : true;
  }
  virtual olxstr GetIdName() const;
  void ToDataItem(TDataItem& item) const;
  virtual TIString ToString() const;
#ifdef _PYTHON
  ConstPtrList<PyObject> PyExport(TPtrList<PyObject>& atoms,
    TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(const TDataItem& item);
  friend class TSRestraintList;
};

class TSRestraintList : public IXVarReferencerContainer,
  public AReleasableContainer<TSimpleRestraint>
{
  short RestraintListType;
  RefinementModel& RefMod;
  olxstr IdName;
  short parameters; // a combination of the rptXXX constants
  bool AllowSymm;
  virtual void OnRestore(TSimpleRestraint& item);
  virtual void OnRelease(TSimpleRestraint& item);
public:
  TSRestraintList(RefinementModel& rm, short restraintListType,
    const olxstr& id_name, short parameters, bool allow_symm)
    : RestraintListType(restraintListType),
    RefMod(rm), IdName(id_name), parameters(parameters), AllowSymm(allow_symm)
  {}
  virtual ~TSRestraintList() {}
  TSimpleRestraint& AddNew();
  /* function checks uniquesness of the restraint data - previously defined
  values are removed
  */
  void ValidateRestraint(TSimpleRestraint& sr);
  void ValidateAll();
  bool DoAllowSymmetry() const {
    return AllowSymm;
  }

  const RefinementModel& GetRM() const { return RefMod; }
  RefinementModel& GetRM() { return RefMod; }
  short GetParameters() const { return parameters; }
  void Assign(const TSRestraintList& rl);
  void Clear();
  short GetRestraintListType() const { return RestraintListType; }
  void UpdateResi();
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void EndAUSort();
  void SortAtomsByTags();
  // IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const { return IdName; }
  virtual size_t GetIdOf(const IXVarReferencer& vr) const {
    if (!vr.Is<TSimpleRestraint>()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var referencer");
    }
    return dynamic_cast<const TSimpleRestraint&>(vr).GetReleasableId();
  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const {
    return GetIdOf(vr);
  }
  virtual IXVarReferencer& GetReferencer(size_t id) const {
    return dynamic_cast<IXVarReferencer&>(GetItem(id));
  }
  virtual size_t ReferencerCount() const { return items.Count(); }
  //
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(const TDataItem& item);
  void FromDataItem(const TDataItem* item) {
    if (item != 0) {
      FromDataItem(*item);
    }
  }
};

typedef TTypeList<TSimpleRestraint> TSimpleRestraintList;
typedef TPtrList<TSimpleRestraint> TSimpleRestraintPList;

EndXlibNamespace()

#endif
