/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "trackbarext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
TTrackBar::TTrackBar(wxWindow *Parent, wxWindowID id,
  int value, int min_v, int max_v,
  const wxPoint& pos, const wxSize& size, long style)
: wxSlider(Parent, id, value, min_v, max_v, pos, size, style),
  AOlxCtrl(this),
  this_Val(value),
  OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
  OnMouseUp(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_up_id))
{
  Bind(wxEVT_SLIDER, &TTrackBar::ScrollEvent, this);
  Bind(wxEVT_LEFT_UP, &TTrackBar::MouseUpEvent, this);
}
//..............................................................................
void TTrackBar::ScrollEvent(wxCommandEvent& evt) {
  evt.Skip();
  if (this_Val == GetValue())  return;
  this_Val = GetValue();
  OnChange.Execute((AOlxCtrl*)this);
}
//..............................................................................
void TTrackBar::MouseUpEvent(wxMouseEvent& evt) {
  evt.Skip();
  OnMouseUp.Execute((AOlxCtrl*)this);
}
