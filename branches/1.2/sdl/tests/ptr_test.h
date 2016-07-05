/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxptr.h"

namespace test {
  class Perishable : public APerishable {
  };


  void PtrTest1(OlxTests& t) {
    t.description = __FUNC__;
    Perishable p, p1;
    olx_perishable_ptr<Perishable> ptr(new Perishable);
    delete &ptr();
    ptr = new Perishable();
    delete ptr.release();
    if (!ptr.is_valid()) {
      Perishable p1;
      ptr = &p;
      ptr = &p1;
      ptr = &p1;
      ptr = 0;
    }
    {
      olx_perishable_ptr<Perishable> ptr_(new Perishable);
      ptr_ = ptr;
      ptr.is_valid();
    }
  }
  //...................................................................................................
  void PtrTests(OlxTests &t) {
    t.Add(&PtrTest1);
  }
};  //namespace test
