/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "texteditext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
TTextEdit::TTextEdit(wxWindow *Parent, wxWindowID id, const wxString& value,
  const wxPoint& pos, const wxSize& size, long style)
: wxTextCtrl(Parent, id, value, pos, size, style),
  AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
  OnLeave(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_leave_id)),
  OnEnter(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_enter_id)),
  OnReturn(AOlxCtrl::ActionQueue::New(Actions, evt_on_return_id)),
  OnClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_click_id)),
  OnChar(AOlxCtrl::ActionQueue::New(Actions, evt_on_char_id)),
  OnKeyDown(AOlxCtrl::ActionQueue::New(Actions, evt_on_key_down_id))
{
  Bind(wxEVT_LEFT_DCLICK, &TTextEdit::ClickEvent, this);
  Bind(wxEVT_TEXT, &TTextEdit::ChangeEvent, this);
  Bind(wxEVT_CHAR, &TTextEdit::CharEvent, this);
  Bind(wxEVT_KEY_DOWN, &TTextEdit::KeyDownEvent, this);
  Bind(wxEVT_TEXT_ENTER, &TTextEdit::EnterPressedEvent, this);
  Bind(wxEVT_KILL_FOCUS, &TTextEdit::LeaveEvent, this);
  Bind(wxEVT_SET_FOCUS, &TTextEdit::EnterEvent, this);
}
//..............................................................................
void TTextEdit::ClickEvent(wxMouseEvent& event)  {
  event.Skip();
  OnClick.Execute(this);
}
//..............................................................................
void TTextEdit::ChangeEvent(wxCommandEvent& event)  {
  event.Skip();
  //StrValue = GetText();
  OnChange.Execute(this);
}
//..............................................................................
void TTextEdit::EnterPressedEvent(wxCommandEvent& event)  {
  event.Skip();
  if (!IsMultiLine()) {
    OnReturn.Execute(this);
  }
}
//..............................................................................
void TTextEdit::KeyDownEvent(wxKeyEvent& event)  {
  event.Skip();
  TKeyEvent evt(event);
  OnKeyDown.Execute(this, &evt);
}
//..............................................................................
void TTextEdit::CharEvent(wxKeyEvent& event)  {
  event.Skip();
  TKeyEvent evt(event);
  OnChar.Execute(this, &evt);
}
//..............................................................................
void TTextEdit::LeaveEvent(wxFocusEvent& event)  {
  event.Skip();
  olxstr v = GetText();
  bool changed = (v != StrValue);
  if( changed )  {
    OnChange.Execute(this);
    StrValue = v;
  }
  OnLeave.Execute(this);
}
//..............................................................................
void TTextEdit::EnterEvent(wxFocusEvent& event)  {
  event.Skip();
  OnEnter.Execute(this);
}
//..............................................................................
