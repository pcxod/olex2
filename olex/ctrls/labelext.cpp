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
//..............................................................................
void TLabel::ClickEvent(wxMouseEvent& event)  {
  event.Skip();
  OnClick.Execute(this);
}
void TLabel::OnPaint(wxPaintEvent& event) {
#ifdef _WIN32
  wxPaintDC dc(this);
  dc.SetBrush(GetBackgroundColour());
  dc.SetPen(GetForegroundColour());
  //dc.Clear();
  wxStaticText::OnPaint(event);
#else
  event.Skip(true);
#endif
}
//..............................................................................
void TLabel::onEraseBG(wxEraseEvent& event) {
#ifdef _WIN32
  event.Skip();
#else
  event.Skip(true);
#endif
}
