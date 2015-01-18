/******************************************************************************
* Copyright (c) 2004-2015 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_func_H
#define __olx_sdl_func_H

struct olx_func {
  template <typename func_t, typename arg1_t>
  struct func1_ {
    func_t func;
    arg1_t a;
    func1_(const func_t &f, const arg1_t &a)
      : func(f), a(a)
    {}
    template <class item_t> void OnItem(item_t &i, size_t) const {
      (olx_ref::get(i).*func)(a);
    }
  };
  template <typename func_t, typename arg1_t, typename arg2_t>
  struct func2_ {
    func_t func;
    arg1_t a;
    arg2_t b;
    func2_(const func_t &f, const arg1_t &a, const arg2_t &b)
      : func(f), a(a), b(b)
    {}
    template <class item_t> void OnItem(item_t &i, size_t) const {
      (olx_ref::get(i).*func)(a, b);
    }
  };
  template <typename func_t, typename arg1_t>
  static func1_<func_t, arg1_t> make(const func_t &f, const arg1_t &a) {
    return func1_<func_t, arg1_t>(f, a);
  }
  template <typename func_t, typename arg1_t, typename arg2_t>
  static func2_<func_t, arg1_t, arg2_t> make(const func_t &f, const arg1_t &a,
    const arg2_t &b)
  {
    return func2_<func_t, arg1_t, arg2_t>(f, a, b);
  }
};


#endif
