/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_queue_h
#define __olx_sdl_queue_h
#include "ebase.h"
BeginEsdlNamespace()

/* very simple queue implementation,
  performs as a
  queue (Push,Pop) alternatively (PushLast, PopFirst)
  stack (PushFirst, PopFirst:pop)
*/
template <class T>
class TQueue {
public:
  struct item {
    T data;
    item* next;
    item(const T& v) : data(v), next(NULL)
    {}
  };
private:
  item *first, *last;
  size_t _count;
public:
  TQueue() : first(NULL), last(NULL), _count(0)
  {}
  TQueue(const TQueue & q) : first(0), last(0), _count(q._count) {
    if (q.first != 0) {
      first = last = new item(q.first->data);
      item *i = q.first->next, *j = first;
      while (i != 0) {
        j->next = new item(i->data);
        last = j = j->next;
        i = i->next;
      }
    }
  }
  ~TQueue()  {  Clear();  }
  void Clear() {
    while (first != NULL) {
      item* p = first->next;
      delete first;
      first = p;
    }
    _count = 0;
  }
  T& Push(const T& v) {
    item* ni = new item(v);
    if (first == NULL ) {
      first = last = ni;
    }
    else {
      last->next = ni;
      last = ni;
    }
    _count++;
    return last->data;
  }
  T& PushLast(const T& v) { return Push(v); }
  T PopFirst() { return Pop(); }
  T& Last() const {
    if (last == NULL) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "queue is empty");
    }
    return last->data;
  }
  T& First() const {
    if (first == NULL) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "queue is empty");
    }
    return first->data;
  }
  item *head() const { return first; }
  T& PushFirst(const T& v) {
    item* ni = new item(v);
    if (first == NULL) {
      first = last = ni;
    }
    else {
      ni->next = first;
      first = ni;
    }
    _count++;
    return first->data;
  }
  T Pop() {
    if (first != NULL) {
      item* i = first->next;
      T rv = first->data;
      delete first;
      first = i;
      _count--;
      return rv;
    }
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "queue is empty");
    // make the compiler happy
    return T();
  }
  bool IsEmpty() const { return _count == 0; }
  size_t Count() const { return _count; }
};

EndEsdlNamespace()
#endif
