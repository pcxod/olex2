/*
The open source library.
www.ccp14.ac.uk/ccp/web-mirors/lcells/opensrc/
c 2004 Oleg V. Dolomanov
*/
//---------------------------------------------------------------------------

#ifndef evailnkH
#define evailnkH
#include <math.h>
#include "eobjects.h"
#include "estrlist.h"
#include "talist.h"
#include "edict.h"
#include "paramlist.h"
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
//
//class SExpression  {
//  struct IEvaluable {
//    virtual ~IEvaluable() {}
//    virtual double Evaluate() const = 0;
//  };
//  struct ConstEvaluable : public IEvaluable  {
//    double val;
//    ConstEvaluable(const double& _val) : val(_val)  {}
//    virtual double Evaluate() const {  return val;  }
//  }
//
//  TDoubleList VarValues;
//  TStrList VarNames;
//  olxdict<olxstr, IEvaluable, olxstrComparator<false> > UserFunctions;
//
//  struct Variable : public IEvaluable  {
//    size_t index;
//    const SExpression& scope;
//    Variable(const SExpression& _scope, size_t _index) : scope(_scope), index(_index) {}
//    double Evaluate() const {  return scope.VarValues[index];  }
//  };
//  // user functions are re-used, where as a new instance of built-ins are created
//  struct IUserFunction {
//    virtual double Evaluate(const TPtrList<IEvaluable>& args) const = 0;
//  };
//  struct UserFunction : public IEvaluable  {
//    IUserFunction& ufunc;
//    TPtrList<IEvaluable> args;
//    UserFunction( IUserFunction& uf) : ufunc(uf)  {}
//    ~UserFunction()  {
//      for( int i=0; i < args.Count(); i++ )
//        delete args[i];
//    }
//    virtual double Evaluate() const {  return ufunc.DoEvaluate(args);  }
//
//  };
//  struct BuiltInFuncFactory {
//    struct IFunc : public IEvaluable  {
//      IEvaluable* arg;
//      IFunc(IEvaluable* _arg) : arg(_arg) {}
//      ~IFunc()  {  delete arg;  }
//      virtual double Evaluate() const = 0;
//    }
//    struct AbsFunc : public IFunc  {
//      AbsFunc(IEvaluable* arg) : IFunc(arg)  {}
//      virtual double Evaluate() const {  return olx_abs(arg->Evaluate);  }
//    }
//    struct CosFunc : public IFunc  {
//      CosFunc(IEvaluable* arg) : IFunc(arg)  {}
//      virtual double Evaluate() const {  return cos(arg->Evaluate);  }
//    }
//    struct SinFunc : public IFunc  {
//      SinFunc(IEvaluable* arg) : IFunc(arg)  {}
//      virtual double Evaluate() const {  return sin(arg->Evaluate);  }
//    }
//    struct TanFunc : public IFunc  {
//      TanFunc(IEvaluable* arg) : IFunc(arg)  {}
//      virtual double Evaluate() const {  return tan(arg->Evaluate);  }
//    }
//    static IEvaluable* Create(const olxstr& name, IEValuable* arg)  {
//      if( name == "abs" )  return new AbsFunc(arg);
//      if( name == "cos" )  return new CosFunc(arg);
//      if( name == "sin" )  return new SinFunc(arg);
//      if( name == "tan" )  return new tanFunc(arg);
//      return NULL;
//    }
//  };
//
//  struct OperatorFactory  {
//    struct IOperator2 : public IEvaluable {
//      IEvaluable *a, *b;
//      IOperator2(IEvaluable* _a, IEvaluabe* _b) : a(_a), b(_b)  {}
//      ~IOperator2()  {
//        delete a;
//        delete b;
//      }
//    }
//    struct AddOperator : public IOperator2  {
//      AddOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return a->Evaluate() + b->Evaluate();  }
//    }
//    struct SubOperator : public IOperator2  {
//      SubOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return a->Evaluate() - b->Evaluate();  }
//    }
//    struct DivOperator : public IOperator2  {
//      DivOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return a->Evaluate() / b->Evaluate();  }
//    }
//    struct MulOperator : public IOperator2  {
//      MulOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return a->Evaluate() * b->Evaluate();  }
//    }
//    struct RemOperator : public IOperator2  {
//      RemOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return (int)a->Evaluate() % (int)b->Evaluate();  }
//    }
//    struct PowOperator : public IOperator2  {
//      PowOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
//      virtual double Evaluate() const {  return pow(a->Evaluate(), b->Evaluate());  }
//    }
//    struct ChsOperator : public IEvaluable  {
//      IEvaluable* arg;
//      ChsOperator(IEvaluable* _arg) : arg(_arg)  {}
//      virtual double Evaluate() const {  return -arg->Evaluate();  }
//    }
//    static IEvaluable* Create(const olxstr& name, IEvaluable* a, IEvaluable* b)  {
//      if( name == '+' )  return AddOperator(a, b);
//      if( name == '-' )  return b == NULL ? ChsOperator(a) : SubOperator(a, b);
//      if( name == '/' )  return DivOperator(a, b);
//      if( name == '*' )  return MulOperator(a, b);
//      if( name == '%' )  return RemOperator(a, b);
//      if( name == '^' )  return PowOperator(a, b);
//      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown operator: ") << name);
//    }
//  };
//  IEvaluable* Parse(const olxstr& exp)  {
//    return NULL;
//  }
//  void ParseList(const olxstr& exp, TPtrList<IEvaluable>& rv)  {
//    TStrList list;
//    TParamList::StrtokParams(exp, ',', list);
//    for( int i=0; i < list.Count(); i++ )
//      rv.Add( CreateOperand(rv) );
//  }
//  IEvaluable* CreateOperand(const olxstr& op)  {
//    if( op.IsNumber() )
//      return new ConstEvaluable(op.ToDouble());
//    int bi = op.IndexOf('(');
//    if( bi != -1 )  {
//      int bc = 1, from = bi;
//      while( bi++ < op.Length() && bc != 0 )  {
//        if( op.CharAt(bi) == '(' )  
//          bc++;
//        else if( op.CharAt(bi) == ')' )
//          bc--;
//      }
//      if( bc != 0 )
//        throw TFunctionFailedException(__OlxSourceInfo, "brackets mismatch");
//      IEvaluable* arg = Parse(op.SubStringFrom(from+1, 1));
//      const olxstr name = op.SubStringTo(from);
//      IEvaluable* rv = BuiltInFuncFactory::Create(name, arg);
//      if( rv == NULL )  {
//        int fi = UserFunctions.IndexOf(name);
//        if( fi == -1 )  {
//          delete arg;
//          throw TFunctionFailedException(__OlxSourceInfo, olxstr("unknown function: ") << name);
//        }
//        ///...
//      }
//    }
//    else  {
//      int vi = VarNames.IndexOf(op);
//      if( vi == -1 )
//        throw TFunctionFailedException(__OlxSourceInfo, olxstr("unknown variable: ") << op);
//      return new Variable(*this, vi);
//    }
//  }
//
//  struct Entry  {
//    IEvaluable* op;
//    Entry* next; 
//    Entry() : op(NULL), next(NULL) {}
//  };
//  // string having no spaces/tabs must be provided
//  IEvaluable* ParseSimple(const olxstr& str)  {
//    Entry* root = new Entry, *en;
//    root->next = NULL;
//    en = root;
//    TIntList op_i;
//    for( int i=0; i < str.Length(); i++ )  {
//      const olxch ch = str.CharAt(i);
//      if( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' )
//        op_i.Add(i);
//    }
//    int po_i = 0;
//    for( int i=0; i < op_i.Count(); i++ )  {
//      olxstr lo = str.SubStringFrom(po_i, op_i[i]);
//      olxstr ro = str.SubStringFrom(op_i[i], ((i+1) < po_i ? op_i[i+1], 0));
//      if( lo.IsEmpty() )  {
//        if( ro.IsEmpty() )
//          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid expression :") << str);
//        if( i == 0 )  {  // chs?
//          if( str.CharAt(op_i[i]) == '-' )
//            en->op = OperatorFactory::Create('-', CreateOperand(ro), NULL);
//          else if( str.CharAt(op_i[i]) == '+' )
//            en->op = CreateOperand(ro);
//          else
//            throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid expression :") << str);
//          en = (en->next = new Entry);
//        }
//        else { // +- or -+
//          if( str.CharAt(op_i[i-1]) == '+'
//        }
//      }
//      else if( ro.IsEmpty() )  {
//        
//      }
//    }
//  }
//
//public:
//  void Build(const olxstr& exp)  {
//    for( int i=0; i < exp.Length(); i++ )  {
//      if( exp.CharAt(i) == '(' )  {
//        int cb = 1;
//        while( cb > 0 && i++ < exp.Length() )  {
//          if( exp.CharAt(i) == ')' )
//            cb--;
//        }
//        if( cb != 0 )
//          throw TFunctionFailedException(__OlxSourceInfo, "brackets mismatch");
//      }
//      if( 
//    }
//  }
//};

#endif
