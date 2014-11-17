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
    TTextEdit(wxWindow *Parent, wxWindowID id = -1,
      const wxString& value = wxEmptyString,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0);

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
  };
}; // end namespace ctrl_ext
#endif
