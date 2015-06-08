/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_ilist_H
#define __olx_ctrl_ilist_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "wx/combo.h"

namespace ctrl_ext {
  template <class parent_t>
  class TItemList : public parent_t {
    struct TDataObj {
      IOlxObject* Data;
      bool Delete;
      TDataObj(IOlxObject *Data, bool Delete)
        : Data(Data), Delete(Delete)
      {}
    };
  protected:
    void _AddObject(const olxstr &Item, IOlxObject* Data, bool Delete) {
      parent_t::Append(Item.u_str());
      if (Data != NULL)  {
        TDataObj* d_o = new TDataObj(Data, Delete);
        parent_t::SetClientData(parent_t::GetCount() - 1, d_o);
      }
      else
        parent_t::SetClientData(parent_t::GetCount() - 1, NULL);
    }
    void _Clear() {
      for (unsigned int i = 0; i < parent_t::GetCount(); i++) {
        TDataObj* d_o = (TDataObj*)parent_t::GetClientData(i);
        if (d_o != NULL) {
          if (d_o->Delete) {
            delete d_o->Data;
          }
          delete d_o;
        }
      }
    }
  public:
    void AddObject(const olxstr &Item, IOlxObject *Data = NULL) {
      _AddObject(Item, Data, false);
    }

    olxstr GetItem(size_t i) const { return parent_t::GetString(i); }

    const IOlxObject* GetObject(size_t i) const {
      TDataObj* res = (TDataObj*)parent_t::GetClientData(i);
      return (res != NULL && !res->Delete) ? res->Data : NULL;
    }


    /*if a list item is constructed like 'name<-value' the pair is added as
    single item, though, once this item is selected, the value of Text()
    function will be the valu part of the item
    */
    void AddItems(const TStrList &items) {
      for (size_t i = 0; i < items.Count(); i++) {
        size_t ind = items[i].IndexOf("<-");
        if (ind != InvalidIndex) {
          olxstr tmp = items[i].SubStringFrom(ind + 2);
          _AddObject(items[i].SubStringTo(ind), tmp.Replicate(), true);
        }
        else
          _AddObject(items[i], NULL, false);
      }
    }

    olxstr ItemsToString(const olxstr &sep) {
      olxstr_buf rv;
      olxstr vs = "<-";
      for (unsigned int i = 0; i < parent_t::GetCount(); i++) {
        rv << sep << parent_t::GetString(i);
        TDataObj* res = (TDataObj*)parent_t::GetClientData(i);
        if (res != NULL && res->Delete)
          rv << vs << res->Data->ToString();
      }
      return olxstr(rv).SubStringFrom(sep.Length());
    }

    size_t Count() const { return parent_t::GetCount(); }
  protected:
    olx_pair_t<size_t, olxstr> _SetText(const olxstr &text) {
      size_t idx = InvalidIndex;
      for (unsigned int i = 0; i < parent_t::GetCount(); i++)  {
        if (text == parent_t::GetString(i)) {
          idx = i;
        }
        TDataObj* res = (TDataObj*)parent_t::GetClientData(i);
        if (res == NULL || !res->Delete) {
          continue;
        }
        olxstr sv = res->Data->ToString();
        if (sv == text) {
          return olx_pair_t<size_t,olxstr>(i, GetItem(i));
        }
      }
      return olx_pair_t<size_t, olxstr>(idx, text);
    }
    olx_pair_t<bool, olxstr> _GetText(size_t i) const {
      const TDataObj* res = (TDataObj*)parent_t::GetClientData(i);
      return (res == NULL || !res->Delete) ?
        olx_pair_t<bool, olxstr>(false, EmptyString())
        : olx_pair_t<bool, olxstr>(true, res->Data->ToString());
    }
  };
};  // end namespace ctrl_ext
#endif
