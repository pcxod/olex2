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
IMPLEMENT_CLASS(TListBox, wxListBox)

BEGIN_EVENT_TABLE(TListBox, wxListBox)
  EVT_LEFT_DCLICK(TListBox::DblClickEvent)
  EVT_LISTBOX(-1, TListBox::ItemSelectEvent)
END_EVENT_TABLE()
//..............................................................................
void TListBox::DblClickEvent(wxMouseEvent& event)  {
  event.Skip();
  OnDblClick.Execute(this);
}
//..............................................................................
void TListBox::ItemSelectEvent(wxCommandEvent& event)  {
  event.Skip();
  if( !Data.IsEmpty() )
    TOlxVars::SetVar(Data, GetValue());
  OnSelect.Execute(this);
}
//..............................................................................

