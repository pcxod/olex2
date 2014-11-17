/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "listboxext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
TListBox::TListBox(wxWindow *Parent, wxWindowID id,
  const wxPoint& pos, const wxSize& size, long style)
: AOlxCtrl(this),
  OnSelect(AOlxCtrl::ActionQueue::New(Actions, evt_on_select_id)),
  OnDblClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_dbl_click_id))
{
  wxListBox::Create(Parent, id, pos, size, 0, 0, style);
  Bind(wxEVT_LEFT_DCLICK, &TListBox::DblClickEvent, this);
  Bind(wxEVT_LISTBOX, &TListBox::ItemSelectEvent, this);
}
void TListBox::DblClickEvent(wxMouseEvent& event) {
  event.Skip();
  OnDblClick.Execute(this);
}
//..............................................................................
void TListBox::ItemSelectEvent(wxCommandEvent& event) {
  event.Skip();
  if (!Data.IsEmpty())
    TOlxVars::SetVar(Data, GetValue());
  OnSelect.Execute(this);
}
//..............................................................................

