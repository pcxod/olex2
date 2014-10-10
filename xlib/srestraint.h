/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_srestraint_H
#define __olx_xl_srestraint_H
#include "catomlist.h"
#undef AddAtom
BeginXlibNamespace()

class TAsymmUnit;
class RefinementModel;
class TSRestraintList;
// restraint atom list types
const short
  rltNone   = 0, // default value for the constructor...
  rltAtoms  = 1, // set of independent atoms
  rltGroup2 = 2, // set of "bonds" - atom pairs
  rltGroup3 = 3, // set of "angles" - atom triplets
  rltGroup4 = 4, // dihedrals
  rltGroup  = 5; // atoms represent a group

const short
  rptNone   = 0x0000,
  rptValue  = 0x0001,
  rptEsd    = 0x0002,
  rptEsd1   = 0x0004,
  rptValue1 = 0x0008
  ;

class TSimpleRestraint : public IEObject, public IXVarReferencer  {
  TSRestraintList& Parent;
  size_t Id;
  short ListType;
  double Value, Esd, Esd1;
  XVarReference* VarRef;
  bool AllNonHAtoms;
  AtomRefList Atoms;
protected:
  void SetId(size_t id)  {  Id = id;  }
public:
  TSimpleRestraint(TSRestraintList& parent, size_t id, const short listType);
  virtual ~TSimpleRestraint()  {}
  void AddAtoms(const TCAtomGroup& atoms);
  TSimpleRestraint &AddAtom(TCAtom& aa, const smatd* ma);
  TSimpleRestraint &AddAtom(TGroupCAtom& a)  {
    return AddAtom(*a.GetAtom(), a.GetMatrix());
  }
  TSimpleRestraint &AddAtomPair(TCAtom& aa, const smatd* ma,
    TCAtom& ab, const smatd* mb)
  {
    AddAtom(aa, ma);
    return AddAtom(ab, mb);
  }
  void AtomsFromExpression(const olxstr &e, const olxstr &resi=EmptyString());

  const TSRestraintList& GetParent() const {  return Parent;  }
  TSRestraintList& GetParent()  {  return Parent;  }
  const AtomRefList &GetAtoms() const { return Atoms; }
  void Delete();
  bool IsEmpty() const { return Atoms.IsEmpty(); }
  olxstr GetAtomExpression() const { return Atoms.GetExpression(); }
  void UpdateResi() { Atoms.UpdateResi(); }
  TSimpleRestraint &Validate();
  // this is called internally by the RM
  void OnAUUpdate() { Atoms.OnAUUpdate(); }
  void BeginAUSort() { Atoms.BeginAUSort(); }
  void EndAUSort() { Atoms.EndAUSort(); }
  void Sort() { Atoms.SortByTag(TPtrList<AtomRefList>()); }
  // copies data from a restrain, but with atoms from the thisAU
  void Assign(const TSimpleRestraint&);

  short GetListType() const {  return ListType;  }

  size_t GetId() const {  return Id;  }
  DefPropP(double, Value)
  DefPropP(double, Esd)
  DefPropP(double, Esd1)
  DefPropBIsSet(AllNonHAtoms)

  // compares pointer addresses only!
  bool operator == (const TSimpleRestraint& sr) const {  return this == &sr;  }
// IXVarReferencer implementation
  virtual size_t VarCount() const {  return 1;  }
  virtual XVarReference* GetVarRef(size_t var_index) const {
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return VarRef;
  }

  virtual olxstr GetVarName(size_t var_index) const;

  virtual void SetVarRef(size_t var_index, XVarReference* var_ref) {
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    VarRef = var_ref;
  }

  virtual IXVarReferencerContainer& GetParentContainer() const;

  virtual double GetValue(size_t var_index) const {
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return Value;
  }
  virtual void SetValue(size_t var_index, const double& val) {
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    Value = val;
  }
  virtual bool IsValid() const {  return true;  }
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

class TSRestraintList : public IEObject, public IXVarReferencerContainer  {
  TTypeList<TSimpleRestraint> Restraints;
  short RestraintListType;
  RefinementModel& RefMod;
  olxstr IdName;
  short parameters; // a combination of the rptXXX constants
public:
  TSRestraintList(RefinementModel& rm, short restraintListType,
      const olxstr& id_name, short parameters)
    : RestraintListType(restraintListType),
      RefMod(rm), IdName(id_name), parameters(parameters)
  {}
  virtual ~TSRestraintList()  {}
  TSimpleRestraint& AddNew();
  /* function checks uniquesness of the restraint data - previously defined
  values are removed
  */
  void ValidateRestraint(TSimpleRestraint& sr);
  void ValidateAll()  {
    olx_list_call(Restraints, &TSimpleRestraint::Validate);
  }

  TSimpleRestraint& Release(size_t i);
  void Release(TSimpleRestraint& sr);
  void Restore(TSimpleRestraint& sr);

  const RefinementModel& GetRM() const {  return RefMod;  }
  RefinementModel& GetRM()  {  return RefMod;  }
  short GetParameters() const { return parameters; }
  void Assign(const TSRestraintList& rl);
  void Clear();
  size_t Count() const {  return Restraints.Count();  }
  TSimpleRestraint& operator [] (size_t i) const {  return Restraints[i];  }
  short GetRestraintListType() const {  return RestraintListType;  }
  void UpdateResi() {
    for (size_t i=0; i < Restraints.Count(); i++)
      Restraints[i].UpdateResi();
  }
  // this is called internally by the RM
  void OnAUUpdate();
  void BeginAUSort();
  void EndAUSort();
  void SortAtomsByTags();
  // IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const {  return IdName;  }
  virtual size_t GetIdOf(const IXVarReferencer& vr) const {
    if( !EsdlInstanceOf(vr, TSimpleRestraint) )
      throw TInvalidArgumentException(__OlxSourceInfo, "var referencer");
    return ((TSimpleRestraint&)vr).GetId();
  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const {
    return GetIdOf(vr);
  }
  virtual IXVarReferencer& GetReferencer(size_t id) const {
    return Restraints[id];
  }
  virtual size_t ReferencerCount() const {  return Restraints.Count();  }
//
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(const TDataItem& item);
  void FromDataItem(const TDataItem* item)  {
    if( item != NULL )
      FromDataItem(*item);
  }
};

typedef TTypeList<TSimpleRestraint> TSimpleRestraintList;
typedef TPtrList<TSimpleRestraint> TSimpleRestraintPList;

EndXlibNamespace()

#endif
