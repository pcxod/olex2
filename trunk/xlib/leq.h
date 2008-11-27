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
  relation_None          = 0,
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
  short var_name;  // one of the var_type
  XVarReference(TCAtom* a, short _var_name) : atom(a), var_name(_var_name) {}
};

class XVar {
  double Value;
  TTypeList<XVarReference> References;  
  TPtrList<XLEQ> Equations; // equations using this variable
public:
  XVar() : Value(0.5) { }
  // adds a new atom, referencing this variable
  void AddReference(TCAtom* a, short var_name)  {
    References.AddNew(a, var_name);
  }
  // removes an atom, referencing this variable
  void RemoveReference(TCAtom* a, short var_name)  {
    for( int i=0; i < References.Count(); i++ )  {
      if( References[i].atom == a && References[i].var_name == var_name )  {
        References.Delete(i);
        break;
      }
    }
  }
  // calculates the number of atoms (excluding the deleted onces) using this variable
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
};

class XLEQ {
  double Value;
  TDoubleList Coefficients;
  TPtrList<XVar> Variables;
public:
  XLEQ(double val) : Value(val) { }
  void AddMember(XVar& var, double coefficient) {
    Variables.Add(&var);
    Coefficients.Add(coefficient);
  }
  DefPropP(double, Value)

};

class XVarManager {
  TTypeList<XVar> Vars;
  TTypeList<XLEQ> Equations;
public:
  XVarManager() {}
  
  XVar& NewVar()                  {  return Vars.AddNew();  }
  int VarCount()            const {  return Vars.Count();  }
  const XVar& GetVar(int i) const {  return Vars[i];  }
  XVar& GetVar(int i)             {  return Vars[i];  }

  XLEQ& NewEquation(double val)   {  return Equations.AddNew(val);  }
  int EquationCount()       const {  return Equations.Count();  }
  const XLEQ& GetEquation(int i) const {  return Equations[i];  }
  XLEQ& GetEquation(int i)        {  return Equations[i];  }

};


EndXlibNamespace()

#endif
