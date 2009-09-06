/*
The open source library.
www.ccp14.ac.uk/ccp/web-mirors/lcells/opensrc/
c 2004 Oleg V. Dolomanov
*/
//---------------------------------------------------------------------------

#ifndef evailnkH
#define evailnkH
#include <math.h>
#include "emath.h"
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
/////////////////////////////////////////////////////////////////////////////////////////////////////
struct ExpParser  {
  static const olxstr control_chars;
  static const olxstr operators[];
  static bool ParseString(const olxstr& exp, olxstr& dest, int& ind)  {
    const olxch qc = exp.CharAt(ind);
    dest << qc;
    bool end_found = false;
    //const int start = ind;
    while( ++ind < exp.Length() )  {
      if( exp.CharAt(ind) == qc && exp.CharAt(ind-1) != '\\' )  {
        dest << qc;
        end_found = true;
        break;
      }
      dest << exp.CharAt(ind);
    }
//    if( end_found )
//      dest = exp.SubString(start, ind - start);
    return end_found;
  }
  static bool ParseBrackets(const olxstr& exp, olxstr& dest, int& ind)  {
    const olxch oc = exp.CharAt(ind), 
      cc = (oc == '(' ? ')' : (oc == '[' ? ']' : (oc == '{' ? '}' : '#')));
    if( cc == '#' )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid bracket char: ") << oc );
    int bc = 1;
//    const int start = ind+1;
    while( ++ind < exp.Length() && bc != 0 )  {
      const olxch ch = exp.CharAt(ind);
      if( ch == cc )       bc--;
      else if( ch == oc )  bc++;
      if( bc != 0 )
        dest << ch;
    }
    ind--;
    //if( bc == 0 )  {
    //  dest = exp.SubString(start, ind-start);
    //  return true;
    //}
    //return false;
    return bc == 0;
  }
  static bool IsOperator(const olxstr& exp)  {
    bool found = false;
    for( int i=0; i < 30; i++ )  {
      if( operators[i] == exp )  {
        found = true;
        break;
      }
    }
    return found;
  }
  static bool ParseControlChars(const olxstr& exp, olxstr& dest, int& ind)  {
    // use of substring will be more efficient
    while( ind < exp.Length() && control_chars.IndexOf(exp.CharAt(ind)) != -1 )  {
      dest << exp.CharAt(ind++);
      if( !IsOperator(dest) )  {
        dest.SetLength(dest.Length()-1);
        ind--;
        break;
      }
    }
    return !dest.IsEmpty();
  }
  static bool IsExpandable(const olxstr& exp)  {
    for( int i=0; i < exp.Length(); i++ )  {
      const olxch ch = exp.CharAt(i);
      if( ch == '(' )
        return true;
      if( ch == '"' || ch == '\'' )  {  // skip strings
        while( ++i < exp.Length() && exp.CharAt(i) != ch && exp.CharAt(i-1) != '\\' )
          ;
        continue;
      }
      if( control_chars.IndexOf(ch) != -1 )
        return true;
    }
    return false;
  }
  static inline bool IsBracket(const olxch& ch)  {
    return ch == '(' || ch == '[' || ch == '{';
  }
  template <class T> struct evaluator  {
    olxstr name;
    T* arg;
    evaluator(const olxstr& _name, T* _arg) : name(_name), arg(_arg) {}
    ~evaluator()  {  delete arg;  }
  };
  struct exp_tree  {
    olxstr data;
    exp_tree *parent, *left, *right;
    evaluator<exp_tree>* evator;
    bool priority;  // the expression is in brackets
    exp_tree(exp_tree* p, const olxstr& dt, exp_tree* l, exp_tree* r, evaluator<exp_tree>* e) : 
      parent(p), data(dt), left(l), right(r), evator(e), priority(false) {}
    exp_tree(exp_tree* p, const olxstr& dt) : 
      parent(p), data(dt), left(NULL), right(NULL), evator(NULL), priority(false) {}
    ~exp_tree()  {
      if( left != NULL )  delete left;
      if( right != NULL )  delete right;
      if( evator != NULL )  delete evator;
    }
    void expand()  {
      data = data.TrimWhiteChars();
      if( !IsExpandable(data) )  return;
      
      olxstr dt;
      for( int i=0; i < data.Length(); i++ )  {
        olxch ch = data.CharAt(i);
        if( olxstr::o_iswhitechar(ch) )
          continue;
        else if( ch == '(' )  {  // parse out brackets
          olxstr arg;
          if( !ParseBrackets(data, arg, i) ) 
            throw TInvalidArgumentException(__OlxSourceInfo, "problem with brackets");
          arg = arg.TrimWhiteChars();
          dt = dt.TrimWhiteChars();
          if( arg.IsEmpty() ) // empty arg list
            evator = new evaluator<exp_tree>(dt, NULL);
          else  {
            left = new exp_tree(this, arg);
            left->expand();
            if( !dt.IsEmpty() )  { // ()
              evator = new evaluator<exp_tree>(dt, left);
              left = NULL;
            }
            else if( i+1 >= data.Length() )  {  // nothing else? then move one level up...
              exp_tree* eta = left;
              left = eta->left;
              right = eta->right;
              evator = eta->evator;
              data = eta->data;
              eta->left = eta->right = NULL;
              eta->evator = NULL;
              delete eta;
              priority = true;
            }
          }
          dt.SetLength(0);
        }
        else if( ch == '"' || ch == '\'' )  { // parse out the strings
          if( !ParseString(data, dt, i) ) 
            throw TInvalidArgumentException(__OlxSourceInfo, "problem with quotations");
        }
        else  { 
          olxstr opr;
          if( ParseControlChars(data, opr, i) )  {
            if( !dt.IsEmpty() )  {
              if( left != NULL )
                throw TInvalidArgumentException(__OlxSourceInfo, "invalid expression");
              left = new exp_tree(this, dt);
              left->expand();
            }
            right = new exp_tree(this, data.SubStringFrom(i));
            right->expand();
            data = opr;
          }
          else
            dt << ch;
        }
      }
    }
    //void list_fv(TStrList& vars, TStrList& funcs)  {
    //  if( evator != NULL )
    //    funcs.Add( evator->name );

    //}
    bool validate() const  {
      if( parent != NULL )  {
        if( left != NULL && right != NULL )
          return left->validate() && right->validate();
      }
      else  {
        if( right != NULL )
          return right->validate();
        return !data.IsEmpty();
      }
    }
  };
  exp_tree* root;
  void expand()  {  root->expand();  }
  bool validate()  {
    
  }
  ExpParser( const olxstr& exp) {
    root = new exp_tree(NULL, exp);
  }
  ~ExpParser()  {  delete root;  }
};
///////////////////////////////////////////////////////////////////////////////////////////
struct SExpression  {
  struct IEvaluable {
    virtual ~IEvaluable() {}
    virtual double Evaluate() const = 0;
  };
  struct ConstEvaluable : public IEvaluable  {
    double val;
    ConstEvaluable(const double& _val) : val(_val)  {}
    virtual double Evaluate() const {  return val;  }
  };

  TDoubleList VarValues;
  TStrList VarNames;
  olxdict<olxstr, IEvaluable*, olxstrComparator<false> > UserFunctions;

  struct Variable : public IEvaluable  {
    size_t index;
    const SExpression& scope;
    Variable(const SExpression& _scope, size_t _index) : scope(_scope), index(_index) {}
    double Evaluate() const {  return scope.VarValues[index];  }
  };
  // user functions are re-used, where as a new instance of built-ins are created
  struct IUserFunction {
    virtual double DoEvaluate(const TPtrList<IEvaluable>& args) const = 0;
  };
  struct UserFunction : public IEvaluable  {
    IUserFunction& ufunc;
    TPtrList<IEvaluable> args;
    UserFunction(IUserFunction& uf) : ufunc(uf)  {}
    ~UserFunction()  {
      for( int i=0; i < args.Count(); i++ )
        delete args[i];
    }
    virtual double Evaluate() const {  return ufunc.DoEvaluate(args);  }
  };

  struct BuiltInsFactory {
    struct IConstFunc : public IEvaluable  {
      IEvaluable* arg;
      IConstFunc(IEvaluable* _arg) : arg(_arg) {}
      ~IConstFunc()  {  delete arg;  }
      virtual double Evaluate() const = 0;
    };
    struct AbsFunc : public IConstFunc  {
      AbsFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return olx_abs(arg->Evaluate());  }
    };
    struct CosFunc : public IConstFunc  {
      CosFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return cos(arg->Evaluate());  }
    };
    struct SinFunc : public IConstFunc  {
      SinFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return sin(arg->Evaluate());  }
    };
    struct TanFunc : public IConstFunc  {
      TanFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return tan(arg->Evaluate());  }
    };
    struct IOperator2 : public IEvaluable {
      IEvaluable *a, *b;
      IOperator2(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b)  {}
      ~IOperator2()  {
        delete a;
        delete b;
      }
    };
    struct AddOperator : public IOperator2  {
      AddOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return a->Evaluate() + b->Evaluate();  }
    };
    struct SubOperator : public IOperator2  {
      SubOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return a->Evaluate() - b->Evaluate();  }
    };
    struct DivOperator : public IOperator2  {
      DivOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return a->Evaluate() / b->Evaluate();  }
    };
    struct MulOperator : public IOperator2  {
      MulOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return a->Evaluate() * b->Evaluate();  }
    };
    struct RemOperator : public IOperator2  {
      RemOperator(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return (int)a->Evaluate() % (int)b->Evaluate();  }
    };
    struct PowFunc : public IOperator2  {
      PowFunc(IEvaluable* a, IEvaluable * b) : IOperator2(a,b)  {}
      virtual double Evaluate() const {  return pow(a->Evaluate(), b->Evaluate());  }
    };
    struct ChsOperator : public IEvaluable  {
      IEvaluable* arg;
      ChsOperator(IEvaluable* _arg) : arg(_arg)  {}
      ~ChsOperator()  {  delete arg;  }
      virtual double Evaluate() const {  return -arg->Evaluate();  }
    };
    struct PlusOperator : public IEvaluable  {
      IEvaluable* arg;
      PlusOperator(IEvaluable* _arg) : arg(_arg)  {}
      ~PlusOperator()  {  delete arg;  }
      virtual double Evaluate() const {  return arg->Evaluate();  }
    };

    struct ICmpOperator2 : public IEvaluable {
      IEvaluable *a, *b;
      ICmpOperator2(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b)  {}
      ~ICmpOperator2()  {
        delete a;
        delete b;
      }
    };
    struct EOperator : public ICmpOperator2  {
      EOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return olx_abs(a->Evaluate()-b->Evaluate()) < 1e-15;  }
    };
    struct NEOperator : public ICmpOperator2  {
      NEOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return olx_abs(a->Evaluate()-b->Evaluate()) > 1e-15;  }
    };
    struct GOperator : public ICmpOperator2  {
      GOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return a->Evaluate() > b->Evaluate();  }
    };
    struct GEOperator : public ICmpOperator2  {
      GEOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return a->Evaluate() >= b->Evaluate();  }
    };
    struct LOperator : public ICmpOperator2  {
      LOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return a->Evaluate() < b->Evaluate();  }
    };
    struct LEOperator : public ICmpOperator2  {
      LEOperator(IEvaluable* a, IEvaluable* b) : ICmpOperator2(a, b) {}
      virtual double Evaluate() const {  return a->Evaluate() <= b->Evaluate();  }
    };

    struct AndOperator : public IEvaluable  {
      IEvaluable *a, *b;
      AndOperator(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b)  {}
      ~AndOperator()  {  delete a;  delete b;  }
      virtual double Evaluate() const {  return a->Evaluate() && b->Evaluate();  }
    };
    struct OrOperator : public IEvaluable  {
      IEvaluable *a, *b;
      OrOperator(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b)  {}
      ~OrOperator()  {  delete a;  delete b;  }
      virtual double Evaluate() const {  return a->Evaluate() || b->Evaluate();  }
    };
    struct NotOperator : public IEvaluable  {
      IEvaluable* arg;
      NotOperator(IEvaluable* _arg) : arg(_arg)  {}
      virtual double Evaluate() const {  return !arg->Evaluate();  }
    };

    static IEvaluable* Create(const olxstr& name, IEvaluable* a, IEvaluable* b=NULL)  {
      if( a == NULL && b == NULL )  {
        return NULL;
      }
      else if( b == NULL )  {
        if( name == "abs" )  return new AbsFunc(a);
        if( name == "cos" )  return new CosFunc(a);
        if( name == "sin" )  return new SinFunc(a);
        if( name == "tan" )  return new TanFunc(a);
        if( name == "-" )  return new ChsOperator(a);
        if( name == "+" )  return new PlusOperator(a);
        if( name == "!" )  return new NotOperator(a);
      }
      else  {
        if( name == '+' )  return new AddOperator(a, b);
        if( name == '-' )  return new SubOperator(a, b);
        if( name == '/' )  return new DivOperator(a, b);
        if( name == '*' )  return new MulOperator(a, b);
        if( name == '%' )  return new RemOperator(a, b);
        if( name == "pow" ) return new PowFunc(a, b);
        if( name == "&&" )  return new AndOperator(a, b);
        if( name == "||" )  return new OrOperator(a, b);
        if( name == "==" )  return new EOperator(a, b);
        if( name == "!=" )  return new NEOperator(a, b);
        if( name == '>' )  return new GOperator(a, b);
        if( name == '<' )  return new LOperator(a, b);
        if( name == ">=" )  return new GEOperator(a, b);
        if( name == "<=" )  return new LEOperator(a, b);
      }
      return NULL;
    }
    static bool IsLogical(const olxstr& name) {  
      return name == "||" || name == "&&" || name == '!';
    }
    static bool IsCmp(const olxstr& name) {  
      return name == "==" || name == "!=" || name == '>' || name == '<' || name == ">=" || name == "<=";
    }
    static bool IsArithmetic(const olxstr& name) {  
      return name == '+' || name == '-' || name == '*' || name == '/' || name == '%';
    }
    static bool HasArithmeticPriority(const olxstr& name) {  
      return name == '*' || name == '/';
    }
  };

  static bool needsSorting(ExpParser::exp_tree* root)  {
    if( BuiltInsFactory::IsCmp(root->data) || BuiltInsFactory::IsArithmetic(root->data) )  {
      if( root->right != NULL && BuiltInsFactory::IsLogical(root->right->data) )  {
        if( root->right->left == NULL )  // +/- number
          return false;
        return !root->right->priority;
      }
    }
    if( BuiltInsFactory::HasArithmeticPriority(root->data) )  {
      if( root->right != NULL && 
         (BuiltInsFactory::IsArithmetic(root->right->data) && 
          !BuiltInsFactory::HasArithmeticPriority(root->right->data)) )  
      {
        if( root->right->left == NULL )  // +/- number
          return false;
        return !root->right->priority;
      }
    }
    return false;
  }
  static ExpParser::exp_tree* sortLogical(ExpParser::exp_tree* root)  {
    // need to test that it is not just -number... root->right->left does it?
    if( root->parent == NULL && root->left == NULL && root->right != NULL && root->right->left != NULL )  {
      ExpParser::exp_tree* top = root->right;
      root->right = top->left;
      top->parent = root->parent;
      root->parent = top;
      top->left = root;
      return sortLogical(top);
    }
    if( needsSorting(root) )  {
      if( root->parent != NULL && root->left == NULL )  {
        ExpParser::exp_tree* top = root->right;
        root->right = top->left;
        top->parent = root->parent->parent;
        top->left = root->parent;
        if( top->right != NULL )
          top->right = sortLogical(top->right);
        return top;
      }
      else  {
        ExpParser::exp_tree* top = root->right;
        root->right = top->left;
        top->left = root;
        top->parent = root->parent;
        root->parent = top;
        if( top->right != NULL )
          top->right = sortLogical(top->right);
        return top;
      }
    }
    if( root->right != NULL )  {
      ExpParser::exp_tree *original = root->right,
        *result = sortLogical(root->right);
      if( result != original )
        return result;
    }
    return root;
  }
  IEvaluable* EvaluatorFromEvator(ExpParser::exp_tree* root)  {
    IEvaluable *arg = CreateEvaluator(root->evator->arg);
    if( arg == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "could not find appropriate evlauable");
    IEvaluable *rv = BuiltInsFactory::Create(root->evator->name, arg);
    // need to test if the function result cannot be changed

    if( EsdlInstanceOf(*arg, ConstEvaluable) && dynamic_cast<BuiltInsFactory::IConstFunc*>(rv) != NULL )  {
      IEvaluable* ce = new ConstEvaluable(rv->Evaluate());
      delete rv;
      return ce;
    }
    return rv;
  }
  IEvaluable* CreateEvaluator(ExpParser::exp_tree* root)  {
    if( root->left != NULL || root->right != NULL )  {
      IEvaluable *left = NULL, *right = NULL;
      try  {
        if( root->left == NULL )  {
          if( root->evator != NULL )
            left = EvaluatorFromEvator(root);
        }
        else
          left = CreateEvaluator(root->left);
        right = (root->right != NULL ? CreateEvaluator(root->right) : NULL);
      }
      catch(const TExceptionBase& exc)  {
        if( left != NULL )  delete left;
        if( right != NULL )  delete right;
        throw TFunctionFailedException(__OlxSourceInfo, exc, "could not find appropriate evlauable");
      }
      if( left == NULL && right != NULL )  {
        left = right;
        right = NULL;
      }
      IEvaluable* rv = BuiltInsFactory::Create(root->data, left, right);
      if( rv == NULL )  {
        if( left != NULL )  delete left;
        if( right != NULL )  delete right;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not create specified evaluator: ") << root->data);
      }
      if( left != NULL && right != NULL )  {
        if( EsdlInstanceOf(*left, ConstEvaluable) && EsdlInstanceOf(*right, ConstEvaluable) )  {
          IEvaluable* ce = new ConstEvaluable(rv->Evaluate());
          delete rv;
          return ce;
        }
      }
      else if( left != NULL )  {
        if( EsdlInstanceOf(*left, ConstEvaluable) )  {
          IEvaluable* ce = new ConstEvaluable(rv->Evaluate());
          delete rv;
          return ce;
        }
      }
      return rv;
    }
    else  {
      if( root->evator != NULL )
        return EvaluatorFromEvator(root);
      else  {
        if( root->data.IsNumber() )
          return new ConstEvaluable(root->data.ToDouble());
        else  {
          if( root->data.Equalsi("PI") )
            return new ConstEvaluable(M_PI);
          else if( root->data.Equalsi("true") )
            return new ConstEvaluable(1.0);
          else if( root->data.Equalsi("false") )
            return new ConstEvaluable(0.0);

          int ind = VarNames.IndexOf(root->data);
          if( ind == -1 )  {
            VarNames.Add( root->data );
            VarValues.Add(0);
            ind = VarNames.Count()-1;
          }
          return new Variable(*this, ind);
        }
      }
    }
  }
public:
  IEvaluable* Build(const olxstr& exp)  {
    // c == 3 || c == -1
    ExpParser expp(exp);
    expp.expand();
    expp.root = sortLogical(expp.root);
    return CreateEvaluator(expp.root);
  }
};

#endif
