/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_textedit_H
#define __olx_ctrl_textedit_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class TTextEdit: public wxTextCtrl, public AOlxCtrl  {
  protected:
    void ClickEvent(wxMouseEvent& event);
    void ChangeEvent(wxCommandEvent& event);
    void KeyDownEvent(wxKeyEvent& event);
    void CharEvent(wxKeyEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    void EnterEvent(wxFocusEvent& event);
    olxstr Data;
    olxstr StrValue;
  public:
    TTextEdit(wxWindow *Parent, int style=0) :
      wxTextCtrl(Parent, -1, wxString(), wxDefaultPosition, wxDefaultSize, style),
      AOlxCtrl(this),
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
      OnLeave(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_leave_id)),
      OnEnter(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_enter_id)),
      OnReturn(AOlxCtrl::ActionQueue::New(Actions, evt_on_return_id)),
      OnClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_click_id)),
      OnChar(AOlxCtrl::ActionQueue::New(Actions, evt_on_char_id)),
      OnKeyDown(AOlxCtrl::ActionQueue::New(Actions, evt_on_key_down_id))  {}

    olxstr GetText() const {  return wxTextCtrl::GetValue(); }
    void SetText(const olxstr &T)  {
      StrValue = T;
      TActionQueueLock ql(&OnChange);
      wxTextCtrl::SetValue(StrValue.u_str());
    }

    inline bool IsReadOnly() const {   return WI.HasWindowStyle(wxTE_READONLY);  }
    inline void SetReadOnly(bool v)  {  WI.AddWindowStyle(wxTE_READONLY);  }

    DefPropC(olxstr, Data)

      AOlxCtrl::ActionQueue &OnChange, &OnReturn, &OnLeave, &OnEnter, 
        &OnClick, &OnChar, &OnKeyDown;

    DECLARE_CLASS(TTextEdit)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
