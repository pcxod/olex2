/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "olxptr.h"

BeginEsdlNamespace()

// a simple scalar state manager
template <typename T>
struct StateManager {
  T& value;
  T start, end;
  StateManager(T& value, const T& start_value, const T& end_value)
    : value(value), start(start_value), end(end_value)
  {
    value = start;
  }
  ~StateManager() {
    value = end;
  }
};

/* a simple way to call a function when this object instance gets out of scope.
* Simply for making it easier to read the code
*/
struct olx_finally {
  struct IFinally {
    virtual void operator ()() = 0;
    virtual ~IFinally() {}
  };

  template <typename rv_t>
  struct StaticFunctionFinally : public IFinally {
    rv_t (*func)();
    StaticFunctionFinally(rv_t(*func)())
      : func(func)
    {}
    virtual void operator ()() {
      (*func)();
    }
  };
  template <typename rv_t, typename base_t>
  struct FunctionFinally : public IFinally {
    base_t& base;
    rv_t (base_t::* func)();
    FunctionFinally(base_t &base, rv_t(base_t::* func)())
      : base(base), func(func)
    {}
    virtual void operator ()() {
      (base.*func)();
    }
  };

  template <typename rv_t, typename base_t>
  struct FunctionFinallyConst : public IFinally {
    const base_t& base;
    rv_t (base_t::* func)() const;
    FunctionFinallyConst(const base_t& base, rv_t(base_t::* func)() const)
      : base(base), func(func)
    {}
    virtual void operator ()() {
      (base.*func)();
    }
  };

  olx_object_ptr<IFinally> code;
  olx_finally() {}
  olx_finally(IFinally* fc)
    : code(fc)
  {}

  ~olx_finally() {
    (*code)();
  }

  template <typename rv_t>
  static olx_finally make(rv_t(*func)()) {
    return olx_finally(new StaticFunctionFinally<rv_t>(func));
  }

  template<typename rv_t, typename base_t>
  static olx_finally make(base_t &base, rv_t (base_t::*func)()) {
    return olx_finally(new FunctionFinally<rv_t, base_t>(base, func));
  }

  template<typename rv_t, typename base_t>
  static olx_finally make_const(const base_t& base, rv_t(base_t::* func)() const) {
    return olx_finally(new FunctionFinallyConst<rv_t, base_t>(base, func));
  }
};
EndEsdlNamespace()
