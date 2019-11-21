/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "colorext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;

//..............................................................................
TColorCtrl::TColorCtrl(wxWindow *Parent, wxWindowID id, const wxColor &value,
  const wxPoint& pos, const wxSize& size, long style)
: wxColourPickerCtrl(Parent, id, value, pos, size, style),
  AOlxCtrl(this),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))
{
  Bind(wxEVT_COLOURPICKER_CHANGED, &TColorCtrl::ChangeEvent, this);
}
//..............................................................................
void TColorCtrl::ChangeEvent(wxColourPickerEvent& event) {
  event.Skip();
  wxColor c = GetColour();
  if (Color == c) {
    return;
  }
  Color = c;
  OnChange.Execute(this);
}
//..............................................................................
