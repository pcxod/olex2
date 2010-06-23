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
    olxstr Data, OnChangeStr;
  public:
    TSpinCtrl(wxWindow *Parent, const wxSize& sz=wxDefaultSize): 
      wxSpinCtrl(Parent, -1, wxEmptyString, wxDefaultPosition, sz),
      AOlxCtrl(this),  
      OnChange(Actions.New(evt_change_id)),
      Data(EmptyString),
      OnChangeStr(EmptyString)
    {
    }
    virtual ~TSpinCtrl()  {}

    DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
    DefPropC(olxstr, Data) // data associated with the object

    int GetValue() const {  return wxSpinCtrl::GetValue(); }
    void SetValue(int v) { 
      Value = v;
      wxSpinCtrl::SetValue(v); 
    }

    TActionQueue &OnChange;

    DECLARE_CLASS(TSpinCtrl)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
