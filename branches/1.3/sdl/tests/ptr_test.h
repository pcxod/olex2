/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxptr.h"
#include "olxvptr.h"

namespace test {
  class Perishable : public APerishable {
  };


  void PtrTest1(OlxTests& t) {
    t.description = __FUNC__;
    Perishable p, p1;
    olx_perishable_ptr<Perishable> ptr(new Perishable);
    delete &ptr();
    if (ptr.is_valid()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    ptr = new Perishable();
    delete ptr.release();
    if (ptr.is_valid()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    {
      Perishable p1_;
      ptr = &p1_;
    }
    if (ptr.is_valid()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    {
      ptr = &p;
      ptr = 0;
    }
    if (ptr.is_valid()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    olx_perishable_ptr<Perishable> ptr_(new Perishable);
    ptr_ = ptr;
    if (ptr_.is_valid()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
  }
  //...................................................................................................
  struct VPtrTest_O: public IOlxObject {
    int v;
    VPtrTest_O(int v) {
      this->v = v;
    }
  };
  struct VPtr : public olx_virtual_ptr<VPtrTest_O> {
    VPtrTest_O *inst;
    VPtr(VPtrTest_O *p) {
      inst = p;
    }
    ~VPtr() {
      if (inst != 0) {
        delete inst;
      }
    }
    void set_inst(VPtrTest_O *p) {
      if (inst != 0) {
        delete inst;
        inst = p;
      }
    }
    virtual IOlxObject *get_ptr() const {
      return inst;
    }

  };
  void VptrTest(OlxTests &t) {
    t.description = __FUNC__;
    VPtr *ptr = new VPtr(new VPtrTest_O(0));
    olx_vptr<VPtrTest_O> x(ptr);
    if (x().v != 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    // chnging the underlying object!
    ptr->set_inst(new VPtrTest_O(1));
    if (x().v != 1) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
  }
 //...................................................................................................
  void PtrTests(OlxTests &t) {
    t.Add(&PtrTest1)
      .Add(&VptrTest);
  }
};  //namespace test
