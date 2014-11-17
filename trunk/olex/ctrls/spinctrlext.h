/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_spin_H
#define __olx_ctrl_spin_H
#include "olxctrlbase.h"
#include "wx/spinctrl.h"

namespace ctrl_ext  {
  class TSpinCtrl: public wxSpinCtrl, public AOlxCtrl {
  private:
    int Value;
  protected:
    void SpinChangeEvent(wxSpinEvent& event);
    void TextChangeEvent(wxCommandEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    void EnterEvent(wxFocusEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    olxstr Data;
  public:
    TSpinCtrl(wxWindow *Parent, wxWindowID id = -1,
      const wxString &value = wxEmptyString,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxSP_ARROW_KEYS | wxALIGN_RIGHT);

    DefPropC(olxstr, Data)

    int GetValue() const {  return wxSpinCtrl::GetValue(); }
    void SetValue(int v) {
      Value = v;
      wxSpinCtrl::SetValue(v);
    }

    AOlxCtrl::ActionQueue &OnChange;
  };
}; // end namespace ctrl_ext
#endif
