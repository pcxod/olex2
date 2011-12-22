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
  StartEvtProcessing()
    OnClick.Execute(this);
    event.Skip();
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::ChangeEvent(wxCommandEvent& event)  {
  //StrValue = GetText();
  StartEvtProcessing()
    OnChange.Execute(this);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::EnterPressedEvent(wxCommandEvent& event)  {
  if (IsMultiLine()) {
    AppendText(wxT("\n"));
  }
  else {
    StartEvtProcessing()
      OnReturn.Execute(this);
    EndEvtProcessing()
  }
}
//..............................................................................
void TTextEdit::KeyDownEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnKeyDown.Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::CharEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnChar.Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::LeaveEvent(wxFocusEvent& event)  {
  olxstr v = GetText();
  bool changed = (v != StrValue);
  StartEvtProcessing()
    if( changed )  {
      OnChange.Execute(this);
      StrValue = v;
    }
    OnLeave.Execute(this);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::EnterEvent(wxFocusEvent& event)  {
  StartEvtProcessing()
    OnEnter.Execute(this);
  EndEvtProcessing()
}
//..............................................................................
