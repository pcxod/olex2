#include "xapp.h"
#include "outstream.h"
#include "hkl.h"
#include "evaln.h"
#include "tptrlist.h"
#include <iostream>
using namespace std;

int sortHKl(const TReflection* r1, const TReflection* r2)  {
  int d = r1->GetL() - r2->GetL();
  if( d != 0 )  return d;
  d = r1->GetK() - r2->GetK();
  if( d != 0 )  return d;
  return r1->GetH() - r2->GetH();
}

typedef TStrPObjList<olxstr,TSOperation*> FuncList;

struct ExeScope  {
  TStrList funcs;
  TDoubleList values;
  TStrPObjList<olxstr,TSOperation*> vars;
};

struct ExeCmd  {
  virtual ~ExeCmd()  {}
  virtual int Execute() = 0;
};
struct ExeBlock : public ExeCmd {
  TPtrList<ExeCmd> Content;
  virtual ~ExeBlock()  {
    for( int i=0; i < Content.Count(); i++ )
      delete Content[i];
  }
  virtual int Execute() {
    for( int i=0; i < Content.Count(); i++ )  {
      Content[i]->Execute();
    }
    return 0;
  }
};

struct Evaluable  {
  virtual ~Evaluable() {}
  virtual double Evaluate() = 0;
};

struct ConstEvaluable : public Evaluable  {
  double value;
  ConstEvaluable(const double& val) : value(val)  {}
  virtual double Evaluate() {  return value;  }
};
struct ComplexEvaluable : public Evaluable  {
  TSOperation* operation;
  ComplexEvaluable(TSOperation* op) : operation(op) {}
  ~ComplexEvaluable()  {  delete operation;  }
  virtual double Evaluate() {  return operation->Evaluate();  }
};
struct EvaluableFactory {
  static Evaluable* Create(ExeScope& exe_scope, const olxstr& str)  {
    if( str.IsNumber() )
      return new ConstEvaluable(str.ToDouble());
    else  {
      TSOperation* so = new TSOperation(NULL, &exe_scope.vars, &exe_scope.funcs, &exe_scope.values);
      if( so->LoadFromExpression(str) )  {
        delete so;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to parse: ") << str);
      }
      return new ComplexEvaluable(so);
    }
  }
};

struct ConditionEvaluator  {
  Evaluable* op1, *op2;
  ConditionEvaluator(Evaluable* o1, Evaluable* o2) : op1(o1), op2(o2)  {}
  virtual ~ConditionEvaluator()  {
    delete op1;
    delete op2;
  }
  virtual bool Evaluate() = 0;
};

struct EqualityEvaluator : public ConditionEvaluator  {
  EqualityEvaluator(Evaluable* o1, Evaluable* o2) : ConditionEvaluator(o1, o2) {}
  virtual bool Evaluate()  {  return op1->Evaluate() == op2->Evaluate();  }
};

struct NonEqualityEvaluator : public ConditionEvaluator  {
  NonEqualityEvaluator(Evaluable* o1, Evaluable* o2) : ConditionEvaluator(o1, o2) {}
  virtual bool Evaluate()  {  return op1->Evaluate() != op2->Evaluate();  }
};

struct ConditionEvaluatorFactory {
  static ConditionEvaluator* Create(ExeScope& exe_scope, const olxstr& opr, const olxstr& op1, const olxstr& op2)  {
    if( opr.Equals("==") )
      return new EqualityEvaluator(EvaluableFactory::Create(exe_scope, op1), 
        EvaluableFactory::Create(exe_scope, op2));
    if( opr.Equals("!=") )
      return new NonEqualityEvaluator(EvaluableFactory::Create(exe_scope, op1), 
        EvaluableFactory::Create(exe_scope, op2));
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate condition evaluator");
  }
};

struct IfBlock : public ExeBlock  {
  ConditionEvaluator* condition;
  IfBlock(ConditionEvaluator* _condition) : condition(_condition)  {} 
  virtual ~IfBlock()  {
    delete condition;
  }
  virtual int Execute()  {
    if( !condition->Evaluate() )  return 0;
    return ExeBlock::Execute();
  }
};
struct OutCmd: public ExeCmd  {
  TRefList& out;
  Evaluable* h, *k, *l, *i, *s, *batch;
  OutCmd(TRefList& _out, Evaluable* _h, Evaluable* _k, Evaluable* _l,
    Evaluable* _i, Evaluable* _s, Evaluable* _batch) : 
    out(_out),
    h(_h),k(_k), l(_l),
    i(_i), s(_s),
    batch(_batch) {}
  ~OutCmd()  {
    delete h;
    delete k;
    delete l;
    delete i;
    delete s;
    delete batch;
  }
  virtual int Execute()  {
    out.Add( 
      new TReflection(
        (int)h->Evaluate(), (int)k->Evaluate(), (int)l->Evaluate(),
        i->Evaluate(), s->Evaluate(), (int)batch->Evaluate() )
    ).SetTag(1);
    return 0;
  }
};

int main(int argc, char* argv[])  {
  try  {
    TXApp XApp( TBasicApp::GuessBaseDir(argv[0]));
    XApp.GetLog().AddStream( new TOutStream(), true );
    if( argc < 3 )  {
      TBasicApp::GetLog() << "Please provide an input and output hkl file names\n";
      return 0;
    }

    TRefList new_refs_script;

    ExeScope exe_scope;
    ExeBlock root;
    IfBlock* ib = new IfBlock( ConditionEvaluatorFactory::Create(exe_scope, "==", "(h+l)%3", "0") );
    ib->Content.Add( 
      new OutCmd(new_refs_script,
        EvaluableFactory::Create(exe_scope, "2*(h+l)/3-h"),
        EvaluableFactory::Create(exe_scope, "-k"),
        EvaluableFactory::Create(exe_scope, "4*(h+l)/3-l"),
        EvaluableFactory::Create(exe_scope, "i"),
        EvaluableFactory::Create(exe_scope, "s"),
        EvaluableFactory::Create(exe_scope, "-2")
      ) 
    );
    root.Content.Add(ib);
    root.Content.Add( 
      new OutCmd(new_refs_script,
        EvaluableFactory::Create(exe_scope, "h"),
        EvaluableFactory::Create(exe_scope, "k"),
        EvaluableFactory::Create(exe_scope, "l"),
        EvaluableFactory::Create(exe_scope, "i"),
        EvaluableFactory::Create(exe_scope, "s"),
        EvaluableFactory::Create(exe_scope, "1")
      ) 
    );
    exe_scope.values.SetCount( exe_scope.vars.Count() );
    int var_indeces[5] = {
      exe_scope.vars.IndexOf('H'),
      exe_scope.vars.IndexOf('K'),
      exe_scope.vars.IndexOf('L'),
      exe_scope.vars.IndexOf('I'),
      exe_scope.vars.IndexOf('S')
    };

    THklFile hkl;
    hkl.LoadFromFile(argv[1]);
    TRefList new_refs;
    TRefPList refs;
    new_refs.SetCapacity( hkl.RefCount()*3);
    refs.SetCapacity( hkl.RefCount());
    for( int i=0; i < hkl.RefCount(); i++ )
      refs.Add(&hkl[i]);
    refs.QuickSorter.SortSF(refs, sortHKl);
    for( int i=0; i < refs.Count(); i++ )  {
      const TReflection& r = *refs[i];
      if( r.GetTag() < 0 )  break;
      if( var_indeces[0] != -1 )  exe_scope.values[var_indeces[0]] = r.GetH();
      if( var_indeces[1] != -1 )  exe_scope.values[var_indeces[1]] = r.GetK();
      if( var_indeces[2] != -1 )  exe_scope.values[var_indeces[2]] = r.GetL();
      if( var_indeces[3] != -1 )  exe_scope.values[var_indeces[3]] = r.GetI();
      if( var_indeces[4] != -1 )  exe_scope.values[var_indeces[4]] = r.GetS();
      root.Execute();
      int sum = r.GetH() + r.GetL();
      if( (sum%3) == 0 )  {
        int n = sum/3;
        new_refs.Add( 
          new TReflection(2*n - r.GetH(), -r.GetK(), 4*n - r.GetL(), r.GetI(), r.GetS(), -2) ).SetTag(1);
      }
      new_refs.Add( 
        new TReflection(r.GetH(), r.GetK(), r.GetL(), r.GetI(), r.GetS(), 1) ).SetTag(1);
    }
    hkl.SaveToFile(argv[2], new_refs);
    hkl.SaveToFile("e:/1.hkl", new_refs_script);
    TBasicApp::GetLog() << "new file has been created\n";
  }
  catch( TExceptionBase& exc )  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }
  printf("\n...");
  std::cin.get();
  return 0;
}