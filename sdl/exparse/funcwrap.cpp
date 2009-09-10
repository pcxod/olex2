#include "funcwrap.h"

void vtest_0() {}
void vtest_1(bool v)  {}
void vtest_2(bool v1, bool v2)  {}

bool test_0()  {  return true;  }
bool test_1(bool v)  {  return !v;  }
bool test_2(bool v1, bool v2)  {  return v1 && v2;  }

struct test_struct  {
  void vtest_0() {}
  void vtest_1(bool v)  {}
  void vtest_2(bool v1, bool v2)  {}

  bool test_0()  {  return true;  }
  bool test_1(bool v)  {  return !v;  }
  bool test_2(bool v1, bool v2)  {  return v1 && v2;  }
};

void exparse::LibraryRegistry::CompileTest()  {
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
  cr.call(ts, "test", args);
}

