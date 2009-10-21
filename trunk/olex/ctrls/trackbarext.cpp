#include "trackbarext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TTrackBar, wxSlider)

BEGIN_EVENT_TABLE(TTrackBar, wxSlider)
  EVT_SCROLL(TTrackBar::ScrollEvent)
  EVT_LEFT_UP(TTrackBar::MouseUpEvent)
END_EVENT_TABLE()

void TTrackBar::ScrollEvent(wxScrollEvent& evt)  {
  if( this_Val == GetValue() )  return;
  this_Val = GetValue();
  if( !Data.IsEmpty() )
    TOlxVars::SetVar(Data, this_Val);
  StartEvtProcessing()
    OnChange.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTrackBar::MouseUpEvent(wxMouseEvent& evt)  {
  evt.Skip();
  StartEvtProcessing()
    OnMouseUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnMouseUpStr()));
  EndEvtProcessing()
}
