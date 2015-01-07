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
IMPLEMENT_CLASS(TTextEdit, wxTextCtrl)

BEGIN_EVENT_TABLE(TTextEdit, wxTextCtrl)
  EVT_LEFT_DCLICK(TTextEdit::ClickEvent)
  EVT_TEXT(-1, TTextEdit::ChangeEvent)
  EVT_CHAR(TTextEdit::CharEvent)
  EVT_KEY_DOWN(TTextEdit::KeyDownEvent)
  EVT_TEXT_ENTER(-1, TTextEdit::EnterPressedEvent)
  EVT_KILL_FOCUS(TTextEdit::LeaveEvent)
  EVT_SET_FOCUS(TTextEdit::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
void TTextEdit::ClickEvent(wxMouseEvent& event)  {
  OnClick.Execute(this);
  event.Skip();
}
//..............................................................................
void TTextEdit::ChangeEvent(wxCommandEvent& event)  {
  //StrValue = GetText();
  OnChange.Execute(this);
  event.Skip();
}
//..............................................................................
void TTextEdit::EnterPressedEvent(wxCommandEvent& event)  {
  if (!IsMultiLine()) {
    OnReturn.Execute(this);
  }
  event.Skip();
}
//..............................................................................
void TTextEdit::KeyDownEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  OnKeyDown.Execute(this, &evt);
  event.Skip();
}
//..............................................................................
void TTextEdit::CharEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  OnChar.Execute(this, &evt);
  event.Skip();
}
//..............................................................................
void TTextEdit::LeaveEvent(wxFocusEvent& event)  {
  olxstr v = GetText();
  bool changed = (v != StrValue);
  if( changed )  {
    OnChange.Execute(this);
    StrValue = v;
  }
  OnLeave.Execute(this);
  event.Skip();
}
//..............................................................................
void TTextEdit::EnterEvent(wxFocusEvent& event)  {
  OnEnter.Execute(this);
  event.Skip();
}
//..............................................................................
