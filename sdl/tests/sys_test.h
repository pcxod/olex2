/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/


namespace test {

class CriticalSectionTest {
  int i, j, k, l;
  bool use_cs;
  olx_critical_section cs;
  class TestTh : public AOlxThread  {
    CriticalSectionTest& inst;
  public:
    TestTh(CriticalSectionTest& _inst) : inst(_inst)  {  Detached = false;  }
    virtual int Run()  {
      for( int _i=0; _i < 100000; _i++ )  {
        if( inst.use_cs == true )
          inst.cs.enter();
        inst.i++; inst.j++; inst.k++; inst.l++;
        if( inst.use_cs == true )
          inst.cs.leave();
      }
      return 0;
    }
  };
public:
  CriticalSectionTest(bool _use_cs) : use_cs(_use_cs), i(0), j(0), k(0), l(0) {}
  void DoTest(OlxTests& t)  {
    t.description << __OlxSrcInfo << " using CS: " << use_cs;
    TestTh* ths[10];
    for( int _i=0; _i < 10; _i++ )  {
      ths[_i] = new TestTh(*this);
      ths[_i]->Start();
    }
    for( int _i=0; _i < 10; _i++ )  {
      ths[_i]->Join();
      delete ths[_i];
    }
    if( i != 1000000 || j != 1000000 || k != 1000000 || l != 1000000 )  {
      if( use_cs )
        throw TFunctionFailedException(__OlxSourceInfo, "critical section test has failed");
    }
    else if( !use_cs )  
      throw TFunctionFailedException(__OlxSourceInfo, "critical section test is ambiguous (it is possible)");
  }
};
//...................................................................................................
};  //namespace test
