#include "leq.h"
#include "asymmunit.h"


olxstr XVarManager::VarNames[] = {"Scale", "X", "Y", "Z", "Sof", "Uiso", "U11", "U22", "U33", "U23", "U13", "U12"};


int XVar::RefCount() const {
  int rv = 0;
  for( int i=0; i < References.Count(); i++ )  
    if( !References[i]->atom->IsDeleted() )
      rv++;
  return rv;
}
//.................................................................................................
//.................................................................................................
//.................................................................................................
void XLEQ::_Assign(const XLEQ& leq)  {
  for( int i=0; i < leq.Vars.Count(); i++ )
    AddMember(Parent.GetVar(leq.Vars[i]->GetId()), leq.Coefficients[i] );
  Value = leq.Value;
  Sigma = leq.Sigma;
}
//.................................................................................................
//.................................................................................................
//.................................................................................................
void XVarManager::ClearAll()  {
  Equations.Clear();
  Vars.Clear();
  References.Clear();
  NextVar = 0;  // the global scale factor
  for( int i=0; i < aunit.AtomCount(); i++ )  {
    TCAtom& ca = aunit.GetAtom(i);
    for( short j=var_name_First; j <= var_name_Last; j++ )
      ca.NullVarRef(j);
  }
}
//.................................................................................................
void XVarManager::Assign(const XVarManager& vm) {
  ClearAll();
  for( int i=0; i < vm.Vars.Count(); i++ )
    NewVar( vm.Vars[i].GetValue() );
  for( int i=0; i < vm.References.Count(); i++ )  {
    XVarReference& vr = vm.References[i];
    TCAtom* ca = aunit.FindCAtomByLoaderId( vr.atom->GetLoaderId() );
    if( ca == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    AddVarRef(Vars[vr.Parent.GetId()], *ca, vr.var_name, vr.relation_type, vr.coefficient);
  }
  for( int i=0; i < vm.Equations.Count(); i++ )
    NewEquation()._Assign(vm.Equations[i]);
}
//.................................................................................................
XVarReference& XVarManager::AddVarRef(XVar& var, TCAtom& a, short var_name, short relation, double coeff)  {
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL && prf->GetId() != -1 )  {
    prf->Parent._RemRef(*prf);
    References.Delete(prf->GetId());
  }
  if( coeff == -10.0 )
    coeff = 1./a.GetDegeneracy();
  XVarReference& rf = References.Add( new XVarReference(var, &a, var_name, relation, coeff) );
  for( int i=0; i < References.Count(); i++ )
    References[i].SetId(i);
  var._AddRef(rf);
  a.SetVarRef(rf);
  if( var_name == var_name_Uiso )
    a.SetUisoOwner(NULL);
  return rf;
}
//.................................................................................................
XVarReference* XVarManager::ReleaseRef(TCAtom& a, short var_name) {
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL )  {
    if( prf->GetId() == -1 )  
      return NULL;
    prf->Parent._RemRef(*prf);
    References.Release(prf->GetId());
    for( int i=0; i < References.Count(); i++ )
      References[i].SetId(i);
    prf->SetId(-1);
  }
  return prf;
}
//.................................................................................................
void XVarManager::RestoreRef(TCAtom& a, short var_name, XVarReference* vr) {
  XVarReference* prf = a.GetVarRef(var_name);
  if( prf != NULL )  {
    prf->Parent._RemRef(*prf);
    if( prf->GetId() != -1 )  // is not released?
      References.Delete(prf->GetId());
  }
  if( vr != NULL )  {
    a.SetVarRef(*vr);
    vr->Parent._AddRef(*vr);
    References.Add(vr);
  }
  else 
    a.NullVarRef(var_name);   
  for( int i=0; i < References.Count(); i++ )
    References[i].SetId(i);
}
//.................................................................................................
double XVarManager::SetAtomParam(TCAtom& ca, short param_name, double val) {
  // despite in shelx a free |var reference| must be > 15, value greater than 10
  // means that the parameter is fixed, therefore we user Vars[0] for this...
  short var_rel = relation_None;
  double coeff = 0, actual_val = val;
  XVar* var = NULL;
  if( fabs(val) > 10 )  {
    int iv = (int)(val/10);
    int var_index = abs(iv);
    var = &GetReferencedVar(var_index);
    if( var_index == 1 )  // fixed parameter
      actual_val = val - iv*10;
    else {
      var_rel = iv > 0 ? relation_AsVar : relation_AsOneMinusVar;
      coeff = val -iv*10;
      actual_val = coeff*var->GetValue();
      if( iv < 0 )
        actual_val += 1;
      coeff = fabs(coeff);
    }
  }
  switch( param_name )  {
    case var_name_X:     ca.ccrd()[0] = actual_val;  break;
    case var_name_Y:     ca.ccrd()[1] = actual_val;  break;
    case var_name_Z:     ca.ccrd()[2] = actual_val;  break;
    case var_name_Sof:   ca.SetOccu(actual_val);  break;
    case var_name_Uiso:  
      ca.SetUiso(actual_val);  
      ca.SetUisoOwner(NULL);
      break;
    case var_name_U11:   break;
    case var_name_U22:   break;
    case var_name_U33:   break;
    case var_name_U12:   break;
    case var_name_U13:   break;
    case var_name_U23:   break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
  if( var != NULL )
    AddVarRef(*var, ca, param_name, var_rel, coeff);
  else
    FreeAtomParam(ca, param_name);
  return actual_val;
}
//.................................................................................................
void XVarManager::FixAtomParam(TCAtom& ca, short param_name) {
  AddVarRef(Vars[0], ca, param_name, relation_None, 1);
}
//.................................................................................................
void XVarManager::FreeAtomParam(TCAtom& ca, short param_name) {
  XVarReference* vr = ca.GetVarRef(param_name);
  if( vr != NULL && vr->GetId() != -1 )  {
    vr->Parent._RemRef( *vr );
    ca.NullVarRef(param_name);
    References.Delete(vr->GetId());
    for( int i=0; i < References.Count(); i++ )
      References[i].SetId(i);
  }
  if( param_name == var_name_Uiso )
    ca.SetUisoOwner(NULL);
}
//.................................................................................................
double XVarManager::GetAtomParam(TCAtom& ca, short param_name, double* Q) {
  XVarReference* vr = ca.GetVarRef(param_name);
  double val = 0;
  if( param_name >= var_name_U11 && param_name <= var_name_U23 && Q == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "qudratic form cannot be NULL");
  switch( param_name )  {
    case var_name_X:     val = ca.ccrd()[0];  break;
    case var_name_Y:     val = ca.ccrd()[1];  break;
    case var_name_Z:     val = ca.ccrd()[2];  break;
    case var_name_Sof:   val = ca.GetOccu();  break;
    case var_name_Uiso:  
      val = ca.GetUiso();  
      break;
    case var_name_U11:   val = Q[0];  break;
    case var_name_U22:   val = Q[1];  break;
    case var_name_U33:   val = Q[2];  break;
    case var_name_U12:   val = Q[3];  break;
    case var_name_U13:   val = Q[4];  break;
    case var_name_U23:   val = Q[5];  break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
  if( vr == NULL )  return val;
  if( vr->relation_type == relation_None )
    return Sign(val)*(fabs(val)+10);
  if( vr->relation_type == relation_AsVar )
    return (vr->Parent.GetId()+1)*10+vr->coefficient;
  return -((vr->Parent.GetId()+1)*10+vr->coefficient);
  return 0;
}
//.................................................................................................
void XVarManager::Validate() {
  bool changes = true;
  while( changes )  {
    changes = false;
    for( int i=0; i < Equations.Count(); i++ )  {
      if( Equations.IsNull(i) )  continue;
      if( !Equations[i].Validate() )  {
        changes = true;
        Equations.NullItem(i);
      }
    }
  }
  for( int i=1; i < Vars.Count(); i++ )  {// start from 1 to leave global scale
    XVar& v = Vars[i];   
    if( !v.IsUsed() ) {
      for( int j=0; j < v.RefCount(); j++ )  {
        XVarReference& vr = v.GetRef(j);
        vr.atom->NullVarRef(vr.var_name);
      }
      Vars.NullItem(i);
    }
  }
  Equations.Pack();
  Vars.Pack();
  for( int i=0; i < Vars.Count(); i++ )
    Vars[i].SetId(i);
  for( int i=0; i < Equations.Count(); i++ )
    Equations[i].SetId(i);
}
//.................................................................................................
void XVarManager::ToDataItem(TDataItem& item) const {
  TDataItem& vars = item.AddItem("vars");
  for( int i=0; i < Vars.Count(); i++ )  {
    TDataItem& vi = vars.AddItem(Vars[i].GetId(), Vars[i].GetValue());
    for( int j=0; j < Vars[i].RefCount(); j++ )  {
      XVarReference& vr = Vars[i].GetRef(j);
      TDataItem& ri = vi.AddItem(VarNames[vr.var_name]);
      ri.AddField("atom_id", vr.atom->GetTag());
      ri.AddField("coefficient", vr.coefficient);
      ri.AddField("atom_id", (vr.relation_type == relation_None) ? "none" : (vr.relation_type == relation_AsVar ? "var" : "one_minus_var"));
    }
  }
  TDataItem& leqs = item.AddItem("leqs");
  for( int i=0; i < Equations.Count(); i++ )  {
    TDataItem& li = leqs.AddItem("leq");
    li.AddField("val", Equations[i].GetValue());
    li.AddField("sig", Equations[i].GetSigma());
    for( int j=0; j < Equations[i].Count(); j++ )  {
      TDataItem& mi = li.AddItem("var");
      mi.AddField("id", Equations[i][j].GetId());
      mi.AddField("coefficient", Equations[i].GetCoefficient(j));
    }
  }
}
//.................................................................................................
void XVarManager::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.................................................................................................

