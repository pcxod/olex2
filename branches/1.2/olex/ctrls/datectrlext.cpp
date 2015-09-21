/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "datectrlext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
TDateCtrl::TDateCtrl(wxWindow *Parent, wxWindowID id, const wxDateTime& value,
const wxPoint& pos, const wxSize& size, long style)
: wxDatePickerCtrl(Parent, id, value, pos, size, style),
  AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))
{
  Value = GetValue();
  Bind(wxEVT_DATE_CHANGED, &TDateCtrl::ChangeEvent, this);
}
//..............................................................................
void TDateCtrl::ChangeEvent(wxDateEvent& event) {
  event.Skip();
  if (event.GetDate() == Value)
    return;
  Value = event.GetDate();
  OnChange.Execute(this);
}
//..............................................................................
