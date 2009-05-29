// generic sorted list
#ifndef __olxs_sptr_list_H
#define __olxs_sptr_list_H
#include "tptrlist.h"

BeginEsdlNamespace()

template <class ListClass, class Comparator, typename TypeClass> class TTSortedList {
  ListClass list;
protected:
  int FindInsertIndex(const TypeClass& entity) const {
    int from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  return to;
      const int index = (to+from)/2;
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
    return -1;  // shold never happen - infinite loop above!
  }
  int FindInsertIndexEx(const TypeClass& entity, bool& exists) const {
    int from = 0, to = list.Count()-1;
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
      const int index = (to+from)/2;
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
    return -1;  // shold never happen - infinite loop above!
  }
  template <class KeyC> int FindIndexOf(const KeyC& entity) const {
    if( list.IsEmpty() )  
      return -1;
    if( list.Count() == 1 )  
      return Comparator::Compare(list[0],entity) == 0 ? 0 : -1;
    int from = 0, to = list.Count()-1;
    const int from_cr = Comparator::Compare(list[from], entity);
    if( from_cr == 0 )  return from;
    if( from_cr > 0  )  return -1;
    const int to_cr = Comparator::Compare(list[to], entity);
    if( to_cr == 0 )  return to;
    if( to_cr < 0  )  return -1;
    while( true ) {
      const int index = (to+from)/2;
      if( index == from || index == to)  
        return -1;
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
    return -1;
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

  bool IsEmpty()                const {  return list.IsEmpty();  }
  int Count()                   const {  return list.Count();  }
  const TypeClass& operator [] (int i)  const {  return list[i];  }
  TTSortedList& operator = (const TTSortedList& _list)  {
    list = _list.list;
    return *this;
  }
  // adds an item to the list and returns its' index
  int Add(TypeClass& entry)  {
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
    const int pos = FindInsertIndex(entry);
    list.Insert(pos, entry);
    return pos;
  }
  int Add(const TypeClass& entry)  {
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
    const int pos = FindInsertIndex(entry);
    list.Insert(pos, entry);
    return pos;
  }
  /* adds an item only if not already in the list, returns true if the item is added, pos is is 
  initialised with the item index */
  bool AddUnique(TypeClass& entry, int& pos)  {
    if( list.IsEmpty() )  {  
      list.Add(entry);  
      pos = 0;
      return true;  
    }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )  {  
        list.Add(entry);  
        pos = 1;
        return true;  
      }
      else if( cmp_val > 0 )  {  
        list.Insert(0, entry);  
        pos = 0;      
        return true;  
      }
      else  {  
        pos = 0;
        return false;  
      }
    }
    if( Comparator::Compare(list[0],entry) > 0 )  {  // smaller than the first
      list.Insert(0, entry);  
      pos = 0;
      return true; 
    }
    if( Comparator::Compare(list.Last(), entry) < 0 )  {  // larger than the last
      list.Add(entry);  
      pos = list.Count()-1;
      return true; 
    }
    bool exists = false;
    pos = FindInsertIndexEx( entry, exists );
    if( exists )  return false;
    list.Insert(pos, entry);
    return true;
  }
  bool AddUnique(const TypeClass& entry, int& pos)  {
    if( list.IsEmpty() )  {  
      list.Add(entry);  
      pos = 0;
      return true;  
    }
    if( list.Count() == 1 )  {
      const int cmp_val = Comparator::Compare(list[0], entry);
      if(  cmp_val < 0 )  {  
        list.Add(entry);  
        pos = 1;
        return true;  
      }
      else if( cmp_val > 0 )  {  
        list.Insert(0, entry);  
        pos = 0;      
        return true;  
      }
      else  {  
        pos = 0;
        return false;  
      }
    }
    if( Comparator::Compare(list[0],entry) > 0 )  {  // smaller than the first
      list.Insert(0, entry);  
      pos = 0;
      return true; 
    }
    if( Comparator::Compare(list.Last(), entry) < 0 )  {  // larger than the last
      list.Add(entry);  
      pos = list.Count()-1;
      return true; 
    }
    bool exists = false;
    pos = FindInsertIndexEx( entry, exists );
    if( exists )  return false;
    list.Insert(pos, entry);
    return true;
  }
  template <class KeyC>
  int IndexOf(const KeyC& entity) const {  return FindIndexOf(entity);  }
  // removes specified entry from the list and returns true if the entry was in the list
  bool Remove(const TypeClass& entity)  {
    int ind = FindIndexOf(entity);
    if( ind == -1 )  return false;
    list.Delete(ind);
    return true;
  }
  void Delete(int ind)        {  list.Delete(ind);  }
  void Clear()                {  list.Clear();  }
  void SetCapacity( int cap)  {  list.SetCapacity(cap);  }
  void SetIncrement(int incr) {  list.SetIncrement(incr);  }
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
    for( int i=0; i < li.Count(); i++ )
      Add(li[i]);
  }
  ~TObjectList() {  
    Clear();
  }
  ObjectClass& operator [] (int i)  {  return *list[i];  }
  const ObjectClass& operator [] (int i) const {  return *list[i];  }
  ObjectClass& Last()  {  return *list.Last();  }
  const ObjectClass& Last(int i) const {  return *list.Last();  }

  int Count() const {  return list.Count();  }
  bool IsEmpty() const {  return list.IsEmpty();  }
  TObjectList<ObjectClass>& operator = (const TObjectList<ObjectClass>& li)  {
    Clear();
    SetCapacity(li.Count());
    for( int i=0; i < li.Count(); i++ )
      Add(li[i]);
    return *this;
  }
  void Add(const ObjectClass& obj)  {
    list.Add( new ObjectClass(obj) );
  }
  void Insert(int index, const ObjectClass& obj)  {
    list.Insert(index, new ObjectClass(obj) );
  }

  void Clear()  {
    for( int i=0; i < list.Count(); i++ )
      delete list[i];
    list.Clear();
  }
  void Delete(int ind)  {
    if( list[ind] != NULL )
      delete list[ind];
    list.Delete(ind);
  }
  void SetCapacity( int cap)   {  list.SetCapacity(cap);  }
  void SetIncrement(int incr) {  list.SetIncrement(incr);  }
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

