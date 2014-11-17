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
//..............................................................................
TChoice::TChoice(wxWindow *Parent, wxWindowID id, const wxPoint& pos,
  const wxSize& size, long style)
: AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
  OnLeave(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_leave_id)),
  OnEnter(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_enter_id))
{
  wxChoice::Create(Parent, id, pos, size, 0, 0, style);
  Bind(wxEVT_CHOICE, &TChoice::ChangeEvent, this);
  Bind(wxEVT_KILL_FOCUS, &TChoice::LeaveEvent, this);
  Bind(wxEVT_SET_FOCUS, &TChoice::EnterEvent, this);
    OnLeave.SetEnabled(false);
  entered_counter = 0;
  OnChangeAlways = false;
}
//..............................................................................
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
  else {
    if (HasDefault() && !IsEmpty()) {
      wxChoice::SetSelection(0);
      StrValue = GetText();
    }
    else {
      wxChoice::SetSelection(wxNOT_FOUND);
      StrValue = EmptyString();
    }
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
