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
  class TSpinCtrl: public wxSpinCtrl, public AOlxCtrl  {
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
    TSpinCtrl(wxWindow *Parent, const wxSize& sz=wxDefaultSize): 
      wxSpinCtrl(Parent, -1, wxEmptyString, wxDefaultPosition, sz),
      AOlxCtrl(this),  
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))  {}

    DefPropC(olxstr, Data)

    int GetValue() const {  return wxSpinCtrl::GetValue(); }
    void SetValue(int v) { 
      Value = v;
      wxSpinCtrl::SetValue(v); 
    }

    AOlxCtrl::ActionQueue &OnChange;

    DECLARE_CLASS(TSpinCtrl)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
