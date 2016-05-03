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

  struct test_struct : public exparse::IEvaluable, public IOlxObject {
    int test_val;
    test_struct() {
      test_val = 0;
    }
    test_struct(int v) {
      test_val = v;
    }
    static test_struct &create0() {
      return *(new test_struct());
    }
    static test_struct &create1(int v) {
      return *(new test_struct(v));
    }
    void vtest_0() {}
    void vtest_1(bool v) {}
    void vtest_2(bool v1, bool v2) {}

    bool test_0() { return true; }
    bool test_1(bool v) { return !v; }
    bool test_2(bool v1, bool v2) { return v1 && v2; }
    int get_val() const { return test_val; }
    void set_val(int v)  { test_val = v; }

    virtual IEvaluable* _evaluate() const {
      return create_proxy_();
    }

    virtual bool is_final() const {
      return true;
    }

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

    ClassInfo<test_struct, test_struct> &cr = *(new ClassInfo<test_struct, test_struct>());
    cr.globals.add("__init__", &test_struct::create0);
    cr.globals.add("__init__", &test_struct::create1);
    cr.functions.add("vtest", &test_struct::vtest_0);
    cr.functions.add("vtest", &test_struct::vtest_1);
    cr.functions.add("vtest", &test_struct::vtest_2);
    cr.functions.add("test", &test_struct::test_0);
    cr.functions.add("test", &test_struct::test_1);
    cr.functions.add("test", &test_struct::test_2);
    cr.functions.add("get_val", &test_struct::get_val);
    cr.functions.add("set_val", &test_struct::set_val);

    evf.types.Add(&typeid(test_struct &), new VarProxy(0));
    cx.classes.Add("test_struct", &cr);
    evf.classes.Add(&typeid(test_struct &), &cr);
    TPtrList<IEvaluable> args;
    test_struct ts;
    EvaluableFactory factory;
    IEvaluableHolder i = cr.functions.call(factory, ts, "test", args);
    if (i.cast<bool>() != true) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    args.Add(_exp.build("a = 1"));
    args.Add(_exp.build("b = 0"));
    i = cr.functions.call(factory, ts, "test", args);
    if (i.cast<bool>() != false) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    args[0] = cx.find_var('a');
    args[1] = _exp.build("b = 1");
    i = cr.functions.call(factory, ts, "test", args);
    if (i.cast<bool>() != true) {
      throw TFunctionFailedException(__OlxSourceInfo, "value");
    }
    i = _exp.build("t = test_struct()");
    i = _exp.build("t = test_struct(1)");
    _exp.build("t.set_val(5)");
    i = _exp.build("t.get_val() == 4");
    i = _exp.build("t.get_val() == 5");
    i = _exp.build("t.test(true,true)");
    i = _exp.build("t.test(true,false)");
    i = _exp.build("t.test(false,false)");
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
