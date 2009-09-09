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
  static void SplitArgs(const olxstr& exp, TStrList& res)  {
    int start = 0;
    for( int i=0; i < exp.Length(); i++ )  {
      const olxch ch = exp.CharAt(i);
      if( ch == '(' )  {
        int bc = 1;
        while( ++i < exp.Length() && bc != 0 )  {
          if( exp.CharAt(i) == '(' )  bc++;
          else if( exp.CharAt(i) == ')' )  bc--;
        }
        i--;
      }
      else if( ch == '"' || ch == '\'' )  {  // skip strings
        while( ++i < exp.Length() && exp.CharAt(i) != ch && exp.CharAt(i-1) != '\\' )
          ;
      }
      else if( ch == ',' )  {
        res.Add( exp.SubString(start, i-start) ).TrimWhiteChars();
        start = i+1;
      }
    }
    if( start < exp.Length() )
      res.Add( exp.SubStringFrom(start) ).TrimWhiteChars();
  }
  static inline bool IsBracket(const olxch& ch)  {
    return ch == '(' || ch == '[' || ch == '{';
  }
  template <class T> struct evaluator  {
    olxstr name;
    TPtrList<T> args;
    evaluator(const olxstr& _name) : name(_name) {}
    ~evaluator()  {  
      for( int i=0; i < args.Count(); i++ )
        delete args[i];
    }
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
            evator = new evaluator<exp_tree>(dt);
          else  {
            TStrList args;
            SplitArgs(arg, args);
            if( args.Count() == 1 )  {
              left = new exp_tree(this, arg);
              left->expand();
              if( !dt.IsEmpty() )  { // ()
                evator = new evaluator<exp_tree>(dt);
                evator->args.Add(left);
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
            else  {
              evator = new evaluator<exp_tree>(dt);
              for( int ai=0; ai < args.Count(); ai++ )
                evator->args.Add(new exp_tree(this, args[ai]))->expand();
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
    
    typedef void* (*cast_operator)(const IEvaluable*);
    typedef olxdict<std::type_info const*, cast_operator, TPointerPtrComparator> operator_dict;
    virtual cast_operator get_cast_operator(const std::type_info&) const = 0;

    template <class T> T cast() const {
      const std::type_info& ti = typeid(T);
      try  {  
        cast_operator co = get_cast_operator(ti);
        if( co != NULL )  {
          T* cast_result = (T*)(*co)(this);
          T result(*cast_result);
          delete cast_result;
          return result;
        }
      }
      catch(...)  {}
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not cast to ") << ti.name());
    }
    template <class T> static const T* cast_helper(const IEvaluable* i)  {
      const T* ci = dynamic_cast<const T*>(i);
      if( ci == NULL )  throw TFunctionFailedException(__OlxSourceInfo, "cast failed");
      return ci;
    }
  };

  struct ANumberEvaluator : public IEvaluable  {
  protected:
    static IEvaluable::operator_dict cast_operators;
    static const IEvaluable::operator_dict::Entry cast_operators_table[];
    static void* bool_cast(const IEvaluable* i)  {  return new bool(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate() != 0);  }
    static void* str_cast(const IEvaluable* i)  {  return new olxstr(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate());  }
    template<class T> static void* primitive_cast(const IEvaluable* i)  {  return new T((T)IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate());  }
    template<class T> static void register_cast()  {  cast_operators.Add(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  }
    template<class T> static IEvaluable::operator_dict::Entry create_operator_entry()  {  
      return IEvaluable::operator_dict::Entry(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  return cast_operators[&ti];  } 
  public:
    ANumberEvaluator()  {}
  };
  //template <class BC>
  //struct TNumberEvaluator : public IEvaluable  {
  //protected:
  //  static IEvaluable::operator_dict cast_operators;
  //  static const IEvaluable::operator_dict::Entry cast_operators_table[];
  //  static void* bool_cast(const IEvaluable* i)  {  return new bool(BC::get_value() != 0);  }
  //  static void* str_cast(const IEvaluable* i)  {  return new olxstr(BC::get_value());  }
  //  template<class T> static void* primitive_cast(const IEvaluable* i)  {  return new T((T)BC::get_value());  }
  //  template<class T> static IEvaluable::operator_dict::Entry create_operator_entry()  {  
  //    return IEvaluable::operator_dict::Entry(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  
  //  }
  //  virtual cast_operator get_cast_operator(const std::type_info& ti) const {  return cast_operators[&ti];  } 
  //public:
  //  TNumberEvaluator()  {}
  //};
  struct ConstEvaluable : public ANumberEvaluator  {
  public:
    double val;
    ConstEvaluable(const double& _val) : val(_val)  {}
    virtual double Evaluate() const {  return val;  }
  };

  //template <class T>
  //struct NumberEvaluator : public TNumberEvaluator<NumberEvaluator<T> >  {
  //public:
  //  T val;
  //  NumberEvaluator(const T& _val) : val(_val)  {}
  //  const T& get_value() const {  return val;  }
  //  virtual double Evaluate() const {  return val;  }
  //  virtual IEvaluable* new_inst(const T& _val) const {  return new NumberEvaluator<T>(_val);  }
  //};

  TDoubleList VarValues;
  TStrList VarNames;
  olxdict<olxstr, IEvaluable*, olxstrComparator<false> > UserFunctions;

  struct Variable : public IEvaluable  {
    size_t index;
    const SExpression& scope;
    Variable(const SExpression& _scope, size_t _index) : scope(_scope), index(_index) {}
    double Evaluate() const {  return scope.VarValues[index];  }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  return NULL;  } 
  };
  // user functions are re-used, where as a new instance of built-ins are created
  struct IUserFunction {
    virtual double DoEvaluate(const TPtrList<IEvaluable>& args) const = 0;
    };
  struct UserFunction : public ANumberEvaluator  {
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
    struct IConstFunc : public ANumberEvaluator  {
      IEvaluable* arg;
      IConstFunc(IEvaluable* _arg) : arg(_arg) {}
      ~IConstFunc()  {  delete arg;  }
    };
    struct IConstFunc2 : public ANumberEvaluator  {
      IEvaluable* a, *b;
      IConstFunc2(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b) {}
      ~IConstFunc2()  {  delete a;  delete b;  }
    };
    struct AbsFunc : public IConstFunc  {
      AbsFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return olx_abs(arg->cast<double>());  }
    };
    struct CosFunc : public IConstFunc  {
      CosFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return cos(arg->cast<double>());  }
    };
    struct SinFunc : public IConstFunc  {
      SinFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return sin(arg->cast<double>());  }
    };
    struct TanFunc : public IConstFunc  {
      TanFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual double Evaluate() const {  return tan(arg->cast<double>());  }
    };
    struct PowFunc : public IConstFunc2  {
      PowFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual double Evaluate() const {  return pow(a->cast<double>(), b->cast<double>());  }
    };
    struct MinFunc : public IConstFunc2  {
      MinFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual double Evaluate() const {  return olx_min(a->cast<double>(), b->cast<double>());  }
    };
    struct MaxFunc : public IConstFunc2  {
      MaxFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual double Evaluate() const {  return olx_max(a->cast<double>(), b->cast<double>());  }
    };

    struct AddOperator : public IConstFunc2  {
      AddOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual double Evaluate() const {  return a->cast<double>() + b->cast<double>();  }
    };
    struct SubOperator : public IConstFunc2  {
      SubOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual double Evaluate() const {  return a->cast<double>() - b->cast<double>();  }
    };
    struct DivOperator : public IConstFunc2  {
      DivOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual double Evaluate() const {  return a->cast<double>() / b->cast<double>();  }
    };
    struct MulOperator : public IConstFunc2  {
      MulOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual double Evaluate() const {  return a->cast<double>() * b->cast<double>();  }
    };
    struct RemOperator : public IConstFunc2  {
      RemOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual double Evaluate() const {  return a->cast<long>() % b->cast<long>();  }
    };
    struct ChsOperator : public IConstFunc  {
      ChsOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual double Evaluate() const {  return -arg->cast<double>();  }
    };
    struct PlusOperator : public IConstFunc  {
      PlusOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual double Evaluate() const {  return arg->cast<double>();  }
    };

    struct EOperator : public IConstFunc2  {
      EOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() == b->cast<double>();  }
    };
    struct NEOperator : public IConstFunc2  {
      NEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() != b->cast<double>();  }
    };
    struct GOperator : public IConstFunc2  {
      GOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() > b->cast<double>();  }
    };
    struct GEOperator : public IConstFunc2  {
      GEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() >= b->cast<double>();  }
    };
    struct LOperator : public IConstFunc2  {
      LOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() < b->cast<double>();  }
    };
    struct LEOperator : public IConstFunc2  {
      LEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual double Evaluate() const {  return a->cast<double>() <= b->cast<double>();  }
    };

    struct AndOperator : public IConstFunc2  {
      AndOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b)  {}
      virtual double Evaluate() const {  return a->cast<bool>() && b->cast<bool>();  }
    };
    struct OrOperator : public IConstFunc2  {
      OrOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b)  {}
      virtual double Evaluate() const {  return a->cast<bool>() || b->cast<bool>();  }
    };
    struct NotOperator : public IConstFunc  {
      NotOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual double Evaluate() const {  return !arg->cast<bool>();  }
    };

    static IEvaluable* Create(const olxstr& name, TPtrList<IEvaluable>& args)  {
      if( args.Count() == 1)
        return Create(name, args[0]);
      else if( args.Count() == 2 )
        return Create(name, args[0], args[1]);
      return NULL;
    }
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
        if( name == "min" ) return new MinFunc(a, b);
        if( name == "max" ) return new MaxFunc(a, b);
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
  IEvaluable* ProcessConstFunc(IEvaluable* func, IEvaluable* left, IEvaluable* right=NULL)  {
    if( (dynamic_cast<BuiltInsFactory::IConstFunc*>(func) != NULL || 
       dynamic_cast<BuiltInsFactory::IConstFunc2*>(func) != NULL) )
    {
      if( left == NULL ||
          (EsdlInstanceOf(*left, ConstEvaluable) && 
          (right == NULL || EsdlInstanceOf(*right, ConstEvaluable))) )  
      {
        IEvaluable* ce = new ConstEvaluable(func->Evaluate());
        delete func;
        return ce;
      }
    }
    return func;
  }
  IEvaluable* EvaluatorFromEvator(ExpParser::exp_tree* root)  {
    TPtrList<IEvaluable> args;
    bool all_const = true;
    for( int i=0; i < root->evator->args.Count(); i++ )  {
      args.Add( CreateEvaluator(root->evator->args[i]) );
      if( args.Last() == NULL )  {
        for( int j=0; j < args.Count()-2; j++ )
          delete args[j];
        throw TInvalidArgumentException(__OlxSourceInfo, "could not find appropriate evluable");
      }
      if( !EsdlInstanceOf(*args.Last(), ConstEvaluable) )
        all_const = false;
    }
    IEvaluable *rv = BuiltInsFactory::Create(root->evator->name, args);
    // need to test if the function result cannot be changed
    if( all_const )  return ProcessConstFunc(rv, NULL);
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
      return ProcessConstFunc(rv, left, right);
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
//  struct XEvaluable  {
//    void* (*cast_operator)(const XEvaluable& );
//    olxdict<std::type_info const*, cast_operator, TPrimitivePtrComparator> cast_operators;   
//    void AddCastOperator(
//  };
public:
  IEvaluable* Build(const olxstr& exp)  {
    // c == 3 || c == -1
    ExpParser expp(exp);
    expp.expand();
    expp.root = sortLogical(expp.root);
    return CreateEvaluator(expp.root);
  }
};
//template <class BC>
//const SExpression::IEvaluable::operator_dict::Entry SExpression::TNumberEvaluator<BC>::cast_operators_table[] = {
//  SExpression::IEvaluable::operator_dict::Entry(&typeid(bool), &SExpression::TNumberEvaluator<BC>::bool_cast),
//  SExpression::IEvaluable::operator_dict::Entry(&typeid(olxstr), &SExpression::TNumberEvaluator<BC>::str_cast),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<char>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<unsigned char>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<short>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<unsigned short>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<unsigned int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<long int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<unsigned long int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<long long int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<unsigned long long int>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<float>(),
//  SExpression::TNumberEvaluator<BC>::create_operator_entry<double>()
//};
//template <class BC>
//olxdict<std::type_info const*, SExpression::IEvaluable::cast_operator, TPointerPtrComparator> 
//  SExpression::TNumberEvaluator<BC>::cast_operators( 
//    SExpression::TNumberEvaluator<BC>::cast_operators_table, 
//    sizeof(SExpression::TNumberEvaluator<BC>::cast_operators_table)/sizeof(SExpression::TNumberEvaluator<BC>::cast_operators_table[0]) 
//  );

#endif
