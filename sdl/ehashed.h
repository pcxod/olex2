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
#include "ebtree.h"
#include "type_splitter.h"

BeginEsdlNamespace();
/*
class IHashable {
public:
  virtual int32_t HashCode() = 0;
};
*/

template <typename float_t>
int normalise_float(float_t v, int max) {
  if (v == 0) {
    return 0;
  }
  int ex = 0;
  if (v < 0) {
    while (v > -1) {
      v *= 10;
      ex++;
    }
    while (v < -1) {
      v /= 10;
      ex--;
    }
  }
  else {
    while (v < 1) {
      v *= 10;
      ex++;
    }
    while (v > 1) {
      v /= 10;
      ex--;
    }
  }
  return (int)(v*max + ex);
}

// 2^32 = N^L * 2^[32 - log_2(N)*L]
template <class basket_factory_t,
  uint32_t N=64, uint32_t L=2>
class TEHashed :
  public AIterable<typename basket_factory_t::value_t>
{
  struct hash_getter_t {
    mutable int hash_code;
    template <typename type_t>
    void p_functor(const type_t& v) const {
      if (olx_is_float<type_t>::is) {
        hash_code = (int)normalise_float(v, 0xFFFFFFFF/2);
      }
      else {
        hash_code = (int)v;
      }
    }
    template <class type_t>
    void c_functor(const type_t& v) const {
      hash_code = v.HashCode();
    }

    void c_functor(const olxstr& v) const {
      hash_code = v.HashCode<true>();
    }
    template <typename key_t, typename val_t, class cmp_t>
    void c_functor(const DictEntry<key_t,val_t,cmp_t>& v) const {
      primitive_type_splitter::make(*this).call(v.key);
    }

    template <typename key_t, typename val_t>
    void c_functor(const TreeMapEntry<key_t, val_t>& v) const {
      primitive_type_splitter::make(*this).call(v.key);
    }
  } hash_getter;
public:
  typedef typename basket_factory_t::basket_t basket_t;
  typedef typename basket_factory_t::value_t value_t;
private:
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
  size_t basket_n, count;
   
#ifdef _DEBUG
  size_t entry_n;
  void init() {
    count = entry_n = basket_n = 0;
  }
#else
  void init() {
    basket_n = 0;
  }
#endif
protected:
  template<class T>
  basket_t& GetBasket(const T& item) {
    primitive_type_splitter::make(hash_getter).call(item);
    uint32_t hash = hash_getter.hash_code;
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
    }
    return *b;
  }

  template<class T>
  bool Add(const T& item) {
    if (GetBasket(item).Add(item)) {
      count++;
      return true;
    }
    return false;
  }

public:

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
    primitive_type_splitter::make(hash_getter).call(key);
    uint32_t hash = hash_getter.hash_code;
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
  bool Remove(const T& key) {
    basket_t* b = Find(key);
    if (b == 0) {
      return false;
    }
    size_t cnt = b->Count();
    if (b->Remove(key)) {
      count -= cnt - b->Count();
      return true;
    }
    return false;
  }

  size_t Count() const {
    return count;
  }

  size_t size() {
    return count;
  }

  class BasketIterator {
    olx_array_ptr<size_t> current;
    const entry_array_t &data;
  public:
    BasketIterator(const entry_array_t& data)
      : current(L+1),
      data(data)
    {
      memset(&current, 0, sizeof(size_t) * (L + 1));
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

    basket_t* get_current() const {
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

  struct FullIterator : public IIterator<value_t> {
    basket_factory_t basket_factory;
    mutable BasketIterator bi;
    typedef typename basket_factory_t::value_iterator_t ValueIterator;
    mutable olx_object_ptr<ValueIterator> vi;
    FullIterator(const BasketIterator& basketIterator, const basket_factory_t& basket_factory)
      : bi(basketIterator),
      basket_factory(basket_factory)
    {}

    bool HasNext() const {
      if (!vi.ok()) {
        if (bi.Next() == 0) {
          return false;
        }
        vi = basket_factory.new_iterator(bi.get_current());
      }
      if (vi->HasNext()) {
        return true;
      }
      while (!vi->HasNext()) {
        if (bi.Next() == 0) {
          return false;
        }
        vi = basket_factory.new_iterator(bi.get_current());
      }
      return true;
    }

    const value_t& Next() {
      return vi->Next();
    }

  };

  BasketIterator IterateBaskets() const {
    return BasketIterator(this->data);
  }

  FullIterator Iterate() const {
    return FullIterator(BasketIterator(this->data), basket_factory);
  }

  IIterator<value_t>* iterate() const {
    return new FullIterator(BasketIterator(this->data), basket_factory);
  }

#ifdef _DEBUG
  size_t mem_usage() const {
    size_t mem_b = sizeof(basket_t) * basket_n +
      N * sizeof(entry_array_base_t) * entry_n;
    return mem_b;
  }
#endif
  // collects stats N of items in baskets vs basket N
  olx_pdict<size_t, size_t>::const_dict_type get_stats() {
    BasketIterator itr = IterateBaskets();
    basket_t* b;
    olx_pdict<size_t, size_t> rv;
    while ((b = itr.Next()) != 0) {
      rv(b->Count(), 0)++;
    }
    return rv;
  }
};

struct MDValueAccessor {
  template <typename item_t, class comparator_t>
  static const item_t& get(const olxset<item_t, comparator_t>& s, size_t idx) {
    return s[idx];
  }
  template <typename key_t, typename item_t, class comparator_t>
  static const item_t& get(const olxdict<key_t, item_t, comparator_t>& d, size_t idx) {
    return d.GetValue(idx);
  }
};

template <typename item_t, class comparator_t>
struct SetFactory {
  typedef olxset<item_t, comparator_t> basket_t;
  typedef item_t value_t;
  typedef IndexableIterator<basket_t, MDValueAccessor, value_t> value_iterator_t;
  comparator_t cmp;
  SetFactory() {}
  SetFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
    //return new basket_t(cmp, olx_reserve(1));
  }
  value_iterator_t* new_iterator(basket_t* b) const {
    return new value_iterator_t(b);
  }
};

template <typename key_t, typename item_t, class comparator_t>
struct MapFactory {
  typedef olxdict<key_t, item_t, comparator_t> basket_t;
  typedef item_t value_t;
  typedef IndexableIterator<basket_t, MDValueAccessor, value_t> value_iterator_t;
  comparator_t cmp;
  MapFactory() {}
  MapFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
    //return new basket_t(cmp, olx_reserve(1));
  }
  value_iterator_t* new_iterator(basket_t* b) const {
    return new value_iterator_t(b);
  }
};

template <typename key_t, class comparator_t>
struct BTSetFactory {
  typedef TreeSetEntry<key_t> value_tt;
  //typedef AVLTreeEntry<value_tt> entry_t;
  //typedef AVLTree<entry_t, comparator_t> basket_t;
  typedef RBTreeEntry<value_tt> entry_t;
  typedef RBTree<entry_t, comparator_t> basket_t;
  typedef key_t value_t;
  typedef typename basket_t::ValueIterator value_iterator_t;
  comparator_t cmp;
  BTSetFactory() {}
  BTSetFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
  }

  value_iterator_t* new_iterator(basket_t* b) const {
    return new value_iterator_t(*b);
  }
};

template <typename key_t, typename item_t, class comparator_t>
struct BTMapFactory {
  typedef TreeMapEntry<key_t, item_t> value_tt;
  //typedef AVLTreeEntry<value_tt> entry_t;
  //typedef AVLTree<entry_t, comparator_t> basket_t;
  typedef RBTreeEntry<value_tt> entry_t;
  typedef RBTree<entry_t, comparator_t> basket_t;
  typedef item_t value_t;
  typedef typename basket_t::ValueIterator value_iterator_t;
  comparator_t cmp;
  BTMapFactory() {}
  BTMapFactory(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
  }
  value_iterator_t* new_iterator(basket_t* b) const {
    return new value_iterator_t(*b);
  }
};

template <typename key_t, typename item_t, class comparator_t>
struct BTMapFactoryEx {
  typedef TreeMapEntry<key_t, item_t> value_tt;
  typedef RBTreeEntryEx<value_tt> entry_t;
  typedef RBTree<entry_t, comparator_t> basket_t;
  typedef item_t value_t;
  typedef typename basket_t::ValueIterator value_iterator_t;
  comparator_t cmp;
  BTMapFactoryEx() {}
  BTMapFactoryEx(const comparator_t& cmp)
    : cmp(cmp)
  {}

  basket_t* new_basket() const {
    return new basket_t(cmp);
  }
  value_iterator_t* new_iterator(basket_t* b) const {
    return new value_iterator_t(*b);
  }
};

template <typename item_t, class comparator_t,
  uint32_t N = 16, uint32_t L = 4>
class TEHashSet : public TEHashed<SetFactory<item_t, comparator_t>, N, L> {
  typedef SetFactory<item_t, comparator_t> factory_t;
  typedef TEHashed<factory_t, N, L> parent_t;
  factory_t factory;
public:
  typedef typename parent_t::basket_t basket_t;

  TEHashSet() {}

  TEHashSet(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  bool Add(const T& item) {
    return parent_t::Add(item);
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

};

template <typename key_t, typename item_t, class comparator_t,
  uint32_t N = 16, uint32_t L = 4>
class TEHashMap : public TEHashed<MapFactory<key_t, item_t, comparator_t>, N, L> {
  typedef MapFactory<key_t, item_t, comparator_t> factory_t;
  typedef TEHashed<factory_t, N, L> parent_t;
  factory_t factory;
public:
  typedef typename parent_t::basket_t basket_t;

  TEHashMap() {}

  TEHashMap(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  bool Add(const T& key, const item_t& value) {
    return parent_t::Add(typename basket_t::list_item_type(key, value));
  }
};

template <typename key_t, class comparator_t,
  uint32_t N = 64, uint32_t L = 2>
class TEHashTreeSet : public TEHashed<BTSetFactory<key_t, comparator_t>, N, L> {
  typedef BTSetFactory<key_t, comparator_t> factory_t;
  typedef TEHashed<factory_t, N, L> parent_t;
  factory_t factory;
public:
  typedef typename parent_t::basket_t basket_t;

  TEHashTreeSet() {}

  TEHashTreeSet(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T> void Add(const T& key) {
    parent_t::Add(key);
  }
};

template <typename key_t, typename item_t, class comparator_t,
  uint32_t N = 64, uint32_t L = 2>
class TEHashTreeMap : public TEHashed<BTMapFactory<key_t, item_t, comparator_t>, N, L> {
  typedef BTMapFactory<key_t, item_t, comparator_t> factory_t;
  typedef TEHashed<factory_t, N, L> parent_t;
  factory_t factory;
public:
  typedef typename parent_t::basket_t basket_t;

  TEHashTreeMap() {}

  TEHashTreeMap(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  void Add(const T& key, const item_t& value) {
    parent_t::Add(factory_t::value_tt(key, value));
  }
};

template <typename key_t, typename item_t, class comparator_t,
  uint32_t N = 64, uint32_t L = 2>
class TEHashTreeMapEx : public TEHashed<BTMapFactoryEx<key_t, item_t, comparator_t>, N, L> {
  typedef BTMapFactoryEx<key_t, item_t, comparator_t> factory_t;
  typedef TEHashed<factory_t, N, L> parent_t;
  factory_t factory;
public:
  typedef typename parent_t::basket_t basket_t;
  typedef typename factory_t::entry_t entry_t;

  TEHashTreeMapEx() {}

  TEHashTreeMapEx(const comparator_t& cmp)
    : factory(cmp)
  {}

  template <typename T>
  void Add(const T& key, const item_t& value) {
    parent_t::Add(typename factory_t::value_tt(key, value));
  }
  template<class T>
  entry_t* Find(const T& key) const {
    basket_t *b = parent_t::Find(key);
    if (b == 0) {
      return 0;
    }
    return b->Find(key);
  }
};
EndEsdlNamespace();