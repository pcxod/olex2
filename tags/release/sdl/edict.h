#ifndef __olx_dictH
#define __olx_dictH
#include "sortedlist.h"

template <typename key_c, typename val_c, class Comparator> struct DictEntry {
  key_c key;
  mutable val_c val;
  int Compare(const DictEntry& sl) const {
    return Comparator::Compare(this->key, sl.key);
  }
  template <class T> int Compare(const T& key) const {
    return Comparator::Compare(this->key, key);
  }
  DictEntry(const DictEntry& v) : key(v.key), val(v.val) {  }
  DictEntry(const key_c& k, const val_c& v) : key(k), val(v) {  }
  DictEntry(const key_c& k) : key(k) {  }
  template <class T> DictEntry(const T& k) : key(k) {  }
  DictEntry& operator = (const DictEntry& v) {  
    key = v.key;
    val = v.val;
    return *this;
  }
};

template <typename KType, typename VType, class Comparator> class olxdict :
protected SortedObjectList<DictEntry<KType,VType,Comparator>, TComparableComparator> {
  typedef DictEntry<KType,VType,Comparator> EntryType;
  typedef SortedObjectList<EntryType, TComparableComparator> SortedL;
public:

  struct Entry  {
    KType key;
    VType value;
    Entry(const KType& _key, const VType& _value) : key(_key), value(_value)  {}
  };

  olxdict() {}
  olxdict(const Entry _values[], size_t cnt) {
    for( size_t i=0; i < cnt; i++ )
      Add(_values[i].key, _values[i].value);
  }
  olxdict(const olxdict& ad) : SortedL(ad) {  }
  olxdict& operator = (const olxdict& ad)  {
    SortedL::operator = (ad);
    return *this;
  }
  template <class T> VType& operator [] (const T& key) const {
    int ind = SortedL::IndexOf(key);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "key");
    return SortedL::operator[] (ind).val;
  }
  inline void Clear()                         {  SortedL::Clear();  }
  inline int Count()                    const {  return SortedL::Count();  }
  inline bool IsEmpty()                 const {  return SortedL::IsEmpty();  }
  inline VType& GetValue(int ind)             {  return SortedL::operator[] (ind).val;  }
  inline const VType& GetValue(int ind) const {  return SortedL::operator[] (ind).val;  }
  inline const KType& GetKey(int ind)   const {  return SortedL::operator[] (ind).key;  }
  inline const EntryType& GetEntry(int ind) const {  return SortedL::operator[] (ind);  }
  template <class T> inline bool HasKey(const T& key) const {
    return SortedL::IndexOf(key) != -1;
  }

  template <typename T> VType& operator () (const T& key, const VType& def) {
    return Add(key, def);
  }
  template <typename T> VType& Add(const T& key, const VType& def) {
    int ind = -1;
    if( SortedL::AddUnique(key, &ind) ) // new entry?
      SortedL::operator[] (ind).val = def;
    return SortedL::operator[] (ind).val;
  }
  template <typename T> VType& Add(const T& key) {
    int ind = -1;
    SortedL::AddUnique(key, &ind);
    return SortedL::operator[] (ind).val;
  }
  template <class T> int IndexOf(const T& key) const {  return SortedL::IndexOf(key);  }
  template <class T> int IndexOfValue(const T& val) const {  
    for( int i=0; i < SortedL::Count(); i++ )
      if( SortedL::operator [] (i).val == val )
        return i;
    return -1;
  }
  template <class T> bool Remove(const T& key)  {
    int ind = SortedL::IndexOf(key);
    if( key != -1 )  {
      SortedL::Delete(ind);
      return true;
    }
    return false;
  }
  void Delete(int ind)  {  SortedL::Delete(ind);  }

};


#endif

