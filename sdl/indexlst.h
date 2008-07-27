#ifndef __olx_index_list
#define __olx_index_list
#include "estlist.h"

BeginEsdlNamespace()
// key class, object class
template <class KC, class OC> class TIndexList {
  struct iEntry {
    OC* obj;
    int ind;
  };
  TPSTypeList<KC, iEntry> SortedList;
  TTypeList<OC> List;
public:
  TIndexList() {}
  TIndexList(const TIndexList& il) {
    List.SetCapacity( il.Count() );
    SortedList.SetCapacity( il.Count() );
    for( int i=0; i < il.Count(); i++ )
      List.AddCCopy(il[i]);
    for( int i=0; i < il.Count(); i++ )  {
      iEntry en = { &List[il.SortedList.Object(i).ind], il.SortedList.Object(i).ind };
      SortedList.Add( il.SortedList.GetComparable(i), en);
    }
  }
  inline OC& operator [] (int i)             {  return List[i];  }
  inline const OC& operator [] (int i) const {  return List[i];  }
  inline int Count()                   const {  return List.Count();  }
  inline bool IsEmpty()                const {  return List.IsEmpty();  }
  OC& Last()                                 {  return List.Last();  }
  OC* FindByKey(const KC& key) {  
    int i = SortedList.IndexOfComparable(key);
    return i == -1 ? NULL : SortedList.Object(i).obj;
  }
  inline TIndexList& SetCapacity(int cp)  {
    List.SetCapacity(cp);
    SortedList.SetCapacity(cp);
    return *this;
  }
  TIndexList& operator = (const TIndexList& il)  {
    Clear();
    List.SetCapacity( il.Count() );
    SortedList.SetCapacity( il.Count() );
    for( int i=0; i < il.Count(); i++ )
      List.AddCCopy(il[i]);
    for( int i=0; i < il.Count(); i++ )  {
      iEntry en = { &List[il.SortedList.Object(i).ind], il.SortedList.Object(i).ind };
      SortedList.Add( il.SortedList.GetComparable(i), en);
    }
    return *this;
  }
  // takes the ownership of the object - will be deleted
  inline OC& Add(const KC& key, OC* obj)  {
    List.Add(*obj);
    iEntry entry = { obj, SortedList.Count() };
    SortedList.Add(key, entry);
    return *obj;
  }
  inline void Clear()  {
    SortedList.Clear();
    List.Clear();
  }
};

EndEsdlNamespace()
#endif