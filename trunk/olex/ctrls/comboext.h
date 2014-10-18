/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_combo_H
#define __olx_ctrl_combo_H
#include "olxctrlbase.h"
#include "itemlist.h"
#include "estrlist.h"
#include "wx/combo.h"

namespace ctrl_ext {
  class TComboBox : public TItemList<wxComboBox>, public AOlxCtrl {
    void ChangeEvent(wxCommandEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    void EnterEvent(wxFocusEvent& event);
    olxstr Data;
    olxstr StrValue;
    int entered_counter;
    bool OnChangeAlways;
  public:
    TComboBox(wxWindow *Parent, bool ReadOnly=false, const wxSize& sz=wxDefaultSize) :
      AOlxCtrl(this),
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
      OnLeave(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_leave_id)),
      OnEnter(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_enter_id)),
      OnReturn(AOlxCtrl::ActionQueue::New(Actions, evt_on_return_id))
    {
      wxComboBox::Create(Parent, -1, wxString(), wxDefaultPosition, sz, 0, NULL,
        wxCB_DROPDOWN | (ReadOnly ? wxCB_READONLY : 0) | wxTE_PROCESS_ENTER);
      OnLeave.SetEnabled(false);
      entered_counter = 0;
      OnChangeAlways = false;
    }
    virtual ~TComboBox();

    void Clear();

    bool HasValue() const {
#if wxCHECK_VERSION(2,9,0)
      if (IsReadOnly() && GetSelection() < 0)
        return false;
#endif
      return true;
    }
    olxstr GetText() const;
    void SetText(const olxstr &text);

    DefPropC(olxstr, Data)

    bool IsReadOnly() const { return WI.HasWindowStyle(wxCB_READONLY); }
    void SetReadOnly(bool v) {
      v ? WI.AddWindowStyle(wxCB_READONLY) : WI.DelWindowStyle(wxCB_READONLY);
    }
    DefPropBIsSet(OnChangeAlways)

    void HandleOnLeave();
    void HandleOnEnter();

    ActionQueue &OnChange, &OnLeave, &OnEnter, &OnReturn;

    DECLARE_CLASS(TComboBox)
    DECLARE_EVENT_TABLE()
  };
};  // end namespace ctrl_ext
#endif
