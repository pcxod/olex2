/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_exparse_builins_H
#define __olx_sdl_exparse_builins_H
#include "evaluable.h"
#include "../emath.h"
BeginEsdlNamespace()

namespace exparse  {

  struct BuiltInsFactory {
    struct IConstFunc : public ANumberEvaluator  {
      IEvaluable* arg;
      IConstFunc(IEvaluable* _arg) : arg(_arg) {
        arg->inc_ref();
      }
      ~IConstFunc()  {  
        if( arg->dec_ref() == 0 )  delete arg;  
      }
    };
    struct IConstFunc2 : public ANumberEvaluator  {
      IEvaluable* a, *b;
      IConstFunc2(IEvaluable* _a, IEvaluable* _b) : a(_a), b(_b) {
        a->inc_ref();
        b->inc_ref();
      }
      ~IConstFunc2()  {  
        if( a->dec_ref() == 0 )  delete a;  
        if( b->dec_ref() == 0 )  delete b;  
      }
    };
    struct AbsFunc : public IConstFunc  {
      AbsFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(olx_abs<double>(arg->cast<double>()));
      }
    };
    struct CosFunc : public IConstFunc  {
      CosFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(cos(arg->cast<double>()));
      }
    };
    struct SinFunc : public IConstFunc  {
      SinFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(sin(arg->cast<double>()));
      }
    };
    struct TanFunc : public IConstFunc  {
      TanFunc(IEvaluable* arg) : IConstFunc(arg)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(tan(arg->cast<double>()));
      }
    };
    struct PowFunc : public IConstFunc2  {
      PowFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(pow(a->cast<double>(), b->cast<double>()));
      }
    };
    struct MinFunc : public IConstFunc2  {
      MinFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(olx_min(a->cast<double>(), b->cast<double>()));
      }
    };
    struct MaxFunc : public IConstFunc2  {
      MaxFunc(IEvaluable* a, IEvaluable *b) : IConstFunc2(a, b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(olx_max(a->cast<double>(), b->cast<double>()));
      }
    };

    struct AddOperator : public IConstFunc2  {
      AddOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(a->cast<double>() + b->cast<double>());
      }
    };
    struct SubOperator : public IConstFunc2  {
      SubOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(a->cast<double>() - b->cast<double>());
      }
    };
    struct DivOperator : public IConstFunc2  {
      DivOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(a->cast<double>() / b->cast<double>());
      }
    };
    struct MulOperator : public IConstFunc2  {
      MulOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(a->cast<double>() * b->cast<double>());
      }
    };
    struct RemOperator : public IConstFunc2  {
      RemOperator(IEvaluable* a, IEvaluable *b) : IConstFunc2(a,b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new IntValue(a->cast<int>() % b->cast<int>());
      }
    };
    struct ChsOperator : public IConstFunc  {
      ChsOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(-arg->cast<double>());
      }
    };
    struct PlusOperator : public IConstFunc  {
      PlusOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual IEvaluable* _evaluate() const  {
        return new DoubleValue(arg->cast<double>());
      }
    };

    struct EOperator : public IConstFunc2  {
      EOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() == b->cast<double>());
      }
    };
    struct NEOperator : public IConstFunc2  {
      NEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() != b->cast<double>());
      }
    };
    struct GOperator : public IConstFunc2  {
      GOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() > b->cast<double>());
      }
    };
    struct GEOperator : public IConstFunc2  {
      GEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() >= b->cast<double>());
      }
    };
    struct LOperator : public IConstFunc2  {
      LOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() < b->cast<double>());
      }
    };
    struct LEOperator : public IConstFunc2  {
      LEOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b) {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<double>() <= b->cast<double>());
      }
    };

    struct AndOperator : public IConstFunc2  {
      AndOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<bool>() && b->cast<bool>());
      }
    };
    struct OrOperator : public IConstFunc2  {
      OrOperator(IEvaluable* a, IEvaluable* b) : IConstFunc2(a, b)  {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(a->cast<bool>() || b->cast<bool>());
      }
    };
    struct NotOperator : public IConstFunc  {
      NotOperator(IEvaluable* a) : IConstFunc(a)  {}
      virtual IEvaluable* _evaluate() const  {
        return new BoolValue(!arg->cast<bool>());
      }
    };

    static IEvaluable* create(const olxstr& name,
      TPtrList<IEvaluable>& args)
    {
      if( args.Count() == 1)
        return create(name, args[0]);
      else if( args.Count() == 2 )
        return create(name, args[0], args[1]);
      return NULL;
    }
    static IEvaluable* create(const olxstr& name, IEvaluable* a,
      IEvaluable* b=NULL)
    {
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
    static bool is_logical(const olxstr& name) {
      return name == "||" || name == "&&" || name == '!';
    }
    static bool is_cmp(const olxstr& name) {  
      return name == "==" || name == "!=" || name == '>' || name == '<' ||
        name == ">=" || name == "<=";
    }
    static bool is_arithmetic(const olxstr& name) {  
      return name == '+' || name == '-' || name == '*' || name == '/' ||
        name == '%';
    }
    static bool has_arithmetic_priority(const olxstr& name) {
      return name == '*' || name == '/';
    }
    static bool is_assignment(const olxstr& name) {
      return name == '=';
    }
  };
};  // end exparse namespace

EndEsdlNamespace()
#endif
