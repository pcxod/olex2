/* primitive directional lists */
#ifndef _ellist_h
#define _ellist_h
#include "ebase.h"
BeginEsdlNamespace()
// none is yet tested!!!
template <class T> TUDTypeList  {
  struct Entry  {
    T* data;
    Entry* next;
    Entry(T* d) : data(d), next(NULL) {  }
  };
  int count, pos;
  Entry* first, *last, *cur;
public:
  TUDTypeList() : count(0), pos(-1) {
    cur = last = first = new Entry(NULL);  
  }
  ~TUDTypeList()       {  
    Clear();  
    delete first;
  }
  void Clear()         {
    cur = first->next;
    while( cur != NULL )  {
      delete cur->data;
      last = cur->next;
      delete cur;
      cur = last;
    }
    cur = last = first;   
    count = 0;
  }
  void Reset()         {  cur = first;  pos = -1;  }
  int Count()    const {  return count;  }
  T& operator [] (int ind)  {
    if( pos == (ind-1) )  return Next();
    pos = -1;
    cur = first;
    while( ind-- >= 0 )  {
      cur = cur->next;
      pos++;
    }
    return *cur->data;
  }
  bool HasNext() const {  return !(cur == NULL || cur->next == NULL);  }
  T& Next() {  
    if( first == NULL || cur == NULL || cur->next == NULL )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
    cur = cur->next;
    return *cur->data;
  }
  const T& Next() const {  
    if( first == NULL || cur == NULL || cur->next == NULL )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
    cur = cur->next;
    pos++;
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
    pos++;
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
    pos++;
    return *v;
  }
};


EndEsdlNamespace()
#endif