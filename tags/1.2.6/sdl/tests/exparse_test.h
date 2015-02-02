/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../exparse/expbuilder.h"
#include "../testsuit.h"

namespace test { namespace exparse {

using namespace esdl::exparse;
void Test(OlxTests& t)  {
  t.description = __FUNC__;
  EvaluableFactory evf;
  context cx;
  context::init_global(cx);
  evf.types.Add(&typeid(olxstr), new StringValue);
  evf.classes.Add(&typeid(olxstr), &StringValue::info);
  evf.types.Add(&typeid(ListValue::list_t), new ListValue);
  evf.classes.Add(&typeid(ListValue::list_t), &ListValue::info);
  StringValue::init_library();
  ListValue::init_library();

  exp_builder _exp(evf, cx);
  olxstr tmp = "a = 'ab c, de\\';()'";
  olxstr val = parser_util::unquote(tmp.SubStringFrom(4));
  IEvaluable* iv = _exp.build(tmp);
  if (*iv->cast<olxstr>().val != val)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (cx.vars.Count() != 1 || cx.vars['a']->undress() != iv ||
      *cx.vars['a']->cast<olxstr>().val!= val)
  {
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  }
  iv = _exp.build("b = 'ab c'");
  if (cx.vars.Count() != 2 || cx.vars['b']->undress() != iv) {
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  }
  _exp.scope.add_var("c", new StringValue("abcdef"));
  if (cx.vars.Count() != 3 || *cx.vars['c']->cast<olxstr>().val!= "abcdef") {
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  }
  iv = _exp.build("a.sub(0,4).sub(1,3).len()");
  if (*iv->cast<size_t>().val != 3)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if( iv->ref_cnt() == 0 )  delete iv;
  iv = _exp.build("x = a.sub(0,4).len() + b.len()");
  if (*_exp.scope.find_var('x')->cast<size_t>().val != 8)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("c = a.sub(0,3) != b.sub(0,3)", false);
  if (*iv->cast<bool>().val != false)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("c = a.sub(0,3) == a.sub(0,3)", false);
  if (*iv->cast<bool>().val != true)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if( iv->ref_cnt() == 0 )  delete iv;
  iv = _exp.build("c = a.sub(0,3) != b.sub(0,3)");
  if (*iv->cast<bool>().val != false)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("c = !(a.sub(0,3) == b.sub(0,3))");
  if (*iv->cast<bool>().val != false)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("c = !(a.sub(0,4) == b.sub(0,3))");
  if (*iv->cast<bool>().val != true)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  //iv = _exp.build("c = c + 100");
  //iv = _exp.build("c = 1.2 + 1.1 - .05");
  iv = _exp.build("a.len() + 1.2 + 1.1 - abs(-.05)*cos(PI/2)");
  if (olx_abs(*iv->cast<double>().val-15.3) > 1e-6)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (iv->ref_cnt() == 0) delete iv;
  iv = _exp.build("a='AaBc'.charAt(2)");
  if (*iv->cast<olxch>().val != L'B')
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("a='AaBc'[1].toUpper()");
  if (*iv->cast<olxstr>().val != L'A')
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("a='100'.atoi()");
  if (*iv->cast<int>().val != 100)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("a=['aBc',a,b, 1.2]");
  ListValue* l = dynamic_cast<ListValue*> (iv);
  if (l == NULL || l->val.Count() != 4)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("a.add(['ab','ac'])");
  if (l == NULL || l->val.Count() != 5)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (iv->ref_cnt() == 0) delete iv;
  iv = _exp.build("a=a[4][1][1].toUpper()");
  if (*iv->cast<olxstr>().val != L'C')
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  iv = _exp.build("cos pi*30/180");
  if (*iv->cast<double>().val != cos(M_PI*30/180))
    throw TFunctionFailedException(__OlxSourceInfo, "assert");

  if (iv->ref_cnt() == 0) delete iv;
}
//...................................................................................................
void ExparseTests(OlxTests& t)  {
  t.Add(&test::exparse::Test);
}
}};  //namespace test::exparse
