/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "comboext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
TComboBox::TComboBox(wxWindow *Parent, wxWindowID id, const wxString &value,
const wxPoint& pos, const wxSize& size, long style)
: AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
  OnLeave(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_leave_id)),
  OnEnter(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_enter_id)),
  OnReturn(AOlxCtrl::ActionQueue::New(Actions, evt_on_return_id))
{
  wxComboBox::Create(Parent, id, value, pos, size, 0, NULL, style);
  Bind(wxEVT_COMBOBOX, &TComboBox::ChangeEvent, this);
  Bind(wxEVT_TEXT_ENTER, &TComboBox::EnterPressedEvent, this);
  Bind(wxEVT_KILL_FOCUS, &TComboBox::LeaveEvent, this);
  Bind(wxEVT_SET_FOCUS, &TComboBox::EnterEvent, this);
  OnLeave.SetEnabled(false);
  entered_counter = 0;
  OnChangeAlways = false;
}
//..............................................................................
TComboBox::~TComboBox() {
  _Clear();
}
//..............................................................................
void TComboBox::SetText(const olxstr& T) {
  olx_pair_t<size_t, olxstr> found = _SetText(T);
  if (!IsReadOnly() || found.a != InvalidIndex) {
    StrValue = found.b;
    SetValue(StrValue.u_str());
  }
  else if (!T.IsEmpty()) {
    wxComboBox::SetSelection(wxNOT_FOUND);
    StrValue = EmptyString();
  }
}
//..............................................................................
void TComboBox::Clear() {
  StrValue.SetLength(0);
  if (GetCount() == 0) return;
  _Clear();
#if defined(__MAC__) && wxCHECK_VERSION(2,9,0)
  wxComboBox::DoClear();
#else
  wxComboBox::Clear();
#endif
  wxComboBox::SetSelection(wxNOT_FOUND);
}
//..............................................................................
void TComboBox::EnterPressedEvent(wxCommandEvent &event)  {
  if( !Data.IsEmpty() )
    TOlxVars::SetVar(Data, GetText());
  OnReturn.Execute(this);
  event.Skip();
}
//..............................................................................
void TComboBox::ChangeEvent(wxCommandEvent& event) {
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
void TComboBox::LeaveEvent(wxFocusEvent& event)  {
  if (--entered_counter == 0)
    HandleOnLeave();
  event.Skip();
}
//..............................................................................
void TComboBox::EnterEvent(wxFocusEvent& event)  {
  if (++entered_counter == 1)
    HandleOnEnter();
  event.Skip();
}
//..............................................................................
olxstr TComboBox::GetText() const {
  if (GetSelection() == -1) {
    return IsReadOnly() ? EmptyString() : olxstr(wxComboBox::GetValue());
  }
  olx_pair_t<bool, olxstr> v = _GetText(GetSelection());
  return (v.a ? v.b : olxstr(GetValue()));
}
//..............................................................................
void TComboBox::HandleOnLeave() {
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
void TComboBox::HandleOnEnter()  {
  if (OnEnter.IsEnabled()) {
    OnEnter.Execute(this);
    OnEnter.SetEnabled(false);
    OnLeave.SetEnabled(true);
  }
}
//..............................................................................
