/*
The open source library.
www.ccp14.ac.uk/ccp/web-mirors/lcells/opensrc/
c 2004 Oleg V. Dolomanov
*/
//---------------------------------------------------------------------------

#ifndef evailnkH
#define evailnkH
#include "eobjects.h"
#include "estrlist.h"
#include "talist.h"
//---------------------------------------------------------------------------
/*
How to use:
when an expression is passed to LoadFromEpression the lits of variales is being
initialised with variable names. Before the Evaluate is called the array of 
IVariables has to beinitialised with the values of the variables. Use 
Variables->IndexOf(VarName) to identify the index of variables.
The Functions have to be predefined. If no variables is expected when the 
constructur can be caled as TSOperatio(NULL, NULL, Function, NULL). If the functions
are not in the expression call TSOperatio(NULL, NULL, NULL, NULL).
Note that the variables list is in the upper case.
*/
const short fPlus      = 1,
            fMinus     = 2,
            fMultiply  = 3,
            fDivide    = 4,
            fFunction  = 5,
            fExt       = 6,
            fRemainder = 7;
const short fNone = 0,
            fAbs  = 1,
            fCos  = 2,
            fSin   = 3,
            fDefined = 4;  // defined by an external program

typedef double (*TEvaluate)(double P1, double P2);

class TSOperation  {
protected:
  TSOperation *ToCalc;  // if evaluable == true these are in use
  TSOperation *Left, *Right;
  short Function;
  double Value;
  bool ChSig;
  void Calculate();
  void SSCalculate();
  void MDCalculate();
  int VariableIndex;
  TSOperation *Parent;
  olxstr FExp;
public:
  TDoubleList* IVariables;
  TStrPObjList<olxstr,TSOperation*>* Variables;
  TStrList* Functions;

  TSOperation(TSOperation* P, TStrPObjList<olxstr,TSOperation*>* Vars, 
    TStrList* Funcs, TDoubleList* IVariables);
  ~TSOperation();
  TEvaluate *Evaluator;
  bool Expandable;
  olxstr Func, Param;
  short Operation;
  double P1, P2;

  double Evaluate();
  int LoadFromExpression(const olxstr &Exp);
};

template <typename Type, typename Left, typename Right> struct AddOperator  {
  Left* left;
  Right* right;
public:
  AddOperator(Left* _left, Right* _right) : left(_left), right(_right) {}
  Type Evaluate() const {  return left->Evaluate() + right->Evaluate();  }
};
template <typename Type, typename Left, typename Right> struct SubOperator  {
  Left* left;
  Right* right;
public:
  SubOperator(Left* _left, Right* _right) : left(_left), right(_right) {}
  Type Evaluate() const {  return left->Evaluate() + right->Evaluate();  }
};
template <typename Type, typename Left, typename Right> struct DivOperator  {
  Left* left;
  Right* right;
public:
  DivOperator(Left* _left, Right* _right) : left(_left), right(_right) {}
  Type Evaluate() const {  return left->Evaluate()/right->Evaluate();  }
};
template <typename Type, typename Left, typename Right> struct MulOperator  {
  Left* left;
  Right* right;
public:
  MulOperator(Left* _left, Right* _right) : left(_left), right(_right) {}
  Type Evaluate() const {  return left->Evaluate() * right->Evaluate();  }
};
template <typename Type, typename Right> struct ChSigOperator  {
  Right* right;
public:
  ChSigOperator(Right* _right) : right(_right) {}
  Type Evaluate() const {  return -right->Evaluate();  }
};


#endif
