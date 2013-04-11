/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_test_suit_H
#define __olx_sdl_test_suit_H
#include "typelist.h"
BeginEsdlNamespace()

/* a very simple test functions wrapper. */
struct OlxTests  {
private:
  struct ITestFunc  {
    virtual void run(OlxTests&) = 0;
    virtual ~ITestFunc(){}
  };
  struct TestSFunc : public ITestFunc {
    void (*f)(OlxTests&);
    TestSFunc( void (*_f)(OlxTests&) ) : f(_f)  {}
    virtual void run(OlxTests& t) {  (*f)(t);  }
  };
  template <typename base> struct TestMFunc : public ITestFunc  {
    base* instance;
    void (base::*f)(OlxTests&);
    TestMFunc(base* _instance, void (base::*_f)(OlxTests&) )
      : instance(_instance), f(_f)
    {}
    ~TestMFunc()  {  delete instance;  }
    virtual void run(OlxTests& t)  { (instance->*f)(t);  }
  };
public:  // test functions can use this to set printed information
  olxstr description;
  OlxTests()  {}
  TTypeList<ITestFunc> tests;
  OlxTests& Add(void (*f)(OlxTests&))  {
    tests.Add( new TestSFunc(f));
    return *this;
  }
  template <typename base>
  OlxTests& Add(base* instance, void (base::*f)(OlxTests&) )  {
    tests.Add(new TestMFunc<base>(instance, f));
    return *this;
  }
  void run();
};

EndEsdlNamespace()
#endif
