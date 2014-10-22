/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_listbox_H
#define __olx_ctrl_listbox_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "itemlist.h"
#include "wx/listbox.h"
#include "wx/listctrl.h"

namespace ctrl_ext {

  class TListBox : public TItemList<wxListBox>, public AOlxCtrl {
    void DblClickEvent(wxMouseEvent& event);
    void ItemSelectEvent(wxCommandEvent& event);
    olxstr Data;
  public:
    TListBox(wxWindow *Parent)
      : AOlxCtrl(this),
      OnSelect(AOlxCtrl::ActionQueue::New(Actions, evt_on_select_id)),
      OnDblClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_dbl_click_id))
    {
      wxListBox::Create(Parent, -1);
    }
    ~TListBox() {
      _Clear();
    }

    DefPropC(olxstr, Data)

    void Clear() {
      _Clear();
      wxListBox::Clear();
    }

    olxstr GetValue(size_t i) const {
      olx_pair_t<bool, olxstr> v = _GetText(i);
      return (v.a ? v.b : GetItem(i));
    }

    bool SetValue(const olxstr &v) {
      olx_pair_t<size_t, olxstr> found = _SetText(v);
      if (found.a != InvalidIndex) {
        SetSelection((int)found.a);
      }
      else {
        SetSelection(-1);
      }
    }
    olxstr GetValue() const  {
      int i = GetSelection();
      return (i < 0 ? EmptyString() : _GetText(i).b);
    }

    ActionQueue &OnDblClick, &OnSelect;

    DECLARE_CLASS(TListBox)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
