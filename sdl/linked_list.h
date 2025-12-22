/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
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

class NewCleanup {
public:
  template <class T>
  static void DoCleanup(T* v) { delete v; }
};
class FreeCleanup {
public:
  template <class T>
  static void DoCleanup(T* v) { free(v); }
};
class DummyCleanup {
public:
  template <class T>
  static void DoCleanup(T& v) {}
};

// primitive linked list
template <typename T, class cleanupClass=DummyCleanup>
class TLinkedList {
protected:
  struct Entry {
    T data;
    Entry* next;
    Entry(const T& d) : data(d), next(0) {}
    ~Entry() {
      cleanupClass::DoCleanup(data);
    }
  };
  size_t count;
  Entry *first, *last;
public:
  TLinkedList()
    : count(0), first(0), last(0)
  {}
  
  virtual ~TLinkedList() { Clear(); }
  
  void Clear() {
    if (first == 0) {
      return;
    }
    Entry *cur = first->next;
    while (cur != 0) {
      last = cur->next;
      delete cur;
      cur = last;
    }
    delete first;
    first = last = 0;
    count = 0;
  }
  
  T& Add(const T &v) {
    if (first == 0) {
      last = first = new Entry(v);
    }
    else {
      last->next = new Entry(v);
      last = last->next;
    }
    count++;
    return last->data;
  }

  template <typename T1>
  void AddAll(const T1 *all, size_t sz) {
    for (size_t i = 0; i < sz; i++) {
      Add(all[i]);
    }
  }

  template <class list_t>
  void AddAll(const list_t &all) {
    for (size_t i = 0; i < all.Count(); i++) {
      Add(all[i]);
    }
  }

  struct Iterator {
  private:
    Entry *first, *cur;
    size_t count;
  public:
    Iterator()
      : first(0), cur(0), count(0)
    {}
    Iterator(const TLinkedList &parent)
      : first(parent.first), cur(0), count(parent.count)
    {}
    bool HasNext() const {
      return (cur == 0 ? first : cur->next) != 0;
    }
    bool IsEmpty() const { return cur == 0; }
    size_t Count() const { return count; }
    T& Next() {
      if (cur != 0) {
        cur = cur->next;
      }
      else {
        cur = first;
      }
      if (cur == 0) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "end of the list");
      }
      return cur->data;
    }

    T& Lookup() const {
      Entry* c = cur;
      if (c != 0) {
        c = c->next;
      }
      else {
        c = first;
      }
      if (c == 0) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "end of the list");
      }
      return c->data;
    }
  };

  Iterator GetIterator() const { return Iterator(*this); }

  TLinkedList &TakeOver(TLinkedList& src, bool do_delete = false) {
    Clear();
    first = src.first;
    last = src.last;
    count = src.count;

    src.first = src.last = 0;
    src.count = 0;
    if (do_delete) {
      delete& src;
    }
    return *this;
  }
public:
  typedef T list_item_type;
};

EndEsdlNamespace()
#endif
