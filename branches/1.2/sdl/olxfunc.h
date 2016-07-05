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

  template <typename func_t>
  struct func0_ {
    func_t func;
    func0_(const func_t &f)
      : func(f)
    {}
    template <class item_t> void OnItem(item_t &i, size_t) const {
      (olx_ref::get(i).*func)();
    }
  };

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
  template <typename base_t, typename func_t>
  struct mfunc1_ {
    base_t &base;
    func_t func;
    mfunc1_(base_t &base, const func_t &f)
      : base(base), func(f)
    {}
    template <class item_t> void OnItem(item_t &i, size_t) const {
      (base.*func)(olx_ref::get(i));
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
  template <typename base_t, typename func_t, typename arg1_t>
  struct mfunc2_ {
    base_t &base;
    func_t func;
    arg1_t a;
    mfunc2_(base_t &base, const func_t &f, const arg1_t &a)
      : base(base), func(f), a(a)
    {}
    template <class item_t> void OnItem(item_t &i, size_t) const {
      (base.*func)(olx_ref::get(i), a);
    }
  };

  template <typename func_t>
  static func0_<func_t> make(const func_t &f) {
    return func0_<func_t>(f);
  }

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

  template <typename base_t, typename func_t>
  static mfunc1_<base_t, func_t> make(base_t *base, const func_t &f) {
    return mfunc1_<base_t, func_t>(*base, f);
  }

  template <typename base_t, typename func_t, typename arg1_t>
  static mfunc2_<base_t, func_t, arg1_t> make(base_t *base, const func_t &f,
    const arg1_t &a)
  {
    return mfunc2_<base_t, func_t, arg1_t>(*base, f, a);
  }
};


#endif
