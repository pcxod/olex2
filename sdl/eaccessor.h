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

struct DirectAccessor  {
  template <typename Item>
  static inline const Item& Access(const Item& item)  {
    return item;
  }
  template <typename Item> static inline Item& Access(Item& item)  {
    return item;
  }

  template <typename Item>
  inline const Item& operator ()(const Item& item) const {
    return item;
  }
  template <typename Item>
  inline Item& operator ()(Item& item) const {
    return item;
  }
};

template <typename CastType> struct CCastAccessor  {
  template <typename Item>
  static inline const CastType& Access(const Item& item)  {
    return (const CastType&)item;
  }
  template <typename Item>
  static inline CastType& Access(Item& item)  {
    return (CastType&)item;
  }
  template <typename Item>
  static inline const CastType* Access(const Item* item)  {
    return (const CastType*)item;
  }
  template <typename Item>
  static inline CastType* Access(Item* item)  {
    return (CastType*)item;
  }

  template <typename Item>
  inline const CastType& operator ()(const Item& item) const {
    return (const CastType&)item;
  }
  template <typename Item>
  inline CastType& operator ()(Item& item) const {
    return (CastType&)item;
  }
  template <typename Item>
  inline const CastType* operator ()(const Item* item) const {
    return (const CastType*)item;
  }
  template <typename Item>
  inline CastType* operator ()(Item* item) const {
    return (CastType*)item;
  }
};
template <typename CastType> struct StaticCastAccessor  {
  template <typename Item>
  static inline const CastType& Access(const Item& item)  {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  static inline CastType& Access(Item& item)  {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  static inline const CastType* Access(const Item* item)  {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  static inline CastType* Access(Item* item)  {
    return static_cast<CastType*>(item);
  }

  template <typename Item>
  inline const CastType& operator ()(const Item& item) const {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  inline CastType& operator ()(Item& item) const {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  inline const CastType* operator ()(const Item* item) const {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  inline CastType* operator ()(Item* item) const {
    return static_cast<CastType*>(item);
  }
};
template <typename CastType> struct DynamicCastAccessor  {
  template <typename Item>
  static inline const CastType& Access(const Item& item)  {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  static inline CastType& Access(Item& item)  {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  static inline const CastType* Access(const Item* item)  {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  static inline CastType* Access(Item* item)  {
    return dynamic_cast<CastType*>(item);
  }

  template <typename Item>
  inline const CastType& operator ()(const Item& item) const {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  inline CastType& operator ()(Item& item) const {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  inline const CastType* operator ()(const Item* item) const {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  inline CastType* operator ()(Item* item) const {
    return dynamic_cast<CastType*>(item);
  }
};

struct FunctionAccessor {
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)() const;
    ConstFunctionAccessorT_(rv_t (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t operator ()(const item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)() const;
    ConstFunctionAccessorR_(rv_t & (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t & operator ()(const item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)() const;
    ConstFunctionAccessorCR_(const rv_t & (base_t::*_func)() const)
      : func(_func)  {}
    template <typename item_t>
    const rv_t & operator ()(const item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };

  template <typename rv_t, typename base_t> struct FunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)();
    FunctionAccessorT_(rv_t (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t operator()(item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)();
    FunctionAccessorR_(rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t & operator()(item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)();
    FunctionAccessorCR_(const rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> const rv_t & operator()(item_t& it) const {
      return (olx_get_ref(it).*func)();
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
      return (*func)(olx_get_ref(it));
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
      return (*func)(olx_get_ref(it));
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
      return (*func)(olx_get_ref(it));
    }
  };

  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorT_<rv_t,base_t> MakeConst(
    rv_t (base_t::*func)() const)
  {
    return ConstFunctionAccessor_<rv_t,base_t>(func);
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

template <typename item_t, class data_list_t>
struct ConstIndexAccessor  {
  typedef item_t return_type;
  const data_list_t &data;
  ConstIndexAccessor(const data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  inline const item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

template <typename item_t, class data_list_t>
struct IndexAccessor  {
  typedef item_t return_type;
  data_list_t &data;
  IndexAccessor(data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  inline item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

#endif
