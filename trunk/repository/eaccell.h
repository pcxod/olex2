/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_eaccell_H
#define __olx_eaccell_H
#include "edict.h"
#include "bapp.h"

template <class T> class TAccellList : public IOlxObject  {
  olx_pdict<int32_t, T> Entries;
  protected:
    int32_t LastId;
  public:
    TAccellList()  {  LastId = 0;  }
    virtual ~TAccellList()  {}

    inline void Clear()  {  Entries.Clear();  }

    T& GetValue(int32_t key)  {
      const size_t ind = Entries.IndexOf(key);
      if( ind == InvalidIndex )
        throw TInvalidArgumentException(__OlxSourceInfo, "key");
      return Entries.GetValue(ind);
    }

    bool ValueExists(int32_t key)  {
      return Entries.IndexOf(key) != InvalidIndex;
    }

    void AddAccell(int32_t id, const T& value)  {
      const size_t ind = Entries.IndexOf(id);
      if( ind != InvalidIndex )
        Entries.GetValue(ind) = value;
      else  {
        Entries.Add(id, value);
        LastId = id;
      }
    }
    void RemoveAccell(int32_t id) {
      const size_t ind = Entries.IndexOf(id);
      if( ind == InvalidIndex )
        throw TInvalidArgumentException(__OlxSourceInfo, "id");
      Entries.Delete(ind);
    }

    inline int32_t GetLastId() const {  return LastId;  }
};
#endif
