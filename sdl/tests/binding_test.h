/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exparse/expbuilder.h"
#include "exparse/funcwrap.h"

namespace test {

  void BindingTest1(OlxTests& t) {
    t.description = __FUNC__;
    using namespace esdl::exparse;
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
    IEvaluableHolder iv = _exp.build("a = 'ab c, de\\';()'");
    iv = _exp.build("b = 'ab c'");
    iv = _exp.build("a.sub(0,4).sub(1,3).len()");
    if (!iv.value().is_final()) {
      throw TFunctionFailedException(__OlxSourceInfo, "final evaluable is not recognised");
    }
    if (iv.cast<int>() != 3) {
      throw TFunctionFailedException(__OlxSourceInfo, "function value");
    }
    // try wrong syntax
    bool failed = true;
    try {
      iv = _exp.build("x = a.sub (0,4).len() + b.len()");
      failed = false;
    }
    catch (const TExceptionBase &e) {
    }
    if (!failed) {
      throw TFunctionFailedException(__OlxSourceInfo, "syntax analysis");
    }
    iv = _exp.build("x = a.sub(0,4).len() + b.len()");
    if (iv.cast<int>() != 8) {
      throw TFunctionFailedException(__OlxSourceInfo, "function value");
    }
    iv = _exp.build("c = a.sub(0,3) == b.sub(0,3)", false);
    if (!cx.find_var('c')->cast<bool>()) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("c = a.sub(0,3) != b.sub(0,3)");
    if (cx.find_var('c')->cast<bool>()) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("c = !(a.sub(0,3) == b.sub(0,3))");
    if (cx.find_var('c')->cast<bool>()) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("c = !(a.sub(0,4) == b.sub(0,3))");
    if (!cx.find_var('c')->cast<bool>()) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("c = b.sub(0,3) + 'dfg'");
    iv = _exp.build("c = c + 100");
    if (*iv.cast<olxstr>().val != "ab dfg100") {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("c = 1.2 + 1.1 - .05");
    if (olx_abs(iv.cast<double>() - 2.25) > 1e-6) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("a.len() + 1.2 + 1.1 - abs(-.05)*cos(PI/2)");
    if (olx_abs(iv.cast<double>() - 15.3) > 1e-6) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("a='AaBc'.charAt(2)");
    if (iv.cast<olxch>() != 'B') {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("a='AaBc'[1].toUpper()");
    if (*iv.cast<olxstr>().val != 'A') {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("a='100'.atoi()");
    if (iv.cast<int>() != 100) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("a=['aBc',a,b, 1.2]");
    {
      TPtrList<IEvaluable> &l = *iv.cast<TPtrList<IEvaluable> >().val;
      if (l.Count() != 4) {
        throw TFunctionFailedException(__OlxSourceInfo, "list size");
      }
      if (olx_abs(l[3]->cast<double>()-1.2) > 1e-6 ) {
        throw TFunctionFailedException(__OlxSourceInfo, "list value");
      }
    }
    iv = _exp.build("a.add(['ab','ac'])");
    if (cx.find_var('a')->cast<TPtrList<IEvaluable> >().val->Count() != 5) {
      throw TFunctionFailedException(__OlxSourceInfo, "list size");
    }
    iv = _exp.build("a=a[4][1][1].toUpper()");
    if (*iv.cast<olxstr>().val != 'C') {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }

    iv = _exp.build("cos pi*30/180");
    if (olx_abs(iv.cast<double>() - cos(M_PI * 30 / 180))) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    iv = _exp.build("b = 35");
    iv = _exp.build("cos pi*b/180");
    if (olx_abs(iv.cast<double>() - cos(M_PI * 35 / 180))) {
      throw TFunctionFailedException(__OlxSourceInfo, "var value");
    }
    //iv = _exp.build("if(a){ a = a.sub(0,3); }else{ a = a.sub(0,4); }");
  }
  //...................................................................................................
  //...................................................................................................
  //...................................................................................................
  void vtest_0() {}
  void vtest_1(bool) {}
  void vtest_2(bool, bool) {}

  bool test_0() { return true; }
  bool test_1(bool v) { return !v; }
  bool test_2(bool v1, bool v2) { return v1 && v2; }

  struct test_struct : public IOlxObject {
    void vtest_0() {}
    void vtest_1(bool v) {}
    void vtest_2(bool v1, bool v2) {}

    bool test_0() { return true; }
    bool test_1(bool v) { return !v; }
    bool test_2(bool v1, bool v2) { return v1 && v2; }
  };

  void BindingTest2(OlxTests& t) {
    t.description = __FUNC__;
    using namespace esdl::exparse;
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

    cx.functions.add("vtest", &vtest_0);
    cx.functions.add("vtest", &vtest_1);
    cx.functions.add("vtest", &vtest_2);
    cx.functions.add("test", &test_0);
    cx.functions.add("test", &test_1);
    cx.functions.add("test", &test_2);

    ClassRegistry<test_struct> &cr = *(new ClassRegistry<test_struct>());
    cr.add("vtest", &test_struct::vtest_0);
    cr.add("vtest", &test_struct::vtest_1);
    cr.add("vtest", &test_struct::vtest_2);
    cr.add("test", &test_struct::test_0);
    cr.add("test", &test_struct::test_1);
    cr.add("test", &test_struct::test_2);
    cx.classes.Add("tst", &cr);
    TPtrList<IEvaluable> args;
    test_struct ts;
    EvaluableFactory factory;
    IEvaluableHolder i = cr.call(factory, ts, "test", args);
    if (i.cast<bool>() != true) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    args.Add(_exp.build("a = 1"));
    args.Add(_exp.build("b = 0"));
    i = cr.call(factory, ts, "test", args);
    if (i.cast<bool>() != false) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    args[0] = cx.find_var('a');
    args[1] = _exp.build("b = 1");
    i = cr.call(factory, ts, "test", args);
    if (i.cast<bool>() != true) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    //i = _exp.build("");
    return;
  }
  //...................................................................................................
  //...................................................................................................
  //...................................................................................................
  void BindingTests(OlxTests &t) {
    t.Add(&BindingTest1).
      Add(&BindingTest2);
  }
};  //namespace test
