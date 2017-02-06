/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_liner_eq_h
#define __olx_liner_eq_h
#include "xbase.h"
#include "evalue.h"
#include "eset.h"
#include "rm_base.h"
#include "typelist.h"
#include "dataitem.h"
#ifdef _PYTHON
  #include "pyext.h"
#endif

BeginXlibNamespace()

const short  // relation of parameters to variables
  relation_None          = 0,  // fixed param
  relation_AsVar         = 1,
  relation_AsOneMinusVar = 2,
  relation_Last          = 2;  // for iterations

/* Any single variable can be shared by several atom parameters (to make them
equal) as well as by several linear equations
*/

class XLEQ;
class XVar;
class XVarManager;
struct XVarReference;

//class IXVar

struct XVarReference {
protected:
  size_t Id;
public:
  XVar& Parent;
  IXVarReferencer& referencer;
  short var_index;  // one of the var_name
  short relation_type; // relationAsVar or relation_AsOneMinusVar
  double coefficient; // like 0.25 in 20.25
  XVarReference(XVar& parent, IXVarReferencer& r, short _var_index,
    short _relation_type, double coeff=1.0) :
    Parent(parent),
    referencer(r),
    var_index(_var_index),
    relation_type(_relation_type),
    coefficient(coeff) {}
  DefPropP(size_t, Id)
  // returns the value of the atom parameter associated with this reference
  double GetActualValue() const {  return referencer.GetValue(var_index);  }
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& referrers);
#endif
  // returns a new instance created with new
  static XVarReference& FromDataItem(const TDataItem& item, XVar& parent);
};

class XVar :public IOlxObject {
  double Value, Esd;
  TPtrList<XVarReference> References;  // owed from the Parent
  TPtrList<XLEQ> Equations; // equations using this variable
  size_t Id;
public:
  XVarManager& Parent;

  XVar(XVarManager& parent, double val=0.5, double esd=0)
    : Value(val), Esd(esd), Id(InvalidIndex),
    Parent(parent)
  {}
  // adds a new atom, referencing this variable, for internal use
  XVarReference& _AddRef(XVarReference& vr)  {  return *References.Add(&vr);  }
  // removes a refence, for internal use
  void _RemRef(XVarReference& vr)  {  References.Remove(&vr);  }
  size_t _RefCount()  const {  return References.Count();  }
  /* calculates the number of atoms (excluding the deleted ones) using this
  variable
  */
  size_t RefCount() const;
  XVarReference& GetRef(size_t i) const {  return *References[i];  }
  const TPtrList<XVarReference> & GetRefs() const { return References; }

  // adds a new equation, referencing this variable
  void _AddLeq(XLEQ& eq)  {  Equations.Add(&eq);  }
  // removes an equation, referencing this variable
  void _RemLeq(XLEQ& eq)  {  Equations.Remove(&eq);  }
  // returns the number of equations, referencing this variable
  size_t LeqCount() const {  return Equations.Count();  }
  const TPtrList<XLEQ> & GetLEQs() const { return Equations; }

  bool IsUsed() const;
  DefPropP(double, Value)
  DefPropP(double, Esd)
  DefPropP(size_t, Id)

  bool IsReserved() const;
  TIString ToString() const;
  void Update(const TEValueD &v);
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  static XVar& FromDataItem(const TDataItem& item, XVarManager& parent);
};

class XLEQ : public IOlxObject {
  double Value, Sigma;
  TDoubleList Coefficients;
  TPtrList<XVar> Vars;
  size_t Id;
public:
  XVarManager& Parent;

  XLEQ(XVarManager& parent, double val=1.0, double sig=0.01)
    :  Value(val), Sigma(sig), Parent(parent)
  {}
  ~XLEQ();
  // copies Coefficients and Vars, internal use
  void _Assign(const XLEQ& leq);
  void AddMember(XVar& var, double coefficient = 1.0);
  size_t Count() const {  return Vars.Count();  }
  const XVar& operator [] (size_t i) const {  return *Vars[i];  }
  XVar& operator [] (size_t i) {  return *Vars[i];  }
  double GetCoefficient(size_t i) const {  return Coefficients[i];  }
  // validates that the equation is valid, if not - releases the variables
  bool Validate();
  DefPropP(double, Value)
  DefPropP(double, Sigma)
  DefPropP(size_t, Id)
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& vars);
#endif
  static XLEQ& FromDataItem(const TDataItem& item, XVarManager& parent);
};

class XVarManager {
  TTypeList<XVar> Vars, ReservedVars;
  TTypeList<XLEQ> Equations;
  TTypeList<XVarReference> References;
  uint16_t NextVar;  // this controls there variables go in sebsequent calls
  void UpdateIds();
  void RemoveReservedVar(XVar &v,
    olxset<XLEQ *, TPointerComparator> &leq_to_remove,
    olxset<XVarReference *, TPointerComparator> &ref_to_remove);
  void FinaliseReservedVarRemoval(
    olxset<XLEQ *, TPointerComparator> &leq_to_remove,
    olxset<XVarReference *, TPointerComparator> &ref_to_remove);
public:

  class RefinementModel& RM;

  static olxstr RelationNames[];
  static short RelationIndex(const olxstr& rn);

  XVarManager(RefinementModel& rm);

  XVar& NewVar(double val = 0.5, bool reindex=true);
  /* returns existing variable or creates a new one. Sets a limit of 1024
  variables
  */
  XVar& GetReferencedVar(size_t ind);
  size_t VarCount() const {  return Vars.Count();  }
  const XVar& GetVar(size_t i) const {  return Vars[i];  }
  XVar& GetVar(size_t i)  {  return Vars[i];  }

  /* For internal use - returns an instance form Vars or Reservedvars
  according to the id
  */
  XVar& GetVar_(size_t id);
  bool IsReserved(const XVar &v) const {
    return v.GetId() >= Vars.Count();
  }

  olxstr getReservedVarName(size_t i) const;

  XLEQ& NewEquation(double val = 1.0, double sig = 0.01);
  size_t EquationCount() const {  return Equations.Count();  }
  const XLEQ& GetEquation(size_t i) const {  return Equations[i];  }
  XLEQ& GetEquation(size_t i)  {  return Equations[i];  }
  XLEQ& ReleaseEquation(size_t i);

  size_t VarRefCount() const {  return References.Count();  }
  XVarReference& GetVarRef(size_t i) {  return References[i];  }
  const XVarReference& GetVarRef(size_t i) const {  return References[i];  }
  // clears all the data and Nulls atoms' varrefs
  void ClearAll();
  void Clear() {  // does not clear the data, just resets the NextVar
    NextVar = 0;  // the global scale factor
    Equations.Clear(); // these are recreatable
  }
  /* sets a relation between an atom parameter and a variable, if the
  coefficient is -10 (default value), the position multiplicity is taken into
  account
  */
  XVarReference& AddVarRef(XVar& var, IXVarReferencer& r, short var_name,
    short relation, double coefficient);
  // for parsing...
  XVarReference& AddVarRef(XVarReference& ref) {
    ref.SetId(References.Count());
    References.Add(ref);
    return ref;
  }
  // releases a reference to the variable, must be deleted, unless restored
  XVarReference* ReleaseRef(IXVarReferencer& r, short var_name);
  // restrores previously released var reference
  void RestoreRef(IXVarReferencer& r, short var_name, XVarReference* vr);
  // removes all unused variables and invalid/incomplete equations
  void Validate();
  /* helps with parsing SHELX specific paramter representation, returns actual
  value of the param
  */
  double SetParam(IXVarReferencer& r, short param_name, double val);
  void FixParam(IXVarReferencer& r, short param_name);
  void FreeParam(IXVarReferencer& r, short param_name);
  // retruns a SHELX specific value like 20.5 or 11.0
  double GetParam(const IXVarReferencer& r, short param_name) const {
    return GetParam(r, param_name, r.GetValue(param_name));
  }
  // uses the override_val instead of the call to GetValue(param_name)
  double GetParam(const IXVarReferencer& r, short param_name,
    double override_val) const;
  // parses FVAR and assignes variable values
  void AddFVAR(const TStrList &fvar);
  bool HasEXTI() const;
  const XVar &GetEXTI() const;
  void SetEXTI(double val, double esd);
  XVar &GetEXTI();
  void ClearEXTI();

  void SetBASF(const TStrList &bs);
  void ClearBASF();
  size_t GetBASFCount() const;
  bool HasBASF() const { return GetBASFCount() > 0; }
  const XVar &GetBASF(size_t i) const;
  XVar &GetBASF(size_t i);

  olxstr GetFVARStr() const;
  void AddSUMP(const TStrList &sump);
  // Validate must be called first to get valid count of equations
  olxstr GetSUMPStr(size_t ind) const;
  void Assign(const XVarManager& vm);

  void Describe(TStrList& lst);

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(const TDataItem& item);
};


EndXlibNamespace()

#endif
