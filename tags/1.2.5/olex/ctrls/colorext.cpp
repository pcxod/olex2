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
IMPLEMENT_CLASS(TColorCtrl, wxColourPickerCtrl)

BEGIN_EVENT_TABLE(TColorCtrl, wxColourPickerCtrl)
  EVT_COLOURPICKER_CHANGED(-1, TColorCtrl::ChangeEvent)
END_EVENT_TABLE()
//..............................................................................
void TColorCtrl::ChangeEvent(wxColourPickerEvent& event)  {
  event.Skip();
  wxColor c = GetColour();
  if( Color == c ) return;
  Color = c;
  OnChange.Execute(this);
}
//..............................................................................
