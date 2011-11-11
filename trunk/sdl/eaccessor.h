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
  template <typename rv_t, typename base_t> struct ConstFunctionAccessor_  {
    rv_t (base_t::*func)() const;
    ConstFunctionAccessor_(rv_t (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t operator ()(const item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessor_  {
    rv_t (base_t::*func)();
    FunctionAccessor_(rv_t (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t operator()(item_t& it) const {
      return (olx_get_ref(it).*func)();
    }
  };
  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessor_  {
    rv_t (*func)(const item_t &);
    StaticFunctionAccessor_(rv_t (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    rv_t operator()(item_t_t& it) const {
      return (*func)(olx_get_ref(it));
    }
  };

  template <typename rv_t, typename base_t> static
  ConstFunctionAccessor_<rv_t,base_t> MakeConst(rv_t (base_t::*func)() const)  {
    return ConstFunctionAccessor_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessor_<rv_t,base_t> Make(rv_t (base_t::*func)())  {
    return FunctionAccessor_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename item_t> static
  StaticFunctionAccessor_<rv_t,item_t>
  MakeStatic(rv_t (*func)(const item_t &))  {
    return StaticFunctionAccessor_<rv_t,item_t>(func);
  }
};

template <typename item_t, class data_list_t>
struct ConstIndexAccessor  {
  const data_list_t &data;
  ConstIndexAccessor(const data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  inline const item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

template <typename item_t, class data_list_t>
struct IndexAccessor  {
  data_list_t &data;
  IndexAccessor(data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  inline item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

#endif
