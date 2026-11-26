/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "edict.h"
#include "bapp.h"

template <class T> class TAccelList : public IOlxObject {
  olx_pdict<int32_t, T> Entries;
protected:
  int32_t LastId;
public:
  TAccelList() { LastId = 0; }

  void Clear() { Entries.Clear(); }

  T& GetValue(int32_t key) {
    const size_t ind = Entries.IndexOf(key);
    if (ind == InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo, "key");
    }
    return Entries.GetValue(ind);
  }

  bool ValueExists(int32_t key) {
    return Entries.IndexOf(key) != InvalidIndex;
  }

  void AddAccell(int32_t id, const T& value) {
    const size_t ind = Entries.IndexOf(id);
    if (ind != InvalidIndex) {
      Entries.GetValue(ind) = value;
    }
    else {
      Entries.Add(id, value);
      LastId = id;
    }
  }
  bool RemoveAccell(int32_t id, bool quiet=false) {
    const size_t ind = Entries.IndexOf(id);
    if (ind == InvalidIndex) {
      if (!quiet) {
        throw TInvalidArgumentException(__OlxSourceInfo, "id");
      }
      return false;
    }
    Entries.Delete(ind);
    return true;
  }

  inline int32_t GetLastId() const { return LastId; }
};
