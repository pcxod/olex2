/* primitive directional lists */
#ifndef _ellist_h
#define _ellist_h
#include "ebase.h"
BeginEsdlNamespace()

template <class T> TUDTypeList  {
  struct Entry  {
    T* data;
    Entry* next;
    Entry(T* d) : data(d), next(NULL) {  }
  };
  int count;
  Entry* first, *last, *cur;
public:
  TUDList() : first(NULL), last(NULL), cur(NULL), count(0)  {  }
  void Reset()  {  cur = NULL;  }
  int Count() const {  return count;  }
  bool HasNext() const {  return cur == NULL ? false : cur->nect != NULL;  }
  T& Next() {  
    if( first == NULL )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "empty list");
    if( cur == NULL )  cur = first;
    else  {
      if( cur->next == NULL )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
      cur = cur->next;
    }
    return *cur->data;
  }
  T& Add(T& v)  {
    if( last == NULL )
      last = first = new Entry(&v);
    else  {
      last->next = new Entry(&v);
      last = last->next;
    }
    count++;
    return v;
  }
  T& Add(T* v)  {
    if( last == NULL )
      last = first = new Entry(v);
    else  {
      last->next = new Entry(v);
      last = last->next;
    }
    count ++;
    return *v;
  }
};

EndEsdlNamespace()
#endif