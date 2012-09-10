/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_color_H
#define __olx_ctrl_color_H
#include "olxctrlbase.h"
#include "wx/clrpicker.h"

namespace ctrl_ext  {

  class TColorCtrl: public wxColourPickerCtrl, public AOlxCtrl  {
  protected:
    void ChangeEvent(wxColourPickerEvent &event);
    olxstr Data;
    wxColor Color;
  public:
    TColorCtrl(wxWindow *Parent, int style=0) :
        wxColourPickerCtrl(Parent, -1, wxColor(), wxDefaultPosition, wxDefaultSize, style),
      AOlxCtrl(this),
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))  {}

    inline bool IsReadOnly() const {   return WI.HasWindowStyle(wxTE_READONLY);  }
    inline void SetReadOnly(bool v)  {  WI.AddWindowStyle(wxTE_READONLY);  }

    void SetValue(wxColor v)  {
      Color = v;
      wxColourPickerCtrl::SetColour(v);
    }
    wxColor GetValue() const {
      return wxColourPickerCtrl::GetColour();
    }

    DefPropC(olxstr, Data)

    AOlxCtrl::ActionQueue &OnChange;

    DECLARE_CLASS(TColorCtrl)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif