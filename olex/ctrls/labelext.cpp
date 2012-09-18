/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "labelext.h"
#include "frameext.h"
#include "egc.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TLabel, wxStaticText)

BEGIN_EVENT_TABLE(TLabel, wxStaticText)
  EVT_COMMAND(-1, wxEVT_LEFT_DOWN, TLabel::ClickEvent)
END_EVENT_TABLE()
//..............................................................................
void TLabel::ClickEvent(wxCommandEvent& event)  {
  event.Skip();
  OnClick.Execute(this);
}
