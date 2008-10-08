// sorted pointer list - fast checking if objects are in the list
#ifndef __olxs_sptr_list_H
#define __olxs_sptr_list_H
#include "tptrlist.h"
template <class T> class TSPtrList {
  TPtrList<T> list;
protected:
  int FindInsertIndex(const T* entity, bool& exists) {
    int from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  return to;
      int index = (to+from)/2;
      if( list[index] < entity )  from = index;
      else
        if( list[index] > entity )  to  = index;
        else
          if( list[index] == entity )  {  exists = true;  return index;  }
    }
    return -1;  // shold never happen - infinite loop above!
  }

  int FindIndexOf(const T* entity) const {
    if( list.Count() == 0 )  return -1;
    if( list.Count() == 1 )  return (list[0] == entity) ? 0 : -1;
    int from = 0, to = list.Count()-1;
    if( list[from] == entity )  return from;
    if( list[from] > entity )  return -1;
    if( list[to] == entity )  return to;
    if( list[to] < entity )  return -1;
    while( true ) {
      int index = (to+from)/2;
      if( index == from || index == to)  
        return -1;
      if( list[index] < entity )  from = index;
      else
        if( list[index] > entity )  to  = index;
        else
          if( list[index] == entity )  return index;
    }
    return -1;
  }
public:
  TSPtrList() {}
  TSPtrList(const TSPtrList& l) : list(l)  {  }

  bool IsEmpty() const {  return list.IsEmpty();  }
  int Count()    const {  return list.Count();  }
  T*& operator [] (int i) {  return list[i];  }
  const T*& operator [] (int i) const {  return list[i];  }
  // ads an item to the list and returns its' index
  int Add(T* entry)  {
    if( list.IsEmpty() )          {  list.Add( entry);  return 0;  }
    if( list.Count() == 1 )  {
      if(  list[0] < entry )      {  list.Add(entry);  return 1;  }
      else if( list[0] > entry )  {  list.Insert(0, entry);  return 0;  }
      else                        {  list.Add(entry);  return 1;  }
    }
    // smaller than the first
    if( list[0] >= entry )        {  list.Insert(0, entry);  return 0; }
    // larger than the last
    if( list.Last() <= entry )    {  list.Add(entry);  return list.Count()-1; }
    // an easy case then with two items
    if( list.Count() == 2 )       {  list.Insert(1, entry);  return 1; }
    bool exists = false;
    int pos = FindInsertIndex( entry, exists );
    list.Insert(pos, entry);
    return pos;
  }
  // adds an item only if not already in the list, returns its index or -1, if the item exists
  int AddUnique(T* entry)  {
    if( list.IsEmpty() )          {  list.Add( entry);  return 0;  }
    if( list.Count() == 1 )  {
      if(  list[0] < entry )      {  list.Add(entry);  return 1;  }
      else if( list[0] > entry )  {  list.Insert(0, entry);  return 0;  }
      else                        {  return -1;  }
    }
    // smaller than the first
    if( list[0] > entry )        {  list.Insert(0, entry);  return 0; }
    // larger than the last
    if( list.Last() < entry )    {  list.Add(entry);  return list.Count()-1; }
    bool exists = false;
    int pos = FindInsertIndex( entry, exists );
    if( exists )  return -1;
    list.Insert(pos, entry);
    return pos;
  }
  int IndexOf(const T* entity) const {  return FindIndexOf(entity);  }
  // removes specified entry from the list and returns true if the entry was in the list
  bool Remove( const T* entity)  {
    int ind = FindIndexOf(entity);
    if( ind == -1 )  return false;
    list.Delete(ind);
    return true;
  }
  void Delete(int ind)        {  list.Delete(ind);  }
  void Clear()                {  list.Clear();  }
  void SetCapcity( int cap)   {  list.SetCapacity(cap);  }
  void SetIncrement(int incr) {  list.SetIncrement(incr);  }
};

#endif