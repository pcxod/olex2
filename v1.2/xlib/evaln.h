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
const short fPlus     = 1,
            fMinus     = 2,
            fMultiply   = 3,
            fDivide    = 4,
            fFunction   = 5,
            fExt    = 6;
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
  void Calculate();
  void SSCalculate();
  void MDCalculate();
  int VariableIndex;
  TSOperation *Parent;
  olxstr FExp;
public:
  double *IVariables;
  TStrPObjList<olxstr,TSOperation*>* Variables;
  TStrList* Functions;

  TSOperation(TSOperation *P, TStrPObjList<olxstr,TSOperation*> *Vars, TStrList *Funcs, double *IVariables);
  ~TSOperation();
  TEvaluate *Evaluator;
  bool Expandable;
  olxstr Func, Param;
  short Operation;
  double P1, P2;

  double Evaluate();
  int LoadFromExpression(const olxstr &Exp);
};
#endif
