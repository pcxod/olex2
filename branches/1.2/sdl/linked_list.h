/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* primitive directional lists, the right cleanup class must be chosen to
avoid memory leaks!
*/
#ifndef __olx_sdl_linked_list_H
#define __olx_sdl_linked_list_H
#include "ebase.h"
BeginEsdlNamespace()

template <class T> class NewCleanup {
public:
  static void DoCleanup(T* v) { delete v; }
};
template <class T> class FreeCleanup {
public:
  static void DoCleanup(T* v) { free(v); }
};
template <class T> class DummyCleanup {
public:
  static void DoCleanup(T& v) {}
};

// primitive linked list
template <typename T, class cleanupClass=DummyCleanup<T> > class TLinkedList {
protected:
  struct Entry  {
    T data;
    Entry* next;
    Entry(T& d) : data(d), next(NULL) {}
    ~Entry()  { cleanupClass::DoCleanup(data); }
  };
  size_t count;
  Entry *first, *last;
public:
  TLinkedList() : count(0), first(NULL), last(NULL) {}
  virtual ~TLinkedList() { Clear(); }
  void Clear() {
    if (first == NULL ) return;
    Entry *cur = first->next;
    while (cur != NULL) {
      last = cur->next;
      delete cur;
      cur = last;
    }
    delete first;
    first = last = NULL;
    count = 0;
  }
  T& Add(T v) {
    if (first == NULL)
      last = first = new Entry(v);
    else {
      last->next = new Entry(v);
      last = last->next;
    }
    count++;
    return last->data;
  }

  struct Iterator {
    const TLinkedList &parent;
    Entry *cur;
    Iterator(const TLinkedList &_parent) : parent(_parent), cur(NULL) {}
    bool HasNext() const {
      return (cur == NULL ? parent.first : cur->next) != NULL;
    }
    bool IsEmpty() const { return cur == NULL; }
    size_t Count() const { return parent.count; }
    T& Next() {
      if (cur != NULL)
        cur = cur->next;
      else
        cur = parent.first;
      if (cur == NULL) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "end of the list");
      }
      return cur->data;
    }
  };

  Iterator GetIterator() const { return Iterator(*this); }
public:
  typedef T list_item_type;
};

EndEsdlNamespace()
#endif
