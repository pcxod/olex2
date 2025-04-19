/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once

#include "typelist.h"

BeginEsdlNamespace()

class AReleasable;
class IReleasableContainer {
protected:
  virtual void Release(AReleasable& item) = 0;
  virtual void Restore(AReleasable& item) = 0;
  virtual void Add(AReleasable* item) = 0;
public:
  virtual ~IReleasableContainer() {}
  friend class AReleasable;
};

class AReleasable {
  size_t ReleasableId;
public:
  typedef IReleasableContainer parent_t;
  parent_t& parent;
  size_t GetReleasableId() const { return ReleasableId; }
  void SetReleasableId(size_t id) {
    ReleasableId = id;
  }
public:
  AReleasable(parent_t& parent);
  virtual ~AReleasable() {}

  void Release();
  void Restore();
  virtual void OnReleasedDelete() {}
};

template <class item_t>
class AReleasableContainer : public IReleasableContainer {
protected:
  TTypeList<item_t> items;
  void Add(AReleasable* item) {
    item->SetReleasableId(items.Count());
    items.Add((item_t*)item);
  }
  void Release(AReleasable& item) {
    items.Release(item.GetReleasableId());
    for (size_t i = 0; i < items.Count(); i++) {
      items[i].SetReleasableId(i);
    }
    OnRelease(dynamic_cast<item_t&>(item));
  }
  //.............................................................................
  void Restore(AReleasable& item) {
    item.SetReleasableId(items.Count());
    item_t& i = dynamic_cast<item_t&>(item);
    items.Add(i);
    OnRestore(i);
  }
  virtual void OnRestore(item_t& item) = 0;
  //.............................................................................
  virtual void OnRelease(item_t& item) = 0;
public:
  virtual ~AReleasableContainer()
  {}
  virtual void Clear() { items.Clear(); }
  //.............................................................................
  void Delete(AReleasable& i) {
    items.Delete(i.GetReleasableId());
  }
  size_t Count() const { return items.Count(); }
  bool IsEmpty() const { return items.IsEmpty(); }

  item_t& operator [] (size_t i) const {
    return items[i];
  }

  item_t& GetItem(size_t i) const {
    return items[i];
  }
};

EndEsdlNamespace()
