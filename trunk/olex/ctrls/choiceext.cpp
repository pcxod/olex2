/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "choiceext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TChoice, wxChoice)
BEGIN_EVENT_TABLE(TChoice, wxChoice)
EVT_COMBOBOX(-1, TChoice::ChangeEvent)
EVT_KILL_FOCUS(TChoice::LeaveEvent)
EVT_SET_FOCUS(TChoice::EnterEvent)
END_EVENT_TABLE()

TChoice::~TChoice() {
  _Clear();
}
//..............................................................................
void TChoice::SetText(const olxstr& T) {
  olx_pair_t<size_t, olxstr> found = _SetText(T);
  if (found.a != InvalidIndex) {
    StrValue = found.b;
    wxChoice::SetSelection(found.a);
  }
  else if (!T.IsEmpty()) {
    wxChoice::SetSelection(wxNOT_FOUND);
    StrValue = EmptyString();
  }
}
//..............................................................................
void TChoice::Clear() {
  StrValue.SetLength(0);
  if (GetCount() == 0) return;
  _Clear();
  wxChoice::Clear();
}
//..............................................................................
void TChoice::ChangeEvent(wxCommandEvent& event) {
  olxstr v = GetValue();
  if (IsOnChangeAlways() || v != StrValue) {
    StrValue = v;
    if (!Data.IsEmpty())
      TOlxVars::SetVar(Data, GetText());
    OnChange.Execute(this);
  }
  event.Skip();
}
//..............................................................................
void TChoice::LeaveEvent(wxFocusEvent& event)  {
  if (--entered_counter == 0)
    HandleOnLeave();
  event.Skip();
}
//..............................................................................
void TChoice::EnterEvent(wxFocusEvent& event)  {
  if (++entered_counter == 1)
    HandleOnEnter();
  event.Skip();
}
//..............................................................................
olxstr TChoice::GetText() const {
  int sel = wxChoice::GetSelection();
  if (sel == wxNOT_FOUND) {
    return EmptyString();
  }
  olx_pair_t<bool, olxstr> v = _GetText(sel);
  return (v.a ? v.b : olxstr(GetValue()));
}
//..............................................................................
void TChoice::HandleOnLeave() {
  if (OnLeave.IsEnabled()) {
    olxstr v = GetValue();
    bool changed = (v != StrValue);
    if (changed) {
      StrValue = v;
      OnChange.Execute(this);
    }
    OnLeave.Execute(this);
    OnLeave.SetEnabled(false);
    OnEnter.SetEnabled(true);
  }
}
//..............................................................................
void TChoice::HandleOnEnter()  {
  if (OnEnter.IsEnabled()) {
    OnEnter.Execute(this);
    OnEnter.SetEnabled(false);
    OnLeave.SetEnabled(true);
  }
}
//..............................................................................
