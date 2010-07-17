// generic sorted list
#ifndef __olxs_sptr_list_H
#define __olxs_sptr_list_H
#include "tptrlist.h"

BeginEsdlNamespace()

template <class ListClass, class Comparator, typename TypeClass> class TTSortedList {
  ListClass list;
protected:
  size_t FindInsertIndex(const TypeClass& entity) const {
    size_t from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  return to;
      const size_t index = (to+from)/2;
      const int cr = Comparator::Compare(list[index], entity);
      if( cr < 0 )  
        from = index;
      else  {
        if( cr > 0 )  
          to  = index;
        else
          if( cr == 0 )  
            return index;  
      }
    }
    return InvalidIndex;  // shold never happen - infinite loop above!
  }
  size_t FindInsertIndexEx(const TypeClass& entity, bool& exists) const {
    size_t from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  {  
        if( Comparator::Compare(list[from], entity) == 0 )  {
          exists = true;
          return from;
        }
        else if( Comparator::Compare(list[to], entity) == 0 )  {
          exists = true;
          return to;
        }
        else
          return to;
      }
      const size_t index = (to+from)/2;
      const int cr = Comparator::Compare(list[index], entity);
      if( cr < 0 )  
        from = index;
      else  {
        if( cr > 0 )  
          to  = index;
        else
          if( cr == 0 )  {  
            exists = true;  
            return index;  
          }
      }
    }
    return InvalidIndex;  // shold never happen - infinite loop above!
  }
  template <class KeyC> size_t FindIndexOf(const KeyC& entity) const {
    if( list.IsEmpty() )  return InvalidIndex;
    if( list.Count() == 1 )  
      return Comparator::Compare(list[0],entity) == 0 ? 0 : InvalidIndex;
    size_t from = 0, to = list.Count()-1;
    const int from_cr = Comparator::Compare(list[from], entity);
    if( from_cr == 0 )  return from;
    if( from_cr > 0  )  return InvalidIndex;
    const int to_cr = Comparator::Compare(list[to], entity);
    if( to_cr == 0 )  return to;
    if( to_cr < 0  )  return InvalidIndex;
    while( true ) {
      const size_t index = (to+from)/2;
      if( index == from || index == to)  
        return InvalidIndex;
      const int cr = Comparator::Compare(list[index], entity);
      if( cr < 0 )  
        from = index;
      else  {
        if( cr > 0 )  
          to  = index;
        else  {
          if( cr == 0 )  
            return index;
        }
      }
    }
    return InvalidIndex;
  }
protected:
  struct Proxy  {
    TypeClass& value;
    Proxy(TypeClass& val) : value(val) {  }
    operator TypeClass& () {  return value;  }
    operator const TypeClass& () const {  return value;  }
    TypeClass& operator = (const TypeClass& v)  {
      throw TFunctionFailedException(__OlxSourceInfo, "cannot modify constant object");
    }
  };
  // to be used by dirived objcts if necessary
  Proxy GetProxyObject(int i)  {  return Proxy(list[i]);  }
public:
  TTSortedList() {}
  TTSortedList(const TTSortedList& l) : list(l.list)  {  }

  bool IsEmpty() const {  return list.IsEmpty();  }
  size_t Count() const {  return list.Count();  }
  const TypeClass& operator [] (size_t i)  const {  return list[i];  }
  TTSortedList& operator = (const TTSortedList& _list)  {
    list = _list.list;
    return *this;
  }
  // adds an item to the list and returns its' index
  size_t Add(TypeClass& entry)  {
    if( list.IsEmpty() )     {  list.Add(entry);  return 0;  }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )     {  list.Add(entry);  return 1;  }
      else if( cmp_val > 0 ) {  list.Insert(0, entry);  return 0;  }
      else                   {  list.Add(entry);  return 1;  }
    }
    // smaller than the first
    if( Comparator::Compare(list[0], entry) >=0  )  {  list.Insert(0, entry);  return 0; }
    // larger than the last
    if( Comparator::Compare(list.Last(), entry) <=0 )  {  list.Add(entry);  return list.Count()-1; }
    // an easy case then with two items
    if( list.Count() == 2 )       {  list.Insert(1, entry);  return 1; }
    const size_t pos = FindInsertIndex(entry);
    list.Insert(pos, entry);
    return pos;
  }
  size_t Add(const TypeClass& entry)  {
    if( list.IsEmpty() )     {  list.Add(entry);  return 0;  }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )     {  list.Add(entry);  return 1;  }
      else if( cmp_val > 0 ) {  list.Insert(0, entry);  return 0;  }
      else                   {  list.Add(entry);  return 1;  }
    }
    // smaller than the first
    if( Comparator::Compare(list[0], entry) >=0  )  {  list.Insert(0, entry);  return 0; }
    // larger than the last
    if( Comparator::Compare(list.Last(), entry) <=0 )  {  list.Add(entry);  return list.Count()-1; }
    // an easy case then with two items
    if( list.Count() == 2 )       {  list.Insert(1, entry);  return 1; }
    const size_t pos = FindInsertIndex(entry);
    list.Insert(pos, entry);
    return pos;
  }
  /* adds an item only if not already in the list, returns true if the item is added, pos is is 
  initialised with the item index */
  bool AddUnique(TypeClass& entry, size_t* pos=NULL)  {
    if( list.IsEmpty() )  {  
      list.Add(entry);  
      if( pos != NULL )  *pos = 0;
      return true;  
    }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )  {  
        list.Add(entry);  
        if( pos != NULL )  *pos = 1;
        return true;  
      }
      else if( cmp_val > 0 )  {  
        list.Insert(0, entry);  
        if( pos != NULL )  *pos = 0;      
        return true;  
      }
      else  {  
        if( pos != NULL )  *pos = 0;
        return false;  
      }
    }
    if( Comparator::Compare(list[0],entry) > 0 )  {  // smaller than the first
      list.Insert(0, entry);  
      if( pos != NULL )  *pos = 0;
      return true; 
    }
    if( Comparator::Compare(list.Last(), entry) < 0 )  {  // larger than the last
      list.Add(entry);  
      if( pos != NULL ) *pos = list.Count()-1;
      return true; 
    }
    bool exists = false;
    size_t ps = FindInsertIndexEx( entry, exists );
    if( pos != NULL )  *pos = ps;
    if( exists )  return false;
    list.Insert(ps, entry);
    return true;
  }
  bool AddUnique(const TypeClass& entry, size_t* pos = NULL)  {
    if( list.IsEmpty() )  {  
      list.Add(entry);  
      if( pos != NULL )  *pos = 0;
      return true;  
    }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )  {  
        list.Add(entry);  
        if( pos != NULL )  *pos = 1;
        return true;  
      }
      else if( cmp_val > 0 )  {  
        list.Insert(0, entry);  
        if( pos != NULL )  *pos = 0;      
        return true;  
      }
      else  {  
        if( pos != NULL )  *pos = 0;
        return false;  
      }
    }
    if( Comparator::Compare(list[0],entry) > 0 )  {  // smaller than the first
      list.Insert(0, entry);  
      if( pos != NULL )  *pos = 0;
      return true; 
    }
    if( Comparator::Compare(list.Last(), entry) < 0 )  {  // larger than the last
      list.Add(entry);  
      if( pos != NULL ) *pos = list.Count()-1;
      return true; 
    }
    bool exists = false;
    size_t ps = FindInsertIndexEx( entry, exists );
    if( pos != NULL )  *pos = ps;
    if( exists )  return false;
    list.Insert(ps, entry);
    return true;
  }
  template <class KeyC>
  size_t IndexOf(const KeyC& entity) const {  return FindIndexOf(entity);  }
  // removes specified entry from the list and returns true if the entry was in the list
  bool Remove(const TypeClass& entity)  {
    size_t ind = FindIndexOf(entity);
    if( ind == InvalidIndex )  return false;
    list.Delete(ind);
    return true;
  }
  void Delete(size_t ind)  {  list.Delete(ind);  }
  void Clear()  {  list.Clear();  }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
  // may be useful for copy constructors, etc
  const ListClass& GetList() const {  return list;  }
  // allows to remove multiple items using a condition
  template <class PackAnalyser> void Pack(const PackAnalyser& pa)  {  list.Pack(pa);  }
  template <class PackAnalyser> void PackEx(const PackAnalyser& pa)  {  list.PackEx(pa);  }
  template <class Functor> void ForEach(const Functor& f) const {  list.ForEach(f);  }
  template <class Functor> void ForEachEx(const Functor& f) const {  list.ForEachEx(f);  }
};
//............................................................................................
// a simple object list to use with sorted list
//............................................................................................
template <class ObjectClass> class TObjectList {
  TPtrList<ObjectClass> list;
public:
  TObjectList() {  }
  TObjectList(const TObjectList& li) {  
    SetCapacity(li.Count());
    for( size_t i=0; i < li.Count(); i++ )
      Add(li[i]);
  }
  ~TObjectList() {  Clear();  }
  ObjectClass& operator [] (size_t i)  {  return *list[i];  }
  const ObjectClass& operator [] (size_t i) const {  return *list[i];  }
  ObjectClass& Last()  {  return *list.Last();  }
  const ObjectClass& Last() const {  return *list.Last();  }

  size_t Count() const {  return list.Count();  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  TObjectList<ObjectClass>& operator = (const TObjectList<ObjectClass>& li)  {
    Clear();
    SetCapacity(li.Count());
    for( size_t i=0; i < li.Count(); i++ )
      Add(li[i]);
    return *this;
  }
  void Add(const ObjectClass& obj)  {  list.Add(new ObjectClass(obj));  }
  void Insert(size_t index, const ObjectClass& obj)  {
    list.Insert(index, new ObjectClass(obj) );
  }

  void Clear()  {
    for( size_t i=0; i < list.Count(); i++ )
      delete list[i];
    list.Clear();
  }
  void Delete(size_t ind)  {
    if( list[ind] != NULL )
      delete list[ind];
    list.Delete(ind);
  }
  void SetCapacity(size_t cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(size_t incr)  {  list.SetIncrement(incr);  }
};
//............................................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for objects having < and > operators only
  TComparableComparator - for objects having Compare method returning -,+,0
*/
template <class ObjectClass, class Comparator> 
class SortedObjectList : public TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass> {
public:
  SortedObjectList() {  }
  SortedObjectList(const SortedObjectList& l) : 
    TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>(l) {  }
  SortedObjectList& operator = (const SortedObjectList& l)  {
    TTSortedList<TObjectList<ObjectClass>, Comparator, ObjectClass>::operator = (l);
    return *this;
  }
};
//............................................................................................
/* A choice of comprators is provided:
  TPrimitiveComparator - for sorting Objects 
  TComparableComparator - for sorting objects having Compare method returning -,+,0
  TPointerPtrComparator - for sorting pointer adresses
*/
template <class ObjectClass, class Comparator> 
class SortedPtrList : public TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*> {
public:
  SortedPtrList() {  }
  SortedPtrList(const SortedPtrList& l) : 
    TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>(l) {  }
  SortedPtrList& operator = (const SortedPtrList& l)  {
    TTSortedList<TPtrList<ObjectClass>, Comparator, ObjectClass*>::operator = (l);
    return *this;
  }
};

EndEsdlNamespace()
#endif

