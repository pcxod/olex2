#ifndef __olx_sdl_queue_h
#define __olx_sdl_queue_h
#include "ebase.h"
BeginEsdlNamespace()

/* very simple queue implementation,
  performs as a
  queue (Push,Pop) alternatively (PushLast, PopFirst)
  stack (PushFirst, PopFirst:pop)
*/
template <class T> class TQueue  {
  struct item  {
    item* next;
    T data;
    item(const T& v) : data(v), next(NULL)  {}
  };
  item* cur, *last;
  int _count;
public:
  TQueue() : cur(NULL), last(NULL), _count(0)  {}
  ~TQueue()  {  Clear();  }
  void Clear()  {
    while( cur != NULL )  {
      item* p = cur->next;
      delete cur;
      cur = p;
    }
    _count = 0;
  }
  inline T& Push(const T& v)  {
    item* ni = new item(v);
    if( cur == NULL )  {
      cur = last = ni;
    }
    else  {
      last->next = ni;
      last = ni;
    }
    _count++;
    return last->data;
  }
  inline T& PushLast(const T& v)  {  return Push(v);  }
  inline T PopFirst()  {  return Pop();  }
  inline T& PushFirst(const T& v)  {
    item* ni = new item(v);
    if( cur == NULL )  {
      cur = last = ni;
    }
    else  {
      ni->next = cur;
      cur = ni;
    }
    _count++;
    return cur->data;
  }
  T Pop()  { 
    if( cur != NULL )  { 
      item* i = cur->next;
      T rv = cur->data;
      delete cur;
      cur = i;
      _count--;
      return rv;
    }
    TExceptionBase::ThrowFunctionFailed(__FILE__, __FUNC__, __LINE__, "queue is empty");
  }
  inline bool IsEmpty() const {  return cur == NULL;  }
  inline int Count() const {  return _count;  }
};

EndEsdlNamespace()
#endif
