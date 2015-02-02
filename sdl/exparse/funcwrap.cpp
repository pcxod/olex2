/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "funcwrap.h"

using namespace exparse;
//IEvaluable* IBasicFunction::find_method(const olxstr& name, const EvaluableFactory& f, const TPtrList<IEvaluable>& args) {
//  size_t i = f.classes.IndexOf(&get_RV_type());
//  if( i == InvalidIndex )  return NULL;
//  return f.classes.GetValue(i)->find(name, args.Count());
//}

void vtest_0() {}
void vtest_1(bool)  {}
void vtest_2(bool, bool)  {}

bool test_0()  {  return true;  }
bool test_1(bool v)  {  return !v;  }
bool test_2(bool v1, bool v2)  {  return v1 && v2;  }

struct test_struct : public IEObject  {
  void vtest_0() {}
  void vtest_1(bool v)  {}
  void vtest_2(bool v1, bool v2)  {}

  bool test_0()  {  return true;  }
  bool test_1(bool v)  {  return !v;  }
  bool test_2(bool v1, bool v2)  {  return v1 && v2;  }
};

void LibraryRegistry::CompileTest()  {
  LibraryRegistry lr;
  lr.add("vtest", &vtest_0);
  lr.add("vtest", &vtest_1);
  lr.add("vtest", &vtest_2);
  lr.add("test", &test_0);
  lr.add("test", &test_1);
  lr.add("test", &test_2);

  ClassRegistry<test_struct> cr;
  cr.add("vtest", &test_struct::vtest_0);
  cr.add("vtest", &test_struct::vtest_1);
  cr.add("vtest", &test_struct::vtest_2);
  cr.add("test", &test_struct::test_0);
  cr.add("test", &test_struct::test_1);
  cr.add("test", &test_struct::test_2);
  TPtrList<IEvaluable> args;
  test_struct ts;
  EvaluableFactory factory;
  cr.call(factory, ts, "test", args);
}
