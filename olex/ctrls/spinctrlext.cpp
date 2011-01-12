#include "spinctrlext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TSpinCtrl, wxSpinCtrl)

BEGIN_EVENT_TABLE(TSpinCtrl, wxSpinCtrl)
  EVT_TEXT(-1, TSpinCtrl::TextChangeEvent)
  EVT_SPINCTRL(-1, TSpinCtrl::SpinChangeEvent)
  EVT_TEXT_ENTER(-1, TSpinCtrl::EnterPressedEvent)
  EVT_KILL_FOCUS(TSpinCtrl::LeaveEvent)
  EVT_SET_FOCUS(TSpinCtrl::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
void TSpinCtrl::SpinChangeEvent(wxSpinEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
    OnChange.Execute(this, &GetOnChangeStr());
  EndEvtProcessing()
}
//..............................................................................
void TSpinCtrl::TextChangeEvent(wxCommandEvent& event)  {
    int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange.Execute(this, &GetOnChangeStr());
  EndEvtProcessing()
}
//..............................................................................
void TSpinCtrl::LeaveEvent(wxFocusEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange.Execute(this, &GetOnChangeStr());
  EndEvtProcessing()
}
//..............................................................................
void TSpinCtrl::EnterEvent(wxFocusEvent& event)  {}
//..............................................................................
void TSpinCtrl::EnterPressedEvent(wxCommandEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange.Execute(this, &GetOnChangeStr());
  EndEvtProcessing()
}
//..............................................................................
