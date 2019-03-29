/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "spinctrlext.h"
#include "frameext.h"
#include "olxvar.h"
#include "wx/renderer.h"

using namespace ctrl_ext;
//..............................................................................
TSpinCtrl::TSpinCtrl(wxWindow *Parent, wxWindowID id, const wxString &value,
  const wxPoint& pos, const wxSize& size, long style)
: wxSpinCtrl(Parent, id, value, pos, size, style),
  AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))
{
  Bind(wxEVT_TEXT, &TSpinCtrl::TextChangeEvent, this);
  Bind(wxEVT_SPINCTRL, &TSpinCtrl::SpinChangeEvent, this);
  Bind(wxEVT_TEXT_ENTER, &TSpinCtrl::EnterPressedEvent, this);
  Bind(wxEVT_KILL_FOCUS, &TSpinCtrl::LeaveEvent, this);
  Bind(wxEVT_SET_FOCUS, &TSpinCtrl::EnterEvent, this);
  Bind(wxEVT_PAINT, &TSpinCtrl::PaintEvent, this);
}
//..............................................................................
void TSpinCtrl::SpinChangeEvent(wxSpinEvent& event) {
  event.Skip();
  int val = GetValue();
  if (val == Value) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::TextChangeEvent(wxCommandEvent& event) {
  event.Skip();
  int val = GetValue();
  if (val == Value) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::LeaveEvent(wxFocusEvent& event) {
  event.Skip();
  int val = GetValue();
  if (val == Value) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::EnterEvent(wxFocusEvent& event) {
  event.Skip();
}
//..............................................................................
void TSpinCtrl::EnterPressedEvent(wxCommandEvent& event) {
  event.Skip();
  int val = GetValue();
  if (val == Value) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::PaintEvent(wxPaintEvent& event) {
  event.Skip(true);
}
//..............................................................................
