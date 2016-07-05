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

  class TTrackBar: public wxSlider, public AOlxCtrl {
  protected:
    void ScrollEvent(wxCommandEvent& event);
    void MouseUpEvent(wxMouseEvent& event);
    olxstr Data;
    int this_Val;  // needed to call events only if value has changed
  public:
    // on gtk, the size cannot be changed after the creation!
    TTrackBar(wxWindow *Parent, wxWindowID id = -1,
      int value = 0, int min_v = 0, int max_v = 100,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxSL_HORIZONTAL | wxSL_AUTOTICKS);

    DefPropC(olxstr, Data)

    inline int GetValue() const {  return wxSlider::GetValue(); }
    inline void SetValue(int v) { wxSlider::SetValue(v); }
    inline int GetMin() const {  return wxSlider::GetMin(); }
    inline void SetMin(int v) { wxSlider::SetRange(this->GetMax(), v); }
    inline int GetMax() const {  return wxSlider::GetMax(); }
    inline void SetMax(int v) { wxSlider::SetRange(v, this->GetMin()); }

    AOlxCtrl::ActionQueue &OnChange, &OnMouseUp;
  };
}; // end namespace ctrl_ext
#endif
