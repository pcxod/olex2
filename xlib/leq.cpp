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
  for( size_t i=0; i < References.Count(); i++ )
    if( References[i]->referencer.IsValid() )
      rv++;
  return rv;
}
//.............................................................................
void XVar::ToDataItem(TDataItem& item) const {
  item.AddField("val", TEValueD(Value, Esd).ToString());
  for( size_t i=0; i < References.Count(); i++ )
    if( References[i]->referencer.IsValid() )
      References[i]->ToDataItem(item.AddItem("reference"));
}
//.............................................................................
#ifdef _PYTHON
PyObject* XVar::PyExport(TPtrList<PyObject>& atoms)  {
  size_t rc = 0;
  for( size_t i=0; i < References.Count(); i++ )  {
    if( References[i]->referencer.IsValid() )
      rc++;
  }
  PyObject* main = PyDict_New(),
    *refs = PyTuple_New(rc);
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));
  if( rc != 0 )  {
    rc = 0;
    for( size_t i=0; i < References.Count(); i++ )  {
      if( References[i]->referencer.IsValid() )
        PyTuple_SetItem(refs, rc++, References[i]->PyExport(atoms));
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
  const size_t rc = RefCount();
  if( LeqCount() == 0 )  {
    if( rc == 1 )  {
      if( TXApp::DoPreserveFVARs() ||
          EsdlInstanceOf(References[0]->referencer, TSimpleRestraint) )
        return true;
    }
    return rc > 1;
  }
  return rc > 0;
}
//.............................................................................
//.............................................................................
//.............................................................................
void XLEQ::_Assign(const XLEQ& leq)  {
  for( size_t i=0; i < leq.Vars.Count(); i++ )
    AddMember(Parent.GetVar(leq.Vars[i]->GetId()), leq.Coefficients[i]);
  Value = leq.Value;
  Sigma = leq.Sigma;
}
//.............................................................................
void XLEQ::ToDataItem(TDataItem& item) const {
  item.AddField("val", Value);
  item.AddField("sig", Sigma);
  for( size_t i=0; i < Vars.Count(); i++ )  {
    TDataItem& mi = item.AddItem("var");
    mi.AddField("id", Vars[i]->GetId());
    mi.AddField("k", Coefficients[i]);
  }
}
//.............................................................................
#ifdef _PYTHON
PyObject* XLEQ::PyExport(TPtrList<PyObject>& _vars)  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "value", Py_BuildValue("d", Value));
  PythonExt::SetDictItem(main, "sigma", Py_BuildValue("d", Sigma));
  PyObject* vars = PyTuple_New(Vars.Count());
  for( size_t i=0; i < Vars.Count(); i++ )  {
    Py_IncRef(_vars[Vars[i]->GetId()]);
    PyTuple_SetItem(vars, i, _vars[Vars[i]->GetId()]);
  }
  PythonExt::SetDictItem(main, "variables", vars);
  return main;
}
#endif
//.............................................................................
XLEQ& XLEQ::FromDataItem(const TDataItem& item, XVarManager& parent) {
  XLEQ* leq = new XLEQ(parent, item.GetFieldByName("val").ToDouble(),
    item.GetFieldByName("sig").ToDouble());
  for( size_t i=0; i < item.ItemCount(); i++ )  {
    const TDataItem& mi = item.GetItemByIndex(i);
    leq->AddMember(parent.GetVar(mi.GetFieldByName("id").ToInt()),
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
}
//.............................................................................
void XVarManager::ClearAll()  {
  Clear();
  Vars.Clear();
  References.Clear();
  RM.ClearVarRefs();
}
//.............................................................................
void XVarManager::Assign(const XVarManager& vm) {
  ClearAll();
  for( size_t i=0; i < vm.Vars.Count(); i++ )
    NewVar(vm.Vars[i].GetValue()).SetEsd(vm.Vars[i].GetEsd());
  for( size_t i=0; i < vm.References.Count(); i++ )  {
    XVarReference& vr = vm.References[i];
    IXVarReferencerContainer& rc =
      RM.GetRefContainer(vr.referencer.GetParentContainer().GetIdName());
    IXVarReferencer& xvr = rc.GetReferencer(vr.referencer.GetReferencerId());
    AddVarRef(Vars[vr.Parent.GetId()],
      xvr, vr.var_index, vr.relation_type, vr.coefficient);
  }
  for( size_t i=0; i < vm.Equations.Count(); i++ )
    NewEquation()._Assign(vm.Equations[i]);
  if( Vars.IsEmpty() )  // odd eh?
    NewVar(1.0).SetId(0);
}
//.............................................................................
XVarReference& XVarManager::AddVarRef(XVar& var, IXVarReferencer& a,
  short var_name, short relation, double coeff)
{
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL && olx_is_valid_index(prf->GetId()) )  {
    prf->Parent._RemRef(*prf);
    References.Delete(prf->GetId());
  }
  XVarReference& rf = References.Add(
    new XVarReference(var, a, var_name, relation, coeff));
  for( size_t i=0; i < References.Count(); i++ )
    References[i].SetId(i);
  var._AddRef(rf);
  a.SetVarRef(var_name, &rf);
//  if( var_name == var_name_Uiso )
//    a.SetUisoOwner(NULL);
  return rf;
}
//.............................................................................
XVarReference* XVarManager::ReleaseRef(IXVarReferencer& a, short var_name) {
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL )  {
    if( !olx_is_valid_index(prf->GetId()) )
      return NULL;
    prf->Parent._RemRef(*prf);
    References.Release(prf->GetId());
    for( size_t i=0; i < References.Count(); i++ )
      References[i].SetId(i);
    prf->SetId(~0);
  }
  return prf;
}
//.............................................................................
void XVarManager::RestoreRef(IXVarReferencer& a, short var_name,
  XVarReference* vr)
{
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL )  {
    prf->Parent._RemRef(*prf);
    if( olx_is_valid_index(prf->GetId()) )  // is not released?
      References.Delete(prf->GetId());
  }
  if( vr != NULL )  {
    a.SetVarRef(var_name, vr);
    vr->Parent._AddRef(*vr);
    References.Add(vr);
  }
  else
    a.SetVarRef(var_name, NULL);
  for( size_t i=0; i < References.Count(); i++ )
    References[i].SetId(i);
}
//.............................................................................
double XVarManager::SetParam(IXVarReferencer& ca, short var_index, double val) {
  // despite in shelx a free |var reference| must be > 15, value greater than 10
  // means that the parameter is fixed, therefore we use Vars[0] for this...
  short var_rel = relation_None;
  double coeff = 0, actual_val = val;
  XVar* var = NULL;
  if( olx_abs(val) > 5 )  {
    int iv = (int)(val/10);
    double a_val = olx_abs(val - iv*10);
    if( a_val >= 5 )
      iv = olx_sign(iv)*(olx_abs(iv)+1);
    int var_index = olx_abs(iv);
    var = &GetReferencedVar(var_index);
    if( var_index == 1 )  // fixed parameter
      actual_val = val - iv*10;
    else {
      var_rel = iv > 0 ? relation_AsVar : relation_AsOneMinusVar;
      coeff = olx_abs(val -iv*10);
      actual_val = coeff*(var_rel == relation_AsVar ? var->GetValue()
        : 1.0 - var->GetValue());
    }
  }
  ca.SetValue(var_index, actual_val);
  if( var != NULL )
    AddVarRef(*var, ca, var_index, var_rel, coeff);
  else
    FreeParam(ca, var_index);
  return actual_val;
}
//.............................................................................
void XVarManager::FixParam(IXVarReferencer& ca, short var_index) {
  // this is to fix states when vars are not defined (like while reading CIF)
  if( Vars.IsEmpty() )
    NewVar(1.0).SetId(0);
  AddVarRef(Vars[0], ca, var_index, relation_None, 1);
}
//.............................................................................
void XVarManager::FreeParam(IXVarReferencer& ca, short var_index) {
  XVarReference* vr = ca.GetVarRef(var_index);
  if( vr != NULL && olx_is_valid_index(vr->GetId()) )  {
    vr->Parent._RemRef( *vr );
    ca.SetVarRef(var_index, NULL);
    References.Delete(vr->GetId());
    for( size_t i=0; i < References.Count(); i++ )
      References[i].SetId(i);
  }
//  if( var_index == var_name_Uiso )
//    ca.SetUisoOwner(NULL);
}
//.............................................................................
double XVarManager::GetParam(const IXVarReferencer& ca, short var_index,
  double val) const
{
  const XVarReference* vr = ca.GetVarRef(var_index);
  if (vr == NULL)  return val;
  if (vr->relation_type == relation_None) {
    const TCAtom *a = dynamic_cast<const TCAtom *>(&ca);
    if (a != NULL && var_index >= catom_var_name_X && var_index <= catom_var_name_Z)
      return 10 + val;  // shelxl pecularity
    else
      return olx_sign(val)*(olx_abs(val)+10);
  }
  if (vr->relation_type == relation_AsVar)
    return (vr->Parent.GetId()+1)*10+vr->coefficient;
  return -((vr->Parent.GetId()+1)*10+vr->coefficient);
  return 0;
}
//.............................................................................
void XVarManager::Validate() {
  bool changes = true;
  while( changes )  {
    changes = false;
    for( size_t i=0; i < Equations.Count(); i++ )  {
      if( Equations.IsNull(i) )  continue;
      if( !Equations[i].Validate() )  {
        changes = true;
        Equations.NullItem(i);
      }
    }
  }
  // start from 1 to leave global scale
  for( size_t i=1; i < Vars.Count(); i++ )  {
    XVar& v = Vars[i];
    if( !v.IsUsed() ) {
      for( size_t j=0; j < v._RefCount(); j++ )  {
        XVarReference& vr = v.GetRef(j);
        vr.referencer.SetVarRef(vr.var_index, NULL);
        References.NullItem( vr.GetId() );
      }
      Vars.NullItem(i);
    }
  }
  Equations.Pack();
  References.Pack();
  Vars.Pack();
  for( size_t i=0; i < Vars.Count(); i++ )
    Vars[i].SetId(i);
  for( size_t i=0; i < Equations.Count(); i++ )
    Equations[i].SetId(i);
  for( size_t i=0; i < References.Count(); i++ )
    References[i].SetId(i);
}
//.............................................................................
short XVarManager::RelationIndex(const olxstr& rn) {
  for( short i=0; i <= relation_Last; i++ )
    if( RelationNames[i] == rn )
      return i;
  throw TInvalidArgumentException(__OlxSourceInfo, "unknown relation name");
}
//.............................................................................
void XVarManager::Describe(TStrList& lst)  {
  Validate();
  for( size_t i=0; i < Equations.Count(); i++ )  {
    olxstr eq_des;
    int var_added = 0;
    for( size_t j=0; j < Equations[i].Count(); j++ )  {
      if( var_added++ != 0 && Equations[i].GetCoefficient(j) >= 0 )
        eq_des << '+';
      eq_des << Equations[i].GetCoefficient(j) << "*[";
      int ref_added = 0;
      for( size_t k=0; k < Equations[i][j]._RefCount(); k++ )  {
        XVarReference& vr = Equations[i][j].GetRef(k);
        if( ref_added++ != 0 )
          eq_des << '+';
        eq_des << vr.referencer.GetVarName(vr.var_index) <<
          '(' << vr.referencer.GetIdName() << ')';
      }
      eq_des << ']';
    }
    lst.Add(' ') << eq_des << '=' << Equations[i].GetValue() <<
      " with esd of " << Equations[i].GetSigma();
  }
  for (size_t i=1; i < Vars.Count(); i++) {
    if (Vars[i]._RefCount() < 2) continue;

    olx_pdict<double, TPtrList<XVarReference> > avd;
    for (size_t j=0; j < Vars[i]._RefCount(); j++) {
      if (Vars[i].GetRef(j).relation_type == relation_AsVar)
        avd.Add(Vars[i].GetRef(j).coefficient).Add(Vars[i].GetRef(j));
      else
        avd.Add(-Vars[i].GetRef(j).coefficient).Add(Vars[i].GetRef(j));
    }
    for (size_t j=0; j < avd.Count(); j++) {
      olxstr &l = lst.Add(' ');
      for (size_t k=0; k < avd.GetValue(j).Count(); k++) {
        XVarReference &vr = *avd.GetValue(j)[k];
        l << vr.referencer.GetVarName(vr.var_index) << '(' <<
          vr.referencer.GetIdName() << ")=";
      }
      if (avd.GetKey(j) < 0) {
        if (olx_abs(avd.GetKey(j)+1) < 1e-3)
          l << "1-FVAR(" << Vars[i].GetId() << ')';
        else
          l << (-avd.GetKey(j)) << "*(1-FVAR(" << (Vars[i].GetId()+1) << "))";
      }
      else {
        if (olx_abs(avd.GetKey(j)-1) < 1e-3)
          l << "FVAR(" << Vars[i].GetId() << ')';
        else
          l << avd.GetKey(j) << "*FVAR(" << (Vars[i].GetId()+1) << ')';
      }
    }
  }
  // fixed params...
  olxstr_dict<olxstr> fixed;
  for( size_t i=0; i < Vars[0]._RefCount(); i++ )  {
    TCAtom *ca = dynamic_cast<TCAtom*>(&Vars[0].GetRef(i).referencer);
    if (ca != NULL) {
      if (ca->GetType() == iQPeakZ) continue;
      if (Vars[0].GetRef(i).var_index == catom_var_name_Sof &&
        olx_abs(ca->GetChemOccu()-1) < 1e-3)
        continue;
    }
    size_t ind = fixed.IndexOf(Vars[0].GetRef(i).referencer.GetVarName(
      Vars[0].GetRef(i).var_index));
    if( ind == InvalidIndex ) {
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
  for( size_t i=0; i < fixed.Count(); i++ ) {
    lst.Add(" Fixed ") << fixed.GetKey(i) << ": " << fixed.GetValue(i);
  }
}
//.............................................................................
void XVarManager::ToDataItem(TDataItem& item) const {
  TDataItem& vars = item.AddItem("vars");
  for( size_t i=0; i < Vars.Count(); i++ )
    Vars[i].ToDataItem(vars.AddItem("item"));
  TDataItem& eqs = item.AddItem("eqs");
  for( size_t i=0; i < Equations.Count(); i++ )
    Equations[i].ToDataItem(eqs.AddItem("item"));
}
//.............................................................................
#ifdef _PYTHON
PyObject* XVarManager::PyExport(TPtrList<PyObject>& atoms)  {
  PyObject* main = PyDict_New();

  TPtrList<PyObject> var_refs(Vars.Count());
  PyObject* vars = PyTuple_New(Vars.Count());
  for( size_t i=0; i < Vars.Count(); i++ )
    PyTuple_SetItem(vars, i, var_refs[i] = Vars[i].PyExport(atoms));
  PythonExt::SetDictItem(main, "variables", vars);

  PyObject* eqs = PyTuple_New(Equations.Count());
  for( size_t i=0; i < Equations.Count(); i++ )
    PyTuple_SetItem(eqs, i, Equations[i].PyExport(var_refs));
  PythonExt::SetDictItem(main, "equations", eqs);
  return main;
}
#endif
//.............................................................................
void XVarManager::FromDataItem(const TDataItem& item) {
  ClearAll();
  TDataItem& vars = item.GetItemByName("vars");
  for (size_t i = 0; i < vars.ItemCount(); i++) {
    Vars.Add(XVar::FromDataItem(vars.GetItemByIndex(i), *this))
      .SetId(Vars.Count());
  }
  TDataItem& eqs = item.GetItemByName("eqs");
  for( size_t i=0; i < eqs.ItemCount(); i++ ) {
    Equations.Add(XLEQ::FromDataItem(eqs.GetItemByIndex(i), *this))
      .SetId(Vars.Count());
  }
  for( size_t i=0; i < References.Count(); i++ ) {
    References[i].referencer.SetVarRef(
      References[i].var_index, &References[i]);
  }
}
//.............................................................................
