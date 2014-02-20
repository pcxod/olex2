/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_datetime_H
#define __olx_ctrl_datetime_H
#include "olxctrlbase.h"
#include "wx/datectrl.h"
#include "wx/dateevt.h"

namespace ctrl_ext  {

  class TDateCtrl: public wxDatePickerCtrl, public AOlxCtrl  {
  protected:
    void ChangeEvent(wxDateEvent& event);
    olxstr Data;
    wxDateTime Value;
  public:
    TDateCtrl(wxWindow *Parent, int style=0) :
        wxDatePickerCtrl(Parent, -1, wxDateTime::Now(), wxDefaultPosition, wxDefaultSize, style),
      AOlxCtrl(this),
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))
    {
      Value = GetValue();
    }

    void SetValue(const wxDateTime &T)  {
      Value = T;
      wxDatePickerCtrl::SetValue(T);
    }

    DefPropC(olxstr, Data)

    AOlxCtrl::ActionQueue &OnChange;

    DECLARE_CLASS(TDateTime)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
