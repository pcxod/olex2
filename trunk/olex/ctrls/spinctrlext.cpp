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

using namespace ctrl_ext;
IMPLEMENT_CLASS(TSpinCtrl, wxSpinCtrl)

BEGIN_EVENT_TABLE(TSpinCtrl, wxSpinCtrl)
  EVT_TEXT(-1, TSpinCtrl::TextChangeEvent)
  EVT_SPINCTRL(-1, TSpinCtrl::SpinChangeEvent)
  EVT_TEXT_ENTER(-1, TSpinCtrl::EnterPressedEvent)
  EVT_KILL_FOCUS(TSpinCtrl::LeaveEvent)
  EVT_SET_FOCUS(TSpinCtrl::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
void TSpinCtrl::SpinChangeEvent(wxSpinEvent& event)  {
  event.Skip();
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::TextChangeEvent(wxCommandEvent& event)  {
  event.Skip();
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::LeaveEvent(wxFocusEvent& event)  {
  event.Skip();
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
void TSpinCtrl::EnterEvent(wxFocusEvent& event)  {
  event.Skip();
}
//..............................................................................
void TSpinCtrl::EnterPressedEvent(wxCommandEvent& event)  {
  event.Skip();
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  OnChange.Execute(this);
}
//..............................................................................
