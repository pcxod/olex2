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

namespace ctrl_ext {

  class TDateCtrl: public wxDatePickerCtrl, public AOlxCtrl {
  protected:
    void ChangeEvent(wxDateEvent& event);
    olxstr Data;
    wxDateTime Value;
  public:
    TDateCtrl(wxWindow *Parent, wxWindowID id = -1,
      const wxDateTime& value = wxDateTime::Now(),
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0);

    void SetValue(const wxDateTime &T)  {
      Value = T;
      wxDatePickerCtrl::SetValue(T);
    }

    DefPropC(olxstr, Data)

    AOlxCtrl::ActionQueue &OnChange;
  };
}; // end namespace ctrl_ext
#endif
