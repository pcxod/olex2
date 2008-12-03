#include "leq.h"
#include "catom.h"

int XVar::ReferenceCount() const {
  int rv = 0;
  for( int i=0; i < References.Count(); i++ )  
    if( !References[i]->atom->IsDeleted() )
      rv++;
  return rv;
}
//.................................................................................................
XVarReference& XVar::AddReference(TCAtom* a, short var_name, short relation, double coefficient)  {
  XVarReference& vr = Parent.AddVarRef(*this, a, var_name, relation, coefficient);
  if( a->GetVarRef(var_name) != NULL )
    a->GetVarRef(var_name)->Parent.RemoveReference(*a->GetVarRef(var_name));
  a->SetVarRef(vr);
  References.Add(&vr);
  return vr;
}
//.................................................................................................
//.................................................................................................
//.................................................................................................
void XVarManager::Assign(const XVarManager& vm) {

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
    int var_index = abs(iv)-1;
    if( var_index == -1 )  // do nothing
      ;
    else  {
      var = &GetReferencedVar(var_index);
      if( var_index == 0 )  // fixed parameter
        actual_val = val - iv*10;
      else {
        var_rel = iv > 0 ? relation_AsVar : relation_AsOneMinusVar;
        coeff = val -iv*10;
        actual_val = coeff*var->GetValue();
        if( iv < 0 )
          actual_val += 1;
      }
    }
  }
  switch( param_name )  {
    case var_name_X:     ca.ccrd()[0] = actual_val;  break;
    case var_name_Y:     ca.ccrd()[1] = actual_val;  break;
    case var_name_Z:     ca.ccrd()[2] = actual_val;  break;
    case var_name_Sof:   ca.SetOccu(actual_val);  break;
    case var_name_Uiso:  ca.SetOccu(actual_val);  break;
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
    var->AddReference(&ca, param_name, var_rel, coeff);
  return actual_val;
}
//.................................................................................................
void XVarManager::FixAtomParam(TCAtom& ca, short param_name) {
  if( ca.GetVarRef(param_name) != NULL )
    ca.GetVarRef(param_name)->Parent.RemoveReference( *ca.GetVarRef(param_name) );
  ca.SetVarRef( Vars[0].AddReference(&ca, param_name, relation_None, 0));
}
//.................................................................................................
void XVarManager::FreeAtomParam(TCAtom& ca, short param_name) {
  if( ca.GetVarRef(param_name) != NULL )  {
    ca.GetVarRef(param_name)->Parent.RemoveReference( *ca.GetVarRef(param_name) );
    ca.NullVarRef(param_name);
  }
}
//.................................................................................................
double XVarManager::GetAtomParam(TCAtom& ca, short param_name, double* Q) {
  return 0;
}
//.................................................................................................
void XVarManager::ToDataItem(TDataItem& item) const {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.................................................................................................
void XVarManager::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.................................................................................................

