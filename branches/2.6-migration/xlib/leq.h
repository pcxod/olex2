#ifndef __olx_liner_eq_h
#define __olx_liner_eq_h
#include "xbase.h"
#include "rm_base.h"
#include "typelist.h"
#include "dataitem.h"
#ifndef _NO_PYTHON
  #include "pyext.h"
#endif

BeginXlibNamespace()

const short  // relation of parameters to variables
  relation_None          = 0,  // fixed param
  relation_AsVar         = 1,
  relation_AsOneMinusVar = 2,
  relation_Last          = 2;  // for iterations

/* Any single variable can be shared by several atom parameters (to make them equal) as well 
as by several linear equations
*/

class XLEQ;
class XVar;
class XVarManager;
struct XVarReference;

//class IXVar

struct XVarReference {
protected:
  int Id;
public:
  IXVarReferencer* referencer;
  short var_index;  // one of the var_name
  short relation_type; // relationAsVar or relation_AsOneMinusVar
  double coefficient; // like 0.25 in 20.25
  XVar& Parent;
  XVarReference(XVar& parent, IXVarReferencer* r, short _var_index, 
    short _relation_type, double coeff=1.0) : 
    Parent(parent),
    referencer(r), 
    var_index(_var_index), 
    relation_type(_relation_type), 
    coefficient(coeff) { }
  DefPropP(int, Id)
  // returns the value of the atom parameter associated with this reference
  double GetActualValue() const {  return referencer->GetValue(var_index);  }
  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& referrers);
#endif
  // returns a new instance created with new
  static XVarReference& FromDataItem(const TDataItem& item, XVar& parent);
};

class XVar {
  double Value;
  TPtrList<XVarReference> References;  // owed from the Parent
  TPtrList<XLEQ> Equations; // equations using this variable
  int Id;
public:

  XVarManager& Parent;

  XVar(XVarManager& parent, double val=0.5) : Parent(parent), Value(val), Id(-1) { }
  // adds a new atom, referencing this variable, for internal use
  XVarReference& _AddRef(XVarReference& vr)  {  return *References.Add(&vr);  }
  // removes a refence, for internal use
  void _RemRef(XVarReference& vr)  {  References.Remove(&vr);  }
  int _RefCount()  const {  return References.Count();  }
  // calculates the number of atoms (excluding the deleted ones) using this variable
  int RefCount() const;
  XVarReference& GetRef(int i) const {  return *References[i];  }

  // adds a new equation, referencing this variable
  void _AddLeq(XLEQ& eq)  {  Equations.Add(&eq);  }
  // removes an equation, referencing this variable
  void _RemLeq(XLEQ& eq)  {  Equations.Remove(&eq);  }
  // returns the number of equations, referencing this variable
  int LeqCount() const {  return Equations.Count();  }
  bool IsUsed() const {
    const int rc = RefCount();
    if( LeqCount() == 0 )  
      return rc > 1;
    return rc > 0;
  }
  DefPropP(double, Value)
  DefPropP(int, Id)  

  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  static XVar& FromDataItem(const TDataItem& item, XVarManager& parent);
};

class XLEQ {
  double Value, Sigma;
  TDoubleList Coefficients;
  TPtrList<XVar> Vars;
  int Id;
public:

  XVarManager& Parent;

  XLEQ(XVarManager& parent, double val=1.0, double sig=0.01) : 
      Parent(parent), 
      Value(val), 
      Sigma(sig) { }
  ~XLEQ() {
    for( int i=0; i < Vars.Count(); i++ )
      Vars[i]->_RemLeq(*this);
  }
  // copies Coefficients and Vars, internal use
  void _Assign(const XLEQ& leq);
  void AddMember(XVar& var, double coefficient=1.0) {
    Vars.Add(&var);
    Coefficients.Add(coefficient);
    var._AddLeq(*this);
  }
  int Count() const {  return Vars.Count();  }
  const XVar& operator [] (int i) const {  return *Vars[i];  }
  XVar& operator [] (int i) {  return *Vars[i];  }
  double GetCoefficient(int i) const {  return Coefficients[i];  }
  // validates that the equation is valid, if not - releases the variables
  bool Validate() {
    int vc = 0;
    for( int i=0; i < Vars.Count(); i++ )  {
      if( Vars[i]->IsUsed() )
        vc++;
      else  {
        Vars[i]->_RemLeq(*this);
        Vars.Delete(i);
        i--;
      }
    }
    if( vc < 2 )  {
      for( int i=0; i < Vars.Count(); i++ )
        Vars[i]->_RemLeq(*this);
      Vars.Clear();
    }
    return vc >= 2;
  }
  DefPropP(double, Value)
  DefPropP(double, Sigma)
  DefPropP(int, Id)
  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& vars);
#endif
  static XLEQ& FromDataItem(const TDataItem& item, XVarManager& parent);
};

class XVarManager {
  TTypeList<XVar> Vars;
  TTypeList<XLEQ> Equations;
  TTypeList<XVarReference> References;  
  int NextVar;  // this controls there variables go in sebsequent calls
public:

  class RefinementModel& RM;
  
  static olxstr RelationNames[];
  static short RelationIndex(const olxstr& rn);
  
  XVarManager(RefinementModel& rm);
  
  XVar& NewVar(double val = 0.5)  {  
    XVar* v = new XVar(*this, val);
    v->SetId(Vars.Count());
    return Vars.Add( v );  
  }
  // returns existing variable or creates a new one. Sets a limit of 1024 variables
  XVar& GetReferencedVar(int ind) {
    if( ind < 1 || ind > 1024 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid variable reference");
    while( Vars.Count() < ind )
      NewVar();
    return Vars[ind-1];
  }
  int VarCount()            const {  return Vars.Count();  }
  const XVar& GetVar(int i) const {  return Vars[i];  }
  XVar& GetVar(int i)             {  return Vars[i];  }

  XLEQ& NewEquation(double val=1.0, double sig=0.01)   {  
    XLEQ* leq = new XLEQ(*this, val, sig);
    leq->SetId(Equations.Count());
    return Equations.Add(leq);  
  }
  int EquationCount()       const {  return Equations.Count();  }
  const XLEQ& GetEquation(int i) const {  return Equations[i];  }
  XLEQ& GetEquation(int i)        {  return Equations[i];  }
  XLEQ& ReleaseEquation(int i)    {  
    Equations[i].SetId(-1);
    XLEQ& eq = Equations.Release(i);
    for( int i=0; i < Equations.Count(); i++ )
      Equations[i].SetId(i);
    return eq;
  }

  int VarRefCount() const {  return References.Count();  }
  XVarReference& GetVarRef(int i) {  return References[i];  }
  const XVarReference& GetVarRef(int i) const {  return References[i];  }
  // clears all the data and Nulls atoms' varrefs
  void ClearAll();
  void Clear() {  // does not clear the data, just resets the NextVar
    NextVar = 0;  // the global scale factor
    Equations.Clear(); // these are recreatable
  }
  /* sets a relation between an atom parameter and a variable, if the coefficient is -10 (default value), 
  the atom degenerocy is taken
  */
  XVarReference& AddVarRef(XVar& var, IXVarReferencer& r, short var_name, short relation, double coefficient);
  // for parsing...
  XVarReference& AddVarRef(XVarReference& ref) {  References.Add(ref).SetId(References.Count());  return ref;  }
  // releases a reference to the variable, must be deleted, unless restored
  XVarReference* ReleaseRef(IXVarReferencer& r, short var_name); 
  // restrores previously released var reference
  void RestoreRef(IXVarReferencer& r, short var_name, XVarReference* vr);
  // removes all unused variables and invalid/incomplete equations
  void Validate();
  // helps with parsing SHELX specific paramter representation, returns actual value of the param
  double SetParam(IXVarReferencer& r, short param_name, double val);
  void FixParam(IXVarReferencer& r, short param_name);
  void FreeParam(IXVarReferencer& r, short param_name);
  // retruns a SHELX specific value like 20.5 or 11.0
  double GetParam(const IXVarReferencer& r, short param_name) const {
    return GetParam(r, param_name, r.GetValue(param_name));
  }
  // uses the override_val instead of the call to GetValue(param_name)
  double GetParam(const IXVarReferencer& r, short param_name, double override_val) const;
  // parses FVAR and assignes variable values
  template <class list> void AddFVAR(const list& fvar) {
    for( int i=0; i < fvar.Count(); i++, NextVar++ )  {
      if( Vars.Count() <= NextVar )
        NewVar(fvar[i].ToDouble());
      else 
        Vars[NextVar].SetValue( fvar[i].ToDouble() );
    }
  }
  olxstr GetFVARStr() const {
    olxstr rv(Vars.IsEmpty() ? 1.0 : Vars[0].GetValue());
    for( int i=1; i < Vars.Count(); i++ )
      rv << ' ' << Vars[i].GetValue();
    return rv;
  }
  template <class list> void AddSUMP(const list& sump) {
    if( sump.Count() < 6 )
      throw TInvalidArgumentException(__OlxSourceInfo, "at least six parameters expected for SUMP");
    if( (sump.Count()%2) != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "even number of arguments is expected for SUMP");
    XLEQ& le = NewEquation(sump[0].ToDouble(), sump[1].ToDouble());
    for( int i=2; i < sump.Count(); i+=2 )  {
      XVar& v = GetReferencedVar(sump[i+1].ToInt());
      le.AddMember(v, sump[i].ToDouble());
    }
  }
  // Validate must be called first to get valid count of equations
  olxstr GetSUMPStr(int ind) const {
    XLEQ& le = Equations[ind];
    olxstr rv(le.GetValue());
    rv << ' ' << le.GetSigma();
    for( int i=0; i < le.Count(); i++ ) 
      rv << ' ' << le.GetCoefficient(i) << ' ' << le[i].GetId()+1;
    return rv;
  }
  void Assign(const XVarManager& vm);

  void Describe(TStrList& lst);

  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(const TDataItem& item);
};


EndXlibNamespace()

#endif
