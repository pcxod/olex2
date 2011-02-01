#ifndef __olx_test_suit_H
#define __olx_test_suit_H
/* a very simple test functions wrapper. */
#include "typelist.h"

BeginEsdlNamespace()

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
    TestMFunc(base* _instance, void (base::*_f)(OlxTests&) ) : instance(_instance), f(_f) {}
    ~TestMFunc()  {  delete instance;  }
    virtual void run(OlxTests& t)  { (instance->*f)(t);  }
  };
public:  // test functions can use this to set printed information
  olxstr description;
  OlxTests();
  TTypeList<ITestFunc> tests;
  void Add(void (*f)(OlxTests&))  {  tests.Add( new TestSFunc(f));  }
  template <typename base> void Add(base* instance, void (base::*f)(OlxTests&) )  {
    tests.Add( new TestMFunc<base>(instance, f) );
  }
  void run();
};

EndEsdlNamespace()
#endif
