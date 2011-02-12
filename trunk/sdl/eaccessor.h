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

#endif
