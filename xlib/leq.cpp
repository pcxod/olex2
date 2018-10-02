/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "leq.h"
#include "refmodel.h"
#include "srestraint.h"
#include "xapp.h"

olxstr XVarManager::RelationNames[] = {"None", "var", "one_minus_var"};

//.............................................................................
void XVarReference::ToDataItem(TDataItem& item) const {
  item.AddField("var_index", var_index);
  item.AddField("id_name", referencer.GetParentContainer().GetIdName());
  item.AddField("owner_id", referencer.GetPersistentId());
  item.AddField("k", coefficient);
  item.AddField("rel", XVarManager::RelationNames[relation_type]);
}
//.............................................................................
#ifdef _PYTHON
PyObject* XVarReference::PyExport(TPtrList<PyObject>& atoms)  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "name",
    PythonExt::BuildString(referencer.GetParentContainer().GetIdName()));
  PythonExt::SetDictItem(main, "id",
    Py_BuildValue("i", referencer.GetPersistentId()));
  PythonExt::SetDictItem(main, "index", Py_BuildValue("i", var_index));
  PythonExt::SetDictItem(main, "relation",
    PythonExt::BuildString(XVarManager::RelationNames[relation_type]));
  PythonExt::SetDictItem(main, "k", Py_BuildValue("d", coefficient));
  return main;
}
#endif
//.............................................................................
XVarReference& XVarReference::FromDataItem(const TDataItem& item,
  XVar& parent)
{
  IXVarReferencerContainer& rc = parent.Parent.RM.GetRefContainer(
    item.GetFieldByName("id_name"));
  IXVarReferencer& ref = rc.GetReferencer(
    item.GetFieldByName("owner_id").ToSizeT());
  return *(new XVarReference(parent, ref,
    item.GetFieldByName("var_index").ToInt(),
    XVarManager::RelationIndex(item.GetFieldByName("rel")),
    item.GetFieldByName("k").ToDouble()));
}
//.............................................................................
//.............................................................................
//.............................................................................
size_t XVar::RefCount() const {
  size_t rv = 0;
  for (size_t i = 0; i < References.Count(); i++) {
    if (References[i]->referencer.IsValid()) {
      rv++;
    }
  }
  return rv;
}
//.............................................................................
void XVar::ToDataItem(TDataItem& item) const {
  item.AddField("val", TEValueD(Value, Esd).ToString());
  for (size_t i = 0; i < References.Count(); i++) {
    if (References[i]->referencer.IsValid()) {
      References[i]->ToDataItem(item.AddItem("reference"));
    }
  }
}
//.............................................................................
#ifdef _PYTHON
PyObject* XVar::PyExport(TPtrList<PyObject>& atoms) {
  size_t rc = 0;
  for (size_t i = 0; i < References.Count(); i++) {
    if (References[i]->referencer.IsValid()) {
      rc++;
    }
  }
  PyObject* main = PyDict_New(),
    *refs = PyTuple_New(rc);
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));
  if (rc != 0) {
    rc = 0;
    for (size_t i = 0; i < References.Count(); i++) {
      if (References[i]->referencer.IsValid()) {
        PyTuple_SetItem(refs, rc++, References[i]->PyExport(atoms));
      }
    }
  }
  PythonExt::SetDictItem(main, "references", refs);
  return main;
}
#endif
//.............................................................................
XVar& XVar::FromDataItem(const TDataItem& item, XVarManager& parent) {
  TEValueD v = item.GetFieldByName("val");
  XVar* var = new XVar(parent, v.GetV(), v.GetE());
  for (size_t i=0; i < item.ItemCount(); i++ ) {
    XVarReference& rf = XVarReference::FromDataItem(item.GetItemByIndex(i), *var);
    parent.AddVarRef(rf);
    var->References.Add(&rf);
  }
  return *var;
}
//.............................................................................
bool XVar::IsUsed() const {
  if (IsReserved()) {
    return true;
  }
  const size_t rc = RefCount();
  if (LeqCount() == 0) {
    if (rc == 1) {
      if (TXApp::DoPreserveFVARs() ||
        References[0]->referencer.Is<TSimpleRestraint>())
      {
        return true;
      }
    }
    return rc > 1;
  }
  return rc > 0;
}
//.............................................................................
bool XVar::IsReserved() const {
  return Parent.IsReserved(*this);
}
//.............................................................................
void XVar::Update(const TEValueD &x) {
  Value = x.GetV();
  Esd = x.GetE();
}
//.............................................................................
TIString XVar::ToString() const {
  return TEValueD(Value, Esd).ToString();
}
//.............................................................................
//.............................................................................
//.............................................................................
XLEQ::~XLEQ() {
  for (size_t i = 0; i < Vars.Count(); i++) {
    Vars[i]->_RemLeq(*this);
  }
}
//.............................................................................
bool XLEQ::Validate() {
  size_t vc = 0;
  for (size_t i = 0; i < Vars.Count(); i++) {
    if (Vars[i]->IsUsed()) {
      vc++;
    }
    else {
      Vars[i]->_RemLeq(*this);
      Vars.Delete(i);
      i--;
    }
  }
  if (vc < 1) {
    for (size_t i = 0; i < Vars.Count(); i++) {
      Vars[i]->_RemLeq(*this);
    }
    Vars.Clear();
  }
  return vc >= 1;
}
//.............................................................................
void XLEQ::_Assign(const XLEQ& leq) {
  for (size_t i = 0; i < leq.Vars.Count(); i++) {
    AddMember(Parent.GetVar_(leq[i].GetId()), leq.Coefficients[i]);
  }
  Value = leq.Value;
  Sigma = leq.Sigma;
}
//.............................................................................
void XLEQ::AddMember(XVar& var, double coefficient) {
  Vars.Add(var);
  Coefficients.Add(coefficient);
  var._AddLeq(*this);
}
//.............................................................................
void XLEQ::ToDataItem(TDataItem& item) const {
  item.AddField("val", Value);
  item.AddField("sig", Sigma);
  for (size_t i = 0; i < Vars.Count(); i++) {
    TDataItem& mi = item.AddItem("var");
    mi.AddField("id", Vars[i]->GetId());
    mi.AddField("k", Coefficients[i]);
  }
}
//.............................................................................
#ifdef _PYTHON
PyObject* XLEQ::PyExport(TPtrList<PyObject>& _vars) {
  for (size_t i = 0; i < Vars.Count(); i++) {
    if (Vars[i]->IsReserved()) {
      return 0;
    }
  }
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));
  PythonExt::SetDictItem(main, "sigma", Py_BuildValue("d", Sigma));
  PyObject* vars = PyTuple_New(Vars.Count());
  for (size_t i = 0; i < Vars.Count(); i++) {
    Py_IncRef(_vars[Vars[i]->GetId()]);
    PyObject* var = PyTuple_New(2);
    PyTuple_SetItem(var, 0, _vars[Vars[i]->GetId()]);
    PyTuple_SetItem(var, 1, Py_BuildValue("d", Coefficients[i]));
    PyTuple_SetItem(vars, i, var);
  }
  PythonExt::SetDictItem(main, "variables", vars);
  return main;
}
#endif
//.............................................................................
XLEQ& XLEQ::FromDataItem(const TDataItem& item, XVarManager& parent) {
  XLEQ* leq = new XLEQ(parent, item.GetFieldByName("val").ToDouble(),
    item.GetFieldByName("sig").ToDouble());
  for (size_t i = 0; i < item.ItemCount(); i++) {
    const TDataItem& mi = item.GetItemByIndex(i);
    leq->AddMember(parent.GetVar_(mi.GetFieldByName("id").ToSizeT()),
      mi.GetFieldByName("k").ToDouble());
  }
  return *leq;
}
//.............................................................................
//.............................................................................
//.............................................................................
XVarManager::XVarManager(RefinementModel& rm) : RM(rm) {
  NextVar = 0;
  NewVar(1.0).SetId(0);
  // unset EXTI
  ReservedVars.Add((XVar *)0);
}
//.............................................................................
void XVarManager::ClearAll() {
  Clear();
  Vars.Clear();
  ReservedVars.Clear();
  References.Clear();
  RM.ClearVarRefs();
}
//.............................................................................
void XVarManager::Assign(const XVarManager& vm) {
  ClearAll();
  for (size_t i = 0; i < vm.Vars.Count(); i++) {
    NewVar(vm.Vars[i].GetValue()).SetEsd(vm.Vars[i].GetEsd());
  }
  ReservedVars.SetCapacity(vm.ReservedVars.Count());
  for (size_t i = 0; i < vm.ReservedVars.Count(); i++) {
    if (i + 1 == vm.ReservedVars.Count() && vm.ReservedVars.IsNull(i)) {
      ReservedVars.Add((XVar *)0);
      break;
    }
    XVar *v = new XVar(*this, vm.ReservedVars[i].GetValue());
    v->SetEsd(vm.ReservedVars[i].GetEsd());
    v->SetId(Vars.Count() + i);
    ReservedVars.Add(v);
  }
  for( size_t i=0; i < vm.References.Count(); i++ )  {
    XVarReference& vr = vm.References[i];
    IXVarReferencerContainer& rc =
      RM.GetRefContainer(vr.referencer.GetParentContainer().GetIdName());
    IXVarReferencer& xvr = rc.GetReferencer(vr.referencer.GetReferencerId());
    AddVarRef(GetVar_(vr.Parent.GetId()),
      xvr, vr.var_index, vr.relation_type, vr.coefficient);
  }
  for (size_t i = 0; i < vm.Equations.Count(); i++) {
    NewEquation()._Assign(vm.Equations[i]);
  }
  if (Vars.IsEmpty()) { // odd eh?
    NewVar(1.0).SetId(0);
  }
  if (ReservedVars.IsEmpty()) { // odd eh?
    ReservedVars.Add((XVar *)0);
  }
}
//.............................................................................
XVar& XVarManager::GetVar_(size_t id) {
  if (id >= Vars.Count()) {
    id -= Vars.Count();
    if (id >= ReservedVars.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "reserved var index");
    }
    return ReservedVars[id];
  }
  return Vars[id];
}
//.............................................................................
XVar& XVarManager::NewVar(double val, bool reindex) {
  XVar* v = new XVar(*this, val);
  v->SetId(Vars.Count());
  Vars.Add(v);
  if (reindex) {
    for (size_t i = 0; i < ReservedVars.Count(); i++) {
      if (!ReservedVars.IsNull(i)) {
        ReservedVars[i].SetId(Vars.Count() + i);
      }
    }
  }
  return *v;
}
//.............................................................................
XVar& XVarManager::GetReferencedVar(size_t ind) {
  if (ind < 1 || ind > 1024) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "invalid variable reference");
  }
  while (Vars.Count() < ind) {
    NewVar();
  }
  return Vars[ind - 1];
}
//.............................................................................
XVarReference& XVarManager::AddVarRef(XVar& var, IXVarReferencer& a,
  short var_name, short relation, double coeff)
{
  XVarReference* prf = a.GetVarRef(var_name);
  if (prf != NULL && olx_is_valid_index(prf->GetId())) {
    prf->Parent._RemRef(*prf);
    References.Delete(prf->GetId());
  }
  XVarReference& rf = References.Add(
    new XVarReference(var, a, var_name, relation, coeff));
  for (size_t i = 0; i < References.Count(); i++) {
    References[i].SetId(i);
  }
  var._AddRef(rf);
  a.SetVarRef(var_name, &rf);
  //  if( var_name == var_name_Uiso )
  //    a.SetUisoOwner(NULL);
  return rf;
}
//.............................................................................
XVarReference* XVarManager::ReleaseRef(IXVarReferencer& a, short var_name) {
  XVarReference* prf = a.GetVarRef(var_name);
  if (prf != 0) {
    if (!olx_is_valid_index(prf->GetId())) {
      return 0;
    }
    prf->Parent._RemRef(*prf);
    References.Release(prf->GetId());
    for (size_t i = 0; i < References.Count(); i++) {
      References[i].SetId(i);
    }
    prf->SetId(~0);
  }
  return prf;
}
//.............................................................................
void XVarManager::RestoreRef(IXVarReferencer& a, short var_name,
  XVarReference* vr)
{
  XVarReference* prf = a.GetVarRef(var_name);
  if (prf != 0) {
    prf->Parent._RemRef(*prf);
    if (olx_is_valid_index(prf->GetId())) { // is not released?
      References.Delete(prf->GetId());
    }
  }
  if (vr != 0) {
    a.SetVarRef(var_name, vr);
    vr->Parent._AddRef(*vr);
    References.Add(vr);
  }
  else {
    a.SetVarRef(var_name, 0);
  }
  for (size_t i = 0; i < References.Count(); i++) {
    References[i].SetId(i);
  }
}
//.............................................................................
double XVarManager::SetParam(IXVarReferencer& ca, short var_index, double val) {
  // despite in shelx a free |var reference| must be > 15, value greater than 10
  // means that the parameter is fixed, therefore we use Vars[0] for this...
  short var_rel = relation_None;
  double coeff = 0, actual_val = val;
  XVar* var = NULL;
  if (olx_abs(val) > 5) {
    int iv = (int)(val / 10);
    double a_val = olx_abs(val - iv * 10);
    if (a_val >= 5) {
      iv = olx_sign(iv)*(olx_abs(iv) + 1);
    }
    int var_index = olx_abs(iv);
    var = &GetReferencedVar(var_index);
    if (var_index == 1) { // fixed parameter
      actual_val = val - iv * 10;
    }
    else {
      var_rel = iv > 0 ? relation_AsVar : relation_AsOneMinusVar;
      coeff = olx_abs(val - iv * 10);
      actual_val = coeff*(var_rel == relation_AsVar ? var->GetValue()
        : 1.0 - var->GetValue());
    }
  }
  ca.SetValue(var_index, actual_val);
  if (var != 0) {
    AddVarRef(*var, ca, var_index, var_rel, coeff);
  }
  else {
    FreeParam(ca, var_index);
  }
  return actual_val;
}
//.............................................................................
void XVarManager::FixParam(IXVarReferencer& ca, short var_index) {
  // this is to fix states when vars are not defined (like while reading CIF)
  if (Vars.IsEmpty()) {
    NewVar(1.0).SetId(0);
  }
  AddVarRef(Vars[0], ca, var_index, relation_None, 1);
}
//.............................................................................
void XVarManager::FreeParam(IXVarReferencer& ca, short var_index) {
  XVarReference* vr = ca.GetVarRef(var_index);
  if (vr != NULL && olx_is_valid_index(vr->GetId())) {
    vr->Parent._RemRef(*vr);
    ca.SetVarRef(var_index, NULL);
    References.Delete(vr->GetId());
    for (size_t i = 0; i < References.Count(); i++) {
      References[i].SetId(i);
    }
  }
  //  if( var_index == var_name_Uiso )
  //    ca.SetUisoOwner(NULL);
}
//.............................................................................
double XVarManager::GetParam(const IXVarReferencer& ca, short var_index,
  double val) const
{
  const XVarReference* vr = ca.GetVarRef(var_index);
  if (vr == NULL) {
    return val;
  }
  if (vr->relation_type == relation_None) {
    const TCAtom *a = dynamic_cast<const TCAtom *>(&ca);
    if (a != NULL && var_index >= catom_var_name_X && var_index <= catom_var_name_Z) {
      return 10 + val;  // shelxl pecularity
    }
    else {
      return olx_sign(val)*(olx_abs(val) + 10);
    }
  }
  if (vr->relation_type == relation_AsVar) {
    return (vr->Parent.GetId() + 1) * 10 + vr->coefficient;
  }
  return -((vr->Parent.GetId()+1)*10+vr->coefficient);
  return 0;
}
//.............................................................................
void XVarManager::Validate() {
  bool changes = true;
  while (changes) {
    changes = false;
    for (size_t i = 0; i < Equations.Count(); i++) {
      if (!Equations.IsNull(i) && !Equations[i].Validate()) {
        changes = true;
        Equations.NullItem(i);
      }
    }
  }
  // start from 1 to leave global scale
  for (size_t i = 1; i < Vars.Count(); i++) {
    XVar& v = Vars[i];
    if (!v.IsUsed()) {
      for (size_t j = 0; j < v._RefCount(); j++) {
        XVarReference& vr = v.GetRef(j);
        vr.referencer.SetVarRef(vr.var_index, NULL);
        References.NullItem(vr.GetId());
      }
      Vars.NullItem(i);
    }
  }
  Equations.Pack();
  References.Pack();
  Vars.Pack();
  UpdateIds();
}
//.............................................................................
void XVarManager::UpdateIds() {
  for (size_t i = 0; i < Vars.Count(); i++) {
    Vars[i].SetId(i);
  }
  for (size_t i = 0; i < Equations.Count(); i++) {
    Equations[i].SetId(i);
  }
  for (size_t i = 0; i < References.Count(); i++) {
    References[i].SetId(i);
  }
  for (size_t i = 0; i < ReservedVars.Count(); i++) {
    if (!ReservedVars.IsNull(i)) {
      ReservedVars[i].SetId(i + Vars.Count());
    }
  }
}
//.............................................................................
short XVarManager::RelationIndex(const olxstr& rn) {
  for (short i = 0; i <= relation_Last; i++) {
    if (RelationNames[i] == rn) {
      return i;
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "unknown relation name");
}
//.............................................................................
XLEQ& XVarManager::NewEquation(double val, double sig) {
  XLEQ* leq = new XLEQ(*this, val, sig);
  leq->SetId(Equations.Count());
  return Equations.Add(leq);
}
//.............................................................................
XLEQ& XVarManager::ReleaseEquation(size_t i) {
  Equations[i].SetId(InvalidIndex);
  XLEQ& eq = Equations.Release(i);
  for (size_t i = 0; i < Equations.Count(); i++) {
    Equations[i].SetId(i);
  }
  return eq;
}
//.............................................................................
void XVarManager::AddFVAR(const TStrList &fvar) {
  for (size_t i = 0; i < fvar.Count(); i++, NextVar++) {
    if (Vars.Count() <= NextVar) {
      NewVar(fvar[i].ToDouble(), false);
    }
    else {
      Vars[NextVar].SetValue(fvar[i].ToDouble());
    }
  }
  for (size_t i = 0; i < ReservedVars.Count(); i++) {
    if (!ReservedVars.IsNull(i)) {
      ReservedVars[i].SetId(Vars.Count() + i);
    }
  }
}
//.............................................................................
void XVarManager::Describe(TStrList& lst) {
  Validate();
  for (size_t i = 0; i < Equations.Count(); i++) {
    olxstr eq_des;
    int var_added = 0;
    for (size_t j = 0; j < Equations[i].Count(); j++) {
      if (var_added++ != 0 && Equations[i].GetCoefficient(j) >= 0) {
        eq_des << '+';
      }
      eq_des << Equations[i].GetCoefficient(j) << "*[";
      int ref_added = 0;
      for (size_t k = 0; k < Equations[i][j]._RefCount(); k++) {
        XVarReference& vr = Equations[i][j].GetRef(k);
        if (ref_added++ != 0) {
          eq_des << '+';
        }
        eq_des << vr.referencer.GetVarName(vr.var_index) <<
          '(' << vr.referencer.GetIdName() << ')';
      }
      if (!ref_added) {
        if (Equations[i][j].IsReserved()) {
          eq_des << getReservedVarName(Equations[i][j].GetId() - Vars.Count());
        }
        else {
          eq_des << '?';
        }
      }
      eq_des << ']';
    }
    lst.Add(' ') << eq_des << '=' << Equations[i].GetValue() <<
      " with esd of " << Equations[i].GetSigma();
  }
  for (size_t i = 1; i < Vars.Count(); i++) {
    if (Vars[i]._RefCount() < 2) {
      continue;
    }

    olx_pdict<double, TPtrList<XVarReference> > avd;
    for (size_t j = 0; j < Vars[i]._RefCount(); j++) {
      if (Vars[i].GetRef(j).relation_type == relation_AsVar) {
        avd.Add(Vars[i].GetRef(j).coefficient).Add(Vars[i].GetRef(j));
      }
      else {
        avd.Add(-Vars[i].GetRef(j).coefficient).Add(Vars[i].GetRef(j));
      }
    }
    for (size_t j = 0; j < avd.Count(); j++) {
      olxstr &l = lst.Add(' ');
      for (size_t k = 0; k < avd.GetValue(j).Count(); k++) {
        XVarReference &vr = *avd.GetValue(j)[k];
        l << vr.referencer.GetVarName(vr.var_index) << '(' <<
          vr.referencer.GetIdName() << ")=";
      }
      if (avd.GetKey(j) < 0) {
        if (olx_abs(avd.GetKey(j) + 1) < 1e-3) {
          l << "1-FVAR(" << Vars[i].GetId() << ')';
        }
        else {
          l << (-avd.GetKey(j)) << "*(1-FVAR(" << (Vars[i].GetId() + 1) << "))";
        }
      }
      else {
        if (olx_abs(avd.GetKey(j) - 1) < 1e-3) {
          l << "FVAR(" << Vars[i].GetId() << ')';
        }
        else {
          l << avd.GetKey(j) << "*FVAR(" << (Vars[i].GetId() + 1) << ')';
        }
      }
    }
  }
  // fixed params...
  olxstr_dict<olxstr> fixed;
  for (size_t i = 0; i < Vars[0]._RefCount(); i++) {
    TCAtom *ca = dynamic_cast<TCAtom*>(&Vars[0].GetRef(i).referencer);
    if (ca != NULL) {
      if (ca->GetType() == iQPeakZ) {
        continue;
      }
      if (Vars[0].GetRef(i).var_index == catom_var_name_Sof &&
        olx_abs(ca->GetChemOccu() - 1) < 1e-3)
      {
        continue;
      }
    }
    size_t ind = fixed.IndexOf(Vars[0].GetRef(i).referencer.GetVarName(
      Vars[0].GetRef(i).var_index));
    if (ind == InvalidIndex) {
      fixed.Add(
        Vars[0].GetRef(i).referencer.GetVarName(Vars[0].GetRef(i).var_index),
        olxstr(Vars[0].GetRef(i).referencer.GetIdName()) << '(' <<
        Vars[0].GetRef(i).GetActualValue() << ')');
    }
    else {
      fixed.GetValue(ind) << ' ' << Vars[0].GetRef(i).referencer.GetIdName()
        << '(' << Vars[0].GetRef(i).GetActualValue() << ')';
    }
  }
  for (size_t i = 0; i < fixed.Count(); i++) {
    lst.Add(" Fixed ") << fixed.GetKey(i) << ": " << fixed.GetValue(i);
  }
}
//.............................................................................
void XVarManager::ToDataItem(TDataItem& item) const {
  TDataItem& vars = item.AddItem("vars");
  for (size_t i = 0; i < Vars.Count(); i++) {
    Vars[i].ToDataItem(vars.AddItem("item"));
  }
  TDataItem& rvars = item.AddItem("rvars");
  if (!ReservedVars.IsEmpty()) {
    for (size_t i = 0; i < ReservedVars.Count() - 1; i++) {
      rvars.AddItem("item", ReservedVars[i].ToString());
    }
    if (HasEXTI()) {
      rvars.AddField("EXTI", GetEXTI().ToString());
    }
  }
  TDataItem& eqs = item.AddItem("eqs");
  for (size_t i = 0; i < Equations.Count(); i++) {
    Equations[i].ToDataItem(eqs.AddItem("item"));
  }
}
//.............................................................................
#ifdef _PYTHON
PyObject* XVarManager::PyExport(TPtrList<PyObject>& atoms)  {
  PyObject* main = PyDict_New();

  TPtrList<PyObject> var_refs(Vars.Count());
  PyObject* vars = PyTuple_New(Vars.Count());
  for (size_t i = 0; i < Vars.Count(); i++) {
    PyTuple_SetItem(vars, i, var_refs[i] = Vars[i].PyExport(atoms));
  }
  PythonExt::SetDictItem(main, "variables", vars);
  TPtrList<PyObject> eqsl;
  for (size_t i = 0; i < Equations.Count(); i++) {
    PyObject *eq = Equations[i].PyExport(var_refs);
    if (eq != 0) {
      eqsl << eq;
    }
  }
  PyObject* eqs = PyTuple_New(eqsl.Count());
  for (size_t i = 0; i < eqsl.Count(); i++) {
    PyTuple_SetItem(eqs, i, eqsl[i]);
  }
  PythonExt::SetDictItem(main, "equations", eqs);
  return main;
}
#endif
//.............................................................................
void XVarManager::FromDataItem(const TDataItem& item) {
  ClearAll();
  TDataItem& vars = item.GetItemByName("vars");
  for (size_t i = 0; i < vars.ItemCount(); i++) {
    size_t cnt = Vars.Count();
    Vars.Add(XVar::FromDataItem(vars.GetItemByIndex(i), *this))
      .SetId(cnt);
  }

  {
    TDataItem* rvars = item.FindItem("rvars");
    if (rvars != 0) {
      for (size_t i = 0; i < rvars->ItemCount(); i++) {
        TEValueD x = rvars->GetItemByIndex(i).GetValue();
        XVar *v = new XVar(*this, x.GetV(), x.GetE());
        v->SetId(ReservedVars.Count() + Vars.Count());
        ReservedVars.Add(v);
      }
      const olxstr &ev = rvars->FindField("EXTI");
      if (ev.IsEmpty()) {
        ReservedVars.Add((XVar *)0);
      }
      else {
        TEValueD x = ev;
        XVar *v = new XVar(*this, x.GetV(), x.GetE());
        v->SetId(ReservedVars.Count() + Vars.Count());
        ReservedVars.Add(v);
      }
    }
  }

  TDataItem& eqs = item.GetItemByName("eqs");
  for( size_t i=0; i < eqs.ItemCount(); i++ ) {
    size_t cnt = Equations.Count();
    Equations.Add(XLEQ::FromDataItem(eqs.GetItemByIndex(i), *this))
      .SetId(cnt);
  }
  for( size_t i=0; i < References.Count(); i++ ) {
    References[i].referencer.SetVarRef(
      References[i].var_index, &References[i]);
  }
}
//.............................................................................
olxstr XVarManager::GetFVARStr() const {
  olxstr rv(Vars.IsEmpty() ? 1.0 : Vars[0].GetValue());
  for (size_t i = 1; i < Vars.Count(); i++) {
    rv << ' ' << Vars[i].GetValue();
  }
  return rv;
}
//.............................................................................
void XVarManager::AddSUMP(const TStrList &sump) {
  if (sump.Count() < 4) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "at least 4 parameters expected for SUMP");
  }
  if ((sump.Count() % 2) != 0) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "even number of arguments is expected for SUMP");
  }
  XLEQ& le = NewEquation(sump[0].ToDouble(), sump[1].ToDouble());
  for (size_t i = 2; i < sump.Count(); i += 2) {
    size_t vi = sump[i + 1].ToSizeT();
    if (vi > 1024) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    double k = sump[i].ToDouble();
    XVar* v;
    if (vi > Vars.Count()) {
      vi -= Vars.Count();
      if (vi > ReservedVars.Count()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "var index");
      }
      v = &ReservedVars[vi - 1];
    }
    else {
      v = &GetReferencedVar(vi);
    }
    le.AddMember(*v, k);
  }
}
//.............................................................................
olxstr XVarManager::GetSUMPStr(size_t ind) const {
  XLEQ& le = Equations[ind];
  olxstr rv(le.GetValue());
  rv << ' ' << le.GetSigma();
  for (size_t i = 0; i < le.Count(); i++) {
    rv << ' ' << le.GetCoefficient(i) << ' ' << le[i].GetId() + 1;
  }
  return rv;
}
//.............................................................................
olxstr XVarManager::getReservedVarName(size_t i) const {
  if (i + 1 == ReservedVars.Count()) {
    return "EXTI";
  }
  return olxstr("BASF") << (i + 1);
}
//.............................................................................
bool XVarManager::HasEXTI() const {
  if (ReservedVars.IsEmpty()) {
    return false;
  }
  return !ReservedVars.IsNull(ReservedVars.Count() - 1);
}
//.............................................................................
const XVar &XVarManager::GetEXTI() const {
  return ReservedVars.GetLast();
}
//.............................................................................
void XVarManager::SetEXTI(double val, double esd) {
  if (ReservedVars.IsEmpty()) {
    ReservedVars.Add(new XVar(*this, val, esd)).SetId(Vars.Count());
  }
  else {
    if (HasEXTI()) {
      GetEXTI().SetValue(val);
      GetEXTI().SetEsd(esd);
    }
    else {
      XVar *v = new XVar(*this, val, esd);
      v->SetId(Vars.Count() + ReservedVars.Count() - 1);
      ReservedVars.Set(ReservedVars.Count() - 1, v);
    }
  }
}
//.............................................................................
void XVarManager::RemoveReservedVar(XVar &v,
  olxset<XLEQ *, TPointerComparator> &leq_to_remove,
  olxset<XVarReference *, TPointerComparator> &ref_to_remove)
{
  for (size_t j = 0; j < v.GetLEQs().Count(); j++) {
    leq_to_remove.Add(v.GetLEQs()[j]);
  }
  for (size_t j = 0; j < v.GetRefs().Count(); j++) {
    ref_to_remove.Add(v.GetRefs()[j]);
  }
}
//.............................................................................
void XVarManager::FinaliseReservedVarRemoval(
  olxset<XLEQ *, TPointerComparator> &leq_to_remove,
  olxset<XVarReference *, TPointerComparator> &ref_to_remove)
{
  for (size_t i = 0; i < leq_to_remove.Count(); i++) {
    Equations.NullItem(leq_to_remove[i]->GetId());
  }
  for (size_t i = 0; i < ref_to_remove.Count(); i++) {
    // release
    ref_to_remove[i]->referencer.SetVarRef(ref_to_remove[i]->var_index, 0);
    References.NullItem(ref_to_remove[i]->GetId());
  }
  Equations.Pack();
  References.Pack();
  for (size_t i = 0; i < Equations.Count(); i++) {
    Equations[i].SetId(i);
  }
  for (size_t i = 0; i < References.Count(); i++) {
    References[i].SetId(i+Vars.Count());
  }
}
//.............................................................................
void XVarManager::ClearEXTI() {
  if (ReservedVars.IsEmpty() || ReservedVars.IsNull(ReservedVars.Count()-1)) {
    return;
  }
  olxset<XLEQ *, TPointerComparator> leq_to_remove;
  olxset<XVarReference *, TPointerComparator> ref_to_remove;
  RemoveReservedVar(ReservedVars.GetLast(), leq_to_remove, ref_to_remove);
  FinaliseReservedVarRemoval(leq_to_remove, ref_to_remove);
  ReservedVars.NullItem(ReservedVars.Count()-1);
}
//.............................................................................
XVar &XVarManager::GetEXTI() {
  return ReservedVars.GetLast();
}
//.............................................................................
void XVarManager::SetBASF(const TStrList& bs) {
  if (ReservedVars.IsEmpty()) { // set EXTI var
    ReservedVars.Add((XVar *)0);
  }
  size_t cnt = ReservedVars.Count();
  ReservedVars.SetCapacity(cnt + bs.Count());
  for (size_t i = 0; i < bs.Count(); i++) {
    ReservedVars.Insert(ReservedVars.Count() - 1,
      new XVar(*this, bs[i].ToDouble()));
  }
  for (size_t i = 0; i < ReservedVars.Count(); i++) {
    if (!ReservedVars.IsNull(i)) {
      ReservedVars[i].SetId(i + Vars.Count());
    }
  }
}
//.............................................................................
size_t XVarManager::GetBASFCount() const {
  return ReservedVars.IsEmpty() ? 0 : ReservedVars.Count() - 1;
}
//.............................................................................
const XVar &XVarManager::GetBASF(size_t i) const {
  return ReservedVars[i];
}
//.............................................................................
XVar &XVarManager::GetBASF(size_t i) {
  return ReservedVars[i];
}
//.............................................................................
void XVarManager::ClearBASF() {
  if (ReservedVars.Count() < 2) {
    return;
  }
  olxset<XLEQ *, TPointerComparator> leq_to_remove;
  olxset<XVarReference *, TPointerComparator> ref_to_remove;
  for (size_t i = 0; i < ReservedVars.Count() - 1; i++) {
    RemoveReservedVar(ReservedVars[i], leq_to_remove, ref_to_remove);
  }
  FinaliseReservedVarRemoval(leq_to_remove, ref_to_remove);
  ReservedVars.DeleteRange(0, ReservedVars.Count() - 1);
  if (HasEXTI()) {
    GetEXTI().SetId(Vars.Count() + ReservedVars.Count() - 1);
  }
}
//.............................................................................
