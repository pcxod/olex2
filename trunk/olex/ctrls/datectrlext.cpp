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
IMPLEMENT_CLASS(TDateCtrl, wxDatePickerCtrl)

BEGIN_EVENT_TABLE(TDateCtrl, wxDatePickerCtrl)
  EVT_DATE_CHANGED(-1, TDateCtrl::ChangeEvent)
END_EVENT_TABLE()
//..............................................................................
void TDateCtrl::ChangeEvent(wxDateEvent& event)  {
  event.Skip();
  if( event.GetDate() == Value )
    return;
  Value = event.GetDate();
  OnChange.Execute(this);
}
//..............................................................................
