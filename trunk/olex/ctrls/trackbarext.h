/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_trackbar_H
#define __olx_ctrl_trackbar_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class TTrackBar: public wxSlider, public AOlxCtrl  {
  protected:
    void ScrollEvent(wxScrollEvent& event);
    void MouseUpEvent(wxMouseEvent& event);
    olxstr Data;
    int this_Val;  // needed to call events only if value has changed
  public:
    // on gtk, the size cannot be changed after the creation!
    TTrackBar(wxWindow *Parent, const wxSize& sz=wxDefaultSize) :
      wxSlider(Parent, -1, 0, 0, 100, wxDefaultPosition, sz, wxSL_HORIZONTAL|wxSL_AUTOTICKS),
      AOlxCtrl(this),
      OnChange(AOlxCtrl::ActionQueue::New(Actions, evt_change_id)),
      OnMouseUp(AOlxCtrl::ActionQueue::New(Actions, evt_on_mouse_up_id)),
      this_Val(0)  {  SetValue(0);  }

    DefPropC(olxstr, Data)

    inline int GetValue() const {  return wxSlider::GetValue(); }
    inline void SetValue(int v) { wxSlider::SetValue(v); }
    inline int GetMin() const {  return wxSlider::GetMin(); }
    inline void SetMin(int v) { wxSlider::SetRange(this->GetMax(), v); }
    inline int GetMax() const {  return wxSlider::GetMax(); }
    inline void SetMax(int v) { wxSlider::SetRange(v, this->GetMin()); }

    AOlxCtrl::ActionQueue &OnChange, &OnMouseUp;

    DECLARE_CLASS(TTrackBar)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
