#ifndef __olx_liner_eq_h
#define __olx_liner_eq_h
#include "xbase.h"
#include "typelist.h"
BeginXlibNamespace()

const short  // variable names
  var_name_Scale = 0,
  var_name_X     = 1,
  var_name_Y     = 2,
  var_name_Z     = 3,
  var_name_Sof   = 4,
  var_name_Uiso  = 5,
  var_name_U11   = 6,
  var_name_U22   = 7,
  var_name_U33   = 8,
  var_name_U12   = 9,
  var_name_U13   = 10,
  var_name_U23   = 11;

const short  // relation of parameters to variables
  relation_AsVar         = 1,
  relation_AsOneMinusVar = 2;

/* Any single variable can be shared by several atom parameters (to make them equal) as well 
as by several linear equations
*/

class XLEQ;
class XVar;
class XVarManager;
struct XVarReference;

struct XVarReference {
  class TCAtom* atom;
  short var_name;  // one of the var_name
  short relation_type; // relationAsVar or relation_AsOneMinusVar
  double coefficient; // line 0.25 in 20.25
  XVarReference(TCAtom* a, short _var_name, short _relation_type, double coeff=1.0) : 
    atom(a), 
    var_name(_var_name), 
    relation_type(_relation_type), 
    coefficient(coeff) { }
};

class XVar {
  double Value;
  TTypeList<XVarReference> References;  
  TPtrList<XLEQ> Equations; // equations using this variable
  int Id;
public:
  XVar() : Value(0.5), Id(-1) { }
  // adds a new atom, referencing this variable
  XVarReference& AddReference(TCAtom* a, short var_name, short relation, double coefficient=1.0)  {
    return References.AddNew(a, var_name, relation, coefficient);
  }
  // removes a refence, it is destroyed here!
  void RemoveReference(XVarReference& vr)  {
    for( int i=0; i < References.Count(); i++ )  {
      if( &References[i] == &vr )  {
        References.Delete(i);
        break;
      }
    }
  }
  // calculates the number of atoms (excluding the deleted ones) using this variable
  int ReferenceCount() const;

  // adds a new equation, referencing this variable
  void AddEquation(XLEQ& eq)  {  Equations.Add(&eq);  }
  // removes an equation, referencing this variable
  void RemoveEquation(XLEQ& eq)  {  Equations.Remove(&eq);  }
  // returns the number of equations, referencing this variable
  int EquationCount() const {  return Equations.Count();  }
  bool IsUsed() const {
    return !(ReferenceCount() == 0 || EquationCount() == 0);
  }
  DefPropP(double, Value)
  DefPropP(int, Id)  
};

class XLEQ {
  double Value, Sigma;
  TDoubleList Coefficients;
  TPtrList<XVar> Variables;
public:
  XLEQ(double val, double sig) : Value(val), Sigma(sig) { }
  void AddMember(XVar& var, double coefficient) {
    Variables.Add(&var);
    Coefficients.Add(coefficient);
    var.AddEquation(*this);
  }
  int Count() const {  return Variables.Count();  }
  const XVar& operator [] (int i) const {  return *Variables[i];  }
  XVar& operator [] (int i) {  return *Variables[i];  }
  double GetCoefficient(int i) const {  Coefficients[i];  }
  // validates that the equation is valid, if not - releases the variables
  bool Validate() {
    int vc = 0;
    for( int i=0; i < Variables.Count(); i++ )  {
      if( Variables[i]->IsUsed() )
        vc++;
    }
    if( vc < 2 )  {
      for( int i=0; i < Variables.Count(); i++ )
        Variables[i]->RemoveEquation(*this);
    }
    return vc >= 2;
  }
  DefPropP(double, Value)
  DefPropP(double, Sigma)

};

class XVarManager {
  TTypeList<XVar> Vars;
  TTypeList<XLEQ> Equations;
  int NextVar;  // this controls there variables go in sebsequent calls
public:
  XVarManager() {
    NextVar = 0;
  }
  
  XVar& NewVar()                  {  return Vars.AddNew();  }
  // returns existing variable or creates a new one. Sets a limit of 1024 variables
  XVar& GetReferencedVar(int ind) {
    if( ind < 0 || ind > 1024 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid variable reference");
    while( Vars.Count() <= ind )
      Vars.AddNew();
    return Vars[ind];
  }
  int VarCount()            const {  return Vars.Count();  }
  const XVar& GetVar(int i) const {  return Vars[i];  }
  XVar& GetVar(int i)             {  return Vars[i];  }

  XLEQ& NewEquation(double val, double sig)   {  return Equations.AddNew(val, sig);  }
  int EquationCount()       const {  return Equations.Count();  }
  const XLEQ& GetEquation(int i) const {  return Equations[i];  }
  XLEQ& GetEquation(int i)        {  return Equations[i];  }
  void Clear()  {
    Equations.Clear();
    Vars.Clear();
    NextVar = 0;  // the global scale factor
  }
  // removes all unused variables and invalid/incomplete equations
  void Validate() {
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
    for( int i=1; i < Vars.Count(); i++ )  // start from 1 to leave global scale
      if( !Vars[i].IsUsed() )
        Vars.NullItem(i);
    Equations.Pack();
    Vars.Pack();
    for( int i=0; i < Vars.Count(); i++ )
      Vars[i].SetId(i);
  }

  template <class list> void AddFVAR(const list& fvar) {
    for( int i=0; i < fvar.Count(); i++, NextVar++ )  {
      if( Vars.Count() <= NextVar )
        Vars.AddNew(fvar[i].ToDouble());
      else
        Vars[NextVar] = fvar[i].ToDouble();
    }
  }
  olxstr GetFVARStr() const {
    olxstr rv(Vars.IsEmpty() ? 1.0 : Vars[0].GetValue());
    for( int i=1; i < Vars.Count(); i++ )
      rv << ' ' << Vars[i].GetValue();
  }
  template <class list> void AddSUMP(const list& sump) {
    if( sump.Count() < 6 )
      throw TInvalidArgumentException(__OlxSourceInfo, "at least six parameters expected for SUMP");
    XLEQ& le = NewEquation(sump[0].ToDouble(), sump[1].ToDouble());
    for( int i=2; i < sump.Count(); i++ )  {
      XVar& v = GetRerefencedVar(sump[i+1].ToInt());
      le.AddMember(v, sump[i].ToDouble());
    }
  }
  // Validate must be called first to get valid count of equations
  olxstr GetSUMPStr(int ind) const {
    XLEQ& le = Equations[ind];
    olxstr rv(le.GetValue());
    rv << ' ' << le.GetSigma();
    for( int i=0; i < le.Count(); i++ ) 
      rv << ' ' << le.GetCoefficient(i) << ' ' << le[i].GetId();
    return rv;
  }
};


EndXlibNamespace()

#endif
