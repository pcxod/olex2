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
IMPLEMENT_CLASS(TComboBox, wxComboBox)

BEGIN_EVENT_TABLE(TComboBox, wxComboBox)
  EVT_COMBOBOX(-1, TComboBox::ChangeEvent)
  EVT_TEXT_ENTER(-1, TComboBox::EnterPressedEvent)
  EVT_KILL_FOCUS(TComboBox::LeaveEvent)
  EVT_SET_FOCUS(TComboBox::EnterEvent)
END_EVENT_TABLE()

TComboBox::~TComboBox() {
  _Clear();
}
//..............................................................................
void TComboBox::SetText(const olxstr& T) {
  olx_pair_t<size_t, olxstr> found = _SetText(T);
  if (!IsReadOnly() || found.a != InvalidIndex) {
    StrValue = found.b;
    SetValue(StrValue.u_str());
    if (found.a == InvalidIndex) {
      selection_index = -1;
    }
    else {
      selection_index = (int)found.a;
#ifndef __WIN32__
      wxComboBox::SetSelection(wxNOT_FOUND);
#endif
    }
  }
  else if (!T.IsEmpty()) {
    wxComboBox::SetSelection(wxNOT_FOUND);
    StrValue = EmptyString();
    selection_index = -1;
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
  selection_index = -1;
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
  selection_index = wxComboBox::GetSelection();
  olxstr v = GetValue();
  if (IsOnChangeAlways() || v != StrValue) {
    StrValue = v;
    if (!Data.IsEmpty())
      TOlxVars::SetVar(Data, GetText());
    OnChange.Execute(this);
  }
#ifndef __WIN32__
  wxComboBox::SetSelection(wxNOT_FOUND);
#endif
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
  if (!HasValue())
    return EmptyString();
  if (selection_index == -1) {
    return IsReadOnly() ? EmptyString() : olxstr(wxComboBox::GetValue());
  }
  olx_pair_t<bool, olxstr> v = _GetText(selection_index);
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
