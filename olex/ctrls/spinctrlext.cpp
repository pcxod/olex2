#include "spinctrlext.h"
#include "../xglapp.h"
#include "../mainform.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TSpinCtrl, wxSpinCtrl)

BEGIN_EVENT_TABLE(TSpinCtrl, wxSpinCtrl)
  EVT_TEXT(101, TSpinCtrl::TextChangeEvent)
  EVT_SPIN(102, TSpinCtrl::SpinChangeEvent)
  EVT_TEXT_ENTER(103, TSpinCtrl::EnterPressedEvent)
  EVT_KILL_FOCUS(TSpinCtrl::LeaveEvent)
  EVT_SET_FOCUS(TSpinCtrl::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
void TSpinCtrl::SpinChangeEvent(wxSpinEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
    OnChange.Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
void TSpinCtrl::TextChangeEvent(wxCommandEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetValue());
  StartEvtProcessing()
   OnChange.Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
void TSpinCtrl::LeaveEvent(wxFocusEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange.Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
void TSpinCtrl::EnterEvent(wxFocusEvent& event)  {}
void TSpinCtrl::EnterPressedEvent(wxCommandEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange.Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
