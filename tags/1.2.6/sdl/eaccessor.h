/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_eaccessor_H
#define __olx_sdl_eaccessor_H

struct DummyAccessor  {
  template <typename item_t>
  const item_t& operator ()(const item_t& item) const { return item; }
  template <typename item_t>
  item_t& operator ()(item_t& item) const { return item; }
};

template <typename item_t>
struct TDirectAccessor  {
  typedef item_t return_type;

  const item_t& operator ()(const item_t& item) const { return item; }
  item_t& operator ()(item_t& item) const { return item; }
};

template <class list_t>
TDirectAccessor<typename list_t::list_item_type>
  ListAccessor(const list_t &t)
{
  return TDirectAccessor<typename list_t::list_item_type>();
}
struct DereferenceAccessor  {
  template <typename item_t>
  const item_t& operator ()(const item_t* item) const { return *item; }
  template <typename item_t>
  item_t& operator ()(item_t* item) const { return *item; }
};

template <typename item_t>
struct TDereferenceAccessor  {
  typedef item_t return_type;

  const item_t& operator ()(const item_t* item) const { return *item; }
  item_t& operator ()(item_t* item) const { return *item; }
};

template <typename CastType> struct CCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return (const CastType&)item;
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return (CastType&)item;
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return (const CastType*)item;
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return (CastType*)item;
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return (const CastType&)item;
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return (CastType&)item;
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return (const CastType*)item;
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return (CastType*)item;
  }
};
template <typename CastType> struct StaticCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return static_cast<CastType*>(item);
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return static_cast<CastType*>(item);
  }
};
template <typename CastType> struct DynamicCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return dynamic_cast<CastType*>(item);
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return dynamic_cast<CastType*>(item);
  }
};

struct FunctionAccessor {
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)() const;
    ConstFunctionAccessorT_(rv_t (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)() const;
    ConstFunctionAccessorR_(rv_t & (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t & operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)() const;
    ConstFunctionAccessorCR_(const rv_t & (base_t::*_func)() const)
      : func(_func)  {}
    template <typename item_t>
    const rv_t & operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };

  template <typename rv_t, typename base_t> struct FunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)();
    FunctionAccessorT_(rv_t (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)();
    FunctionAccessorR_(rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t & operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)();
    FunctionAccessorCR_(const rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> const rv_t & operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };

  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (*func)(const item_t &);
    StaticFunctionAccessorT_(rv_t (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    rv_t operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };
  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(*func)(const item_t &);
    StaticFunctionAccessorR_(rv_t & (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    rv_t & operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };
  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(*func)(const item_t &);
    StaticFunctionAccessorCR_(const rv_t & (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    const rv_t & operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };

  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorT_<rv_t,base_t> MakeConst(
    rv_t (base_t::*func)() const)
  {
    return ConstFunctionAccessorT_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorR_<rv_t,base_t> MakeConst(
    rv_t & (base_t::*func)() const)
  {
    return ConstFunctionAccessorR_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorCR_<rv_t,base_t> MakeConst(
    const rv_t &(base_t::*func)() const)
  {
    return ConstFunctionAccessorCR_<rv_t,base_t>(func);
  }

  template <typename rv_t, typename base_t> static
  FunctionAccessorT_<rv_t,base_t> Make(rv_t (base_t::*func)())  {
    return FunctionAccessorT_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessorR_<rv_t,base_t> Make(rv_t & (base_t::*func)())  {
    return FunctionAccessorR_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessorCR_<rv_t,base_t> Make(const rv_t & (base_t::*func)())  {
    return FunctionAccessorCR_<rv_t,base_t>(func);
  }

  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorT_<rv_t,item_t>
  MakeStatic(rv_t (*func)(const item_t &))  {
    return StaticFunctionAccessorT_<rv_t,item_t>(func);
  }
  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorR_<rv_t,item_t>
  MakeStatic(rv_t & (*func)(const item_t &))  {
    return StaticFunctionAccessorR_<rv_t,item_t>(func);
  }
  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorCR_<rv_t,item_t>
  MakeStatic(const rv_t & (*func)(const item_t &))  {
    return StaticFunctionAccessorCR_<rv_t,item_t>(func);
  }
};

struct ComplexAccessor {
  template <class acc1_t, class acc2_t>
  class TComplexAccessor {
    acc1_t acc1;
    acc2_t acc2;
  public:
    TComplexAccessor(const acc1_t &a1, const acc2_t &a2)
      : acc1(a1), acc2(a2)
    {}
    typedef typename acc2_t::return_type return_type;
    template <typename i_t>
    const return_type& operator ()(const i_t& item) const {
      return acc2(acc1(item));
    }
    template <typename i_t>
    return_type& operator ()(i_t& item) const {
      return acc2(acc1(item));
    }
  };

  template <class acc1_t, class acc2_t>
  class TComplexAccessorP {
    acc1_t acc1;
    acc2_t acc2;
  public:
    TComplexAccessorP(const acc1_t &a1, const acc2_t &a2)
      : acc1(a1), acc2(a2)
    {}
    typedef typename acc2_t::return_type return_type;
    template <typename i_t>
    return_type operator ()(const i_t& item) const {
      return acc2(acc1(item));
    }
    template <typename i_t>
    return_type operator ()(i_t& item) const {
      return acc2(acc1(item));
    }
  };
  // for accessors returning references like const int &f()
  template <class acc1_t, class acc2_t>
  static TComplexAccessor<acc1_t, acc2_t>
    Make(const acc1_t &a1, const acc2_t &a2) {
      return TComplexAccessor<acc1_t, acc2_t>(a1, a2);
    }
  // for accessors returning instances like int f()
  template <class acc1_t, class acc2_t>
  static TComplexAccessorP<acc1_t, acc2_t>
    MakeP(const acc1_t &a1, const acc2_t &a2) {
      return TComplexAccessorP<acc1_t, acc2_t>(a1, a2);
    }
};


struct IndexAccessor {
  template <typename data_list_t>
  struct ConstIndexAccessor_  {
    typedef typename data_list_t::list_item_type return_type;
    const data_list_t &data;
    ConstIndexAccessor_(const data_list_t &data_) : data(data_) {}
    template <typename IndexT>
    const return_type& operator ()(const IndexT& idx) const {
      return data[idx];
    }
    template <typename IndexT>
    const return_type& operator [](const IndexT& idx) const {
      return data[idx];
    }
  };
  template <class data_list_t>
  struct IndexAccessor_  {
    typedef typename data_list_t::list_item_type return_type;
    data_list_t &data;
    IndexAccessor_(data_list_t &data_) : data(data_) {}
    template <typename IndexT>
    return_type& operator ()(const IndexT& idx) const {
      return data[idx];
    }
    template <typename IndexT>
    return_type& operator [](const IndexT& idx) const {
      return data[idx];
    }
  };

  template <class list_t>
  static IndexAccessor_<list_t> Make(list_t &l) {
    return IndexAccessor_<list_t>(l);
  }
  template <class list_t>
  static ConstIndexAccessor_<list_t> MakeConst(const list_t &l) {
    return ConstIndexAccessor_<list_t>(l);
  }
};

#endif
