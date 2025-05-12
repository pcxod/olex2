/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "eset.h"
#include "edict.h"

BeginEsdlNamespace();
/*
class IHashable {
public:
  virtual int32_t HashCode() = 0;
};
*/

// 2^32 = N^L * 2^[32 - log_2(N)*L]
template <class basket_factory_t,
  uint32_t N=64, uint32_t L=4>
class TEHashed {
  struct entry_array_base_t {
    void** data;
    entry_array_base_t() {
      this->data = new void*[N];
      memset(this->data, 0, N * sizeof(void*));
    }
    virtual ~entry_array_base_t() {
      olx_del_arr(this->data);
    }
  };

  struct entry_array_t : public entry_array_base_t {
    virtual ~entry_array_t() {
      for (size_t i = 0; i < N; i++) {
        if (this->data[i] != 0) {
          delete (entry_array_t*)this->data[i];
        }
      }
    }
  };

  struct basket_array_t : public entry_array_base_t {
    virtual ~basket_array_t() {
      for (size_t i = 0; i < N; i++) {
        if (this->data[i] != 0) {
          delete (basket_t*)this->data[i];
        }
      }
    }
  };

  basket_factory_t basket_factory;
  entry_array_t data;
  size_t basket_n;
#ifdef _DEBUG
  size_t entry_n, mem_b;
  void init() {
    entry_n = basket_n = mem_b = 0;
  }
#else
  void init() {
    basket_n = 0;
  }
#endif
public:

  typedef typename basket_factory_t::basket_t basket_t;
  TEHashed() {
    init();
  }

  TEHashed(const basket_factory_t &basket_factory)
    : basket_factory(basket_factory)
  {
    init();
  }
  

  virtual ~TEHashed() {
  }

  template<class T>
  basket_t *Find(const T &key) const {
    uint32_t hash = key.HashCode();
    const entry_array_base_t* en = &data;
    for (int i = 0; i < L; i++) {
      uint32_t f = hash / N;
      uint32_t r = hash - f * N;
      hash = f;
      en = ((const entry_array_base_t**)en->data)[r];
      if (en == 0) {
        return 0;
      }
    }
    uint32_t f = hash / N;
    uint32_t r = hash - f * N;
    return ((basket_t**)en->data)[r];
  }

  template<class T>
  const bool Contains(const T& key) const {
    const basket_t* b = Find(key);
    return b == 0 ? false : b->Contains(key);
  }

  template<class T>
  basket_t& GetBasket(const T& item) {
    uint32_t hash = item.HashCode();
    entry_array_base_t* en = &data;
    for (int i = 0; i < L; i++) {
      uint32_t f = hash / N;
      uint32_t r = hash - f * N;
      hash = f;
      entry_array_base_t* en_ = ((entry_array_base_t**)en->data)[r];
      if (en_ == 0) {
#ifdef _DEBUG
        entry_n++;
#endif
        if (i + 1 < L) {
          en_ = new entry_array_t();
        }
        else {
          en_ = new basket_array_t();
        }
        en->data[r] = en_;
#ifdef _DEBUG
        mem_b += N * sizeof(entry_array_base_t);
#endif
      }
      en = en_;
    }
    uint32_t f = hash / N;
    uint32_t r = hash - f * N;
    basket_t* b = ((basket_t**)en->data)[r];
    if (b == 0) {
      b = basket_factory.new_basket();
      en->data[r] = b;
      basket_n++;
#ifdef _DEBUG
      mem_b += sizeof(basket_t);
#endif
    }
    return *b;
  }

  template<class T>
  void Add(const T &item) {
    GetAddBasket(item).Add(item);
  }

  template<class T>
  bool Remove(const T& key) {
    basket_t* b = Find(key);
    if (b == 0) {
      return false;
    }
    return b->Remove(key);
  }

  class Iterator {
    size_t current[L+1] = { 0 };
    const entry_array_t &data;
  public:
    Iterator(const entry_array_t& data)
      : data(data)
    {
      current[L] = -1;
    }

    size_t inc_index(size_t idx) {
      while (true) {
        if (++current[idx] == N) {
          if (idx == 0) {
            return InvalidIndex;
          }
          current[idx] = 0;
          current[L] = -1;
          idx--;
        }
        else {
          return idx;
        }
      }
    }

    basket_t* get_current() {
      const entry_array_base_t* en = &data;
      for (size_t i = 0; i < L; i++) {
        const entry_array_base_t* en_ = ((const entry_array_base_t**)en->data)[current[i]];
        if (en_ == 0) {
          return 0;
        }
        en = en_;
      }
      return ((basket_t**)en->data)[current[L]];
    }

    basket_t* Next() {
      if (current[0] == N) {
        return 0;
      }
      while (true) {
        const entry_array_base_t* en = &data;
        bool restart = false;
        for (size_t i = 0; i < L; i++) {
          const entry_array_base_t* en_ = ((const entry_array_base_t**)en->data)[current[i]];
          while (en_ == 0) {
            size_t idx = inc_index(i);
            if (idx == InvalidIndex) {
              return 0;
            }
            if (idx != i) {
              restart = true;
              break;
            }
            en_ = ((const entry_array_base_t**)en->data)[current[i]];
          }
          if (restart) {
            break;
          }
          en = en_;
        }
        if (restart) {
          continue;
        }
        while (true) {
          size_t idx = inc_index(L);
          if (idx == InvalidIndex) {
            return 0;
          }
          if (idx != L) {
            break;
          }
          basket_t* b = ((basket_t**)en->data)[current[L]];
          if (b != 0) {
            return b;
          }
        }
      }
      return 0;
    }

    void Reset() {
      memset(&current[0], 0, (L + 1) * sizeof(current[0]));
      current[L] = -1;
    }

  };

  Iterator Iterate() const {
    return Iterator(this->data);
  }
};

template <typename item_t, class comparator_t>
struct SetFactory {
  typedef olxset<item_t, comparator_t> basket_t;
  comparator_t cmp;
  SetFactory() {}
  SetFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
    //return new basket_t(cmp, olx_reserve(1));
  }
};

template <typename key_t, typename item_t, class comparator_t>
struct MapFactory {
  typedef olxdict<key_t, item_t, comparator_t> basket_t;
  comparator_t cmp;
  MapFactory() {}
  MapFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
    //return new basket_t(cmp, olx_reserve(1));
  }
};

template <typename item_t, class comparator_t,
  uint32_t N = 64, uint32_t L = 4>
class TEHashSet : protected TEHashed<SetFactory<item_t, comparator_t>, N, L> {
  typedef TEHashed<SetFactory<item_t, comparator_t>, N, L> parent_t;
  SetFactory<item_t, comparator_t> factory;
public:
  typedef typename parent_t::Iterator iterator_t;
  typedef typename parent_t::basket_t basket_t;

  TEHashSet() {}

  TEHashSet(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  bool Add(const T& item) {
    return parent_t::GetBasket(item).Add(item);
  }

  template <class coll_t>
  TEHashSet& AddAll(const coll_t& l) {
    for (size_t i = 0; i < l.Count(); i++) {
      Add(l[i]);
    }
    return *this;
  }

  template <class coll_t, class Accessor>
  TEHashSet& AddAll(const coll_t& l, const Accessor& accessor) {
    for (size_t i = 0; i < l.Count(); i++) {
      Add(accessor(l[i]));
    }
    return *this;
  }

  template <typename T>
  bool Contains(const T& item) const {
    return parent_t::Contains(item);
  }

  template<class T>
  bool Remove(const T& key) {
    return parent_t::Remove(key);
  }

  iterator_t Iterate() const {
    return parent_t::Iterate();
  }

};

template <typename key_t, typename item_t, class comparator_t,
  uint32_t N = 64, uint32_t L = 4>
class TEHashMap : protected TEHashed<MapFactory<key_t, item_t, comparator_t>, N, L> {
  typedef TEHashed<MapFactory<key_t, item_t, comparator_t>, N, L> parent_t;
  MapFactory<key_t, item_t, comparator_t> factory;
public:
  typedef typename parent_t::Iterator iterator_t;
  typedef typename parent_t::basket_t basket_t;

  TEHashMap() {}

  TEHashMap(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  item_t& Add(const T& key, const item_t &value) {
    return parent_t::GetBasket(key).Add(key, value);
  }

  template <typename T>
  bool Contains(const T& key) const {
    return parent_t::Contains(key);
  }

  template<class T>
  bool Remove(const T& key) {
    return parent_t::Remove(key);
  }

  iterator_t Iterate() const {
    return parent_t::Iterate();
  }
};
EndEsdlNamespace();