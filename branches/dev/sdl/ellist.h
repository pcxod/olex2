/* primitive directional lists, optimised for sequential access, the right cleanup class must be
chosen to avoid memory leaks! */
#ifndef __olx_sdl_linkedlist_H
#define __olx_sdl_linkedlist_H
#include "ebase.h"
BeginEsdlNamespace()

template <class T> class NewCleanup {
public:
  static inline void DoCleanup(T* v)  {  delete v;  }
};
template <class T> class DummyCleanup {
public:
  static inline void DoCleanup(T& v)  {}
};
template <typename T, class cleanupClass=DummyCleanup<T> > class TUDTypeList  {
  struct Entry  {
    T data;
    Entry* next;
    Entry(T& d) : data(d), next(NULL)  {}
    ~Entry()  {  cleanupClass::DoCleanup(data);  }
  };
  size_t count;
  mutable size_t pos;
  Entry* first, *last;
  mutable Entry *cur;
public:
  TUDTypeList() : count(0), pos(InvalidIndex), cur(NULL), last(NULL), first(NULL) {}
  virtual ~TUDTypeList()  {  Clear();  }
  void Clear()  {
    if( first == NULL )  return;
    cur = first->next;
    while( cur != NULL )  {
      last = cur->next;
      delete cur;
      cur = last;
    }
    delete first;
    cur = last = first = NULL;
    count = 0;
    pos = InvalidIndex;
  }
  void Reset()  {
    cur = first;
    pos = InvalidIndex;
  }
  size_t Count() const {  return count;  }
  T& operator [] (size_t ind) const {
    if( ind >= count )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid index");
    if( pos == (ind-1) )
      return Next();
    else if( ind == 0 )  {
      cur = first;
      pos = 0;
      return cur->data;
    }
    pos = 0;
    cur = first;
    while( ind-- != 1 )  {
      cur = cur->next;
      pos++;
    }
    return cur->data;
  }
  bool HasNext() const {  return cur == NULL ? false : cur->next != NULL;  }
  T& Next() const {  
    if( cur == NULL || cur->next == NULL )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
    cur = cur->next;
    pos++;
    return cur->data;
  }
  T& Add(T v)  {
    if( last == NULL )
      last = first = new Entry(v);
    else  {
      last->next = new Entry(v);
      last = last->next;
    }
    count++;
    pos++;
    return last->data;
  }
};


EndEsdlNamespace()
#endif