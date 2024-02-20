/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_alg_H
#define __olx_sdl_alg_H
#include "eaccessor.h"

struct olx_alg {
protected:
  // logical NOT operator for an analyser
  template <class Analyser> struct not_ {
    const Analyser& analyser;
    not_(const Analyser& _analyser)
      : analyser(_analyser)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return !analyser.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return !analyser.OnItem(o, i);
    }
  };
  // logical AND operator for two analysers
  template <class AnalyserA, class AnalyserB> struct and_ {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    and_(const AnalyserA& _analyserA, const AnalyserB& _analyserB)
      : analyserA(_analyserA), analyserB(_analyserB)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) && analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) && analyserB.OnItem(o, i);
    }
  };
  // logical OR operator for two analysers
  template <class AnalyserA, class AnalyserB> struct or_ {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    or_(const AnalyserA& _analyserA, const AnalyserB& _analyserB)
      : analyserA(_analyserA), analyserB(_analyserB)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) || analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) || analyserB.OnItem(o, i);
    }
  };

  template <class Accessor> struct chsig_ {
    const Accessor &accessor;
    chsig_(const Accessor &accessor_) : accessor(accessor_) {}
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o, size_t) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type operator () (const Item& o) const {
      return -accessor(o);
    }
  };

  struct op_eq {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t == to;
    }
  };
  struct op_neq {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t != to;
    }
  };
  struct op_lt {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t < to;
    }
  };
  struct op_le {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t <= to;
    }
  };
  struct op_gt {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t > to;
    }
  };
  struct op_ge {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t >= to;
    }
  };

  template <typename to_t, class Accessor, class op_t> struct op_ {
    to_t to;
    const Accessor &accessor;
    op_(const to_t &t, const Accessor &accessor_) : to(t), accessor(accessor_)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return op_t::op(to, accessor(o));
    }
    template <class Item> bool OnItem(const Item& o, size_t) const {
      return op_t::op(to, accessor(o));
    }
  };
public:
  /* creates a new not logical operator */
  template <class Analyser>
  static not_<Analyser> olx_not(const Analyser& a) {
    return not_<Analyser>(a);
  }
  /* creates a new and logical operator */
  template <class AnalyserA, class AnalyserB>
  static and_<AnalyserA, AnalyserB> olx_and(
    const AnalyserA& a, const AnalyserB& b)
  {
    return and_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new or logical operator */
  template <class AnalyserA, class AnalyserB>
  static or_<AnalyserA, AnalyserB> olx_or(
    const AnalyserA& a, const AnalyserB& b) {
    return or_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new chsig arithmetic functor/accessor */
  template <class Accessor>
  static chsig_<Accessor> olx_chsig(const Accessor& a) {
    return chsig_<Accessor>(a);
  }
  /* creates a new equality checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_eq> olx_eq(const to_t &to) {
    return op_<to_t, DummyAccessor, op_eq>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_eq> olx_eq(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_eq>(to, a);
  }
  /* creates a new non-equality checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_neq> olx_neq(const to_t &to) {
    return op_<to_t, DummyAccessor, op_neq>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_neq> olx_neq(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_neq>(to, a);
  }
  /* creates a LT checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_lt> olx_lt(const to_t &to) {
    return op_<to_t, DummyAccessor, op_lt>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_lt> olx_lt(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_lt>(to, a);
  }
  /* creates a LE checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_le> olx_le(const to_t &to) {
    return op_<to_t, DummyAccessor, op_le>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_le> olx_le(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_le>(to, a);
  }
  /* creates a GT checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_gt> olx_gt(const to_t &to) {
    return op_<to_t, DummyAccessor, op_gt>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_gt> olx_gt(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_gt>(to, a);
  }
  /* creates a GE checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_ge> olx_ge(const to_t &to) {
    return op_<to_t, DummyAccessor, op_ge>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_ge> olx_ge(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_ge>(to, a);
  }
};

// can turn a bool function into an anlyaser for filtering/packing
struct FunctionAccessorAnalyser {
  template <class acc_t>
  struct AccessorAnalyser_ {
    const acc_t& acc;
    AccessorAnalyser_(const acc_t& acc) : acc(acc) {}
    template <class item_t>
    bool OnItem(const item_t& item, size_t idx = InvalidIndex) const {
      return acc(item);
    }
  };

  template <class acc_t>
  static AccessorAnalyser_<acc_t> Make(const acc_t& acc) {
    return AccessorAnalyser_<acc_t>(acc);
  }
};


/* swaps two objects using a temporary variable (copy constructor must be
available for complex types) */
template <typename obj> inline void olx_swap(obj& o1, obj& o2) {
  obj tmp = o1;
  o1 = o2;
  o2 = tmp;
}
// returns 10^val, cannot put it to emath due to dependencies...
template <typename FT> FT olx_pow10(size_t val)  {
  if (val == 0) {
    return 1;
  }
  FT rv = 10;
  while (--val > 0) {
    rv *= 10;
  }
  return rv;
}
/* comparison function (useful for the size_t on Win64, where size_t=uint64_t
*  and int is int32_t) */
template <typename T1, typename T2> inline
int olx_cmp(T1 a, T2 b) { return a < b ? -1 : (a > b ? 1 : 0); }

template <typename T, typename T1> bool olx_is(const T1 &v) {
  return typeid(T) == typeid(olx_ref::get(v));
};

template<typename from_t, typename to_t>
void olx_copy(const from_t &f, to_t &t, size_t cnt, size_t f_off=0, size_t t_off=0) {
  for (size_t i = 0; i < cnt; i++) {
    t[t_off+i] = f[f_off+i];
  }
}
template<typename from_t, typename to_t, class Accessor>
void olx_copy(const from_t& f, to_t& t, const Accessor &a,
  size_t cnt, size_t f_off, size_t t_off)
{
  for (size_t i = 0; i < cnt; i++) {
    t[t_off + i] = a(f[f_off + i]);
  }
}

#endif
