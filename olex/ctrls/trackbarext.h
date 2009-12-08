#ifndef __olx_ctrl_trackbar_H
#define __olx_ctrl_trackbar_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class TTrackBar: public wxSlider, public AOlxCtrl  {
  protected:
    void ScrollEvent(wxScrollEvent& event);
    void MouseUpEvent(wxMouseEvent& event);
    olxstr Data, OnChangeStr, OnMouseUpStr;
    int this_Val;  // needed to call events only if value has changed
  public:
    // on gtk, the size cannot be changed after the creation!
    TTrackBar(wxWindow *Parent, const wxSize& sz=wxDefaultSize) : 
      wxSlider(Parent, -1, 0, 0, 100, wxDefaultPosition, sz, wxSL_HORIZONTAL|wxSL_AUTOTICKS),
      AOlxCtrl(this),
      OnChange(Actions.New(evt_change_id)),
      OnMouseUp(Actions.New(evt_on_mouse_up_id)),
      this_Val(0),
      Data(EmptyString),
      OnChangeStr(EmptyString),
      OnMouseUpStr(EmptyString)  {  SetValue(0);  }
    virtual ~TTrackBar()  {}

    DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
    DefPropC(olxstr, OnMouseUpStr) // this is passed to the OnChange event
    DefPropC(olxstr, Data) // data associated with the object

    inline int GetValue() const {  return wxSlider::GetValue(); }
    inline void SetValue(int v) { wxSlider::SetValue(v); }
    inline int GetMin()   const {  return wxSlider::GetMin(); }
    inline void SetMin( int v ) { wxSlider::SetRange(this->GetMax(), v); }
    inline int GetMax()   const {  return wxSlider::GetMax(); }
    inline void SetMax( int v ) { wxSlider::SetRange(v, this->GetMin()); }

    TActionQueue &OnChange, &OnMouseUp;

    DECLARE_CLASS(TTrackBar)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
