#ifndef __olx_sdl_eaccessor_H
#define __olx_sdl_eaccessor_H

struct DirectAccessor  {
  template <typename Item> static inline const Item& Access(const Item& item)  {
    return item;
  }
  template <typename Item> static inline Item& Access(Item& item)  {
    return item;
  }
};
template <typename CastType> struct CCastAccessor  {
  template <typename Item> static inline const CastType& Access(const Item& item)  {
    return (const CastType&)item;
  }
  template <typename Item> static inline CastType& Access(Item& item)  {
    return (CastType&)item;
  }
  template <typename Item> static inline const CastType* Access(const Item* item)  {
    return (const CastType*)item;
  }
  template <typename Item> static inline CastType* Access(Item* item)  {
    return (CastType*)item;
  }
};
template <typename CastType> struct StaticCastAccessor  {
  template <typename Item> static inline const CastType& Access(const Item& item)  {
    return static_cast<const CastType&>(item);
  }
  template <typename Item> static inline CastType& Access(Item& item)  {
    return static_cast<CastType&>(item);
  }
  template <typename Item> static inline const CastType* Access(const Item* item)  {
    return static_cast<const CastType*>(item);
  }
  template <typename Item> static inline CastType* Access(Item* item)  {
    return static_cast<CastType*>(item);
  }
};
template <typename CastType> struct DynamicCastAccessor  {
  template <typename Item> static inline const CastType& Access(const Item& item)  {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item> static inline CastType& Access(Item& item)  {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item> static inline const CastType* Access(const Item* item)  {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item> static inline CastType* Access(Item* item)  {
    return dynamic_cast<CastType*>(item);
  }
};

struct FunctionAccessor {
  template <typename rv_t, typename base_t> struct ConstFunctionAccessor_  {
    rv_t (base_t::*func)() const;
    ConstFunctionAccessor_(rv_t (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t Access(const item_t& it) const {
      return (olx_get_ref(it).*func)();    
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessor_  {
    rv_t (base_t::*func)();
    FunctionAccessor_(rv_t (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t Access(item_t& it) const {
      return (olx_get_ref(it).*func)();    
    }
  };

  template <typename rv_t, typename base_t> static
  ConstFunctionAccessor_<rv_t,base_t> Make(rv_t (base_t::*func)() const)  {
    return ConstFunctionAccessor_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessor_<rv_t,base_t> Make(rv_t (base_t::*func)())  {
    return FunctionAccessor_<rv_t,base_t>(func);
  }
};

#endif
