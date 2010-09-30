#include "texteditext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TTextEdit, wxTextCtrl)

BEGIN_EVENT_TABLE(TTextEdit, wxTextCtrl)
  EVT_LEFT_DCLICK(TTextEdit::ClickEvent)
  EVT_TEXT(-1, TTextEdit::ChangeEvent)
  EVT_CHAR(TTextEdit::CharEvent)
  EVT_KEY_DOWN(TTextEdit::KeyDownEvent)
  EVT_TEXT_ENTER(-1, TTextEdit::EnterPressedEvent)
END_EVENT_TABLE()
//..............................................................................
void TTextEdit::ClickEvent(wxMouseEvent& event)  {
  StartEvtProcessing()
    OnClick.Execute(this);
    event.Skip();
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::ChangeEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnChange.Execute(this);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::EnterPressedEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnChange.Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
    OnReturn.Execute(this, &TEGC::New<olxstr>(GetOnReturnStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::KeyDownEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnKeyDown.Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::CharEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnChar.Execute(this, &evt);
  EndEvtProcessing()
}
