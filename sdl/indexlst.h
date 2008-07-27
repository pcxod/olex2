#ifndef __olx_index_list
#define __olx_index_list
#include "estlist.h"

BeginEsdlNamespace()
// key class, object class
template <class KC, class OC> class TIndexList {
  TPSTypeList<KC, OC*> SortedList;
  TTypeList<OC> List;
public:
  TIndexList() {}
  inline OC& operator [] (int i)             {  return List[i];  }
  inline const OC& operator [] (int i) const {  return List[i];  }
  inline int Count()                   const {  return List.Count();  }
  inline bool IsEmpty()                const {  return List.IsEmpty();  }
  OC& Last()                                 {  return List.Last();  }
  OC* FindByKey(const KC& key) {  
    int i = SortedList.IndexOfComparable(key);
    return i == -1 ? NULL : SortedList.Object(i);
  }
  inline OC& Add(const KC& key, OC* obj)  {
    List.Add(*obj);
    SortedList.Add(key, obj);
    return *obj;
  }
  inline void Clear()  {
    SortedList.Clear();
    List.Clear();
  }
};

EndEsdlNamespace()
#endif