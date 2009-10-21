#include "labelext.h"
#include "frameext.h"
#include "egc.h"
using namespace ctrl_ext;
IMPLEMENT_CLASS(TLabel, wxStaticText)

BEGIN_EVENT_TABLE(TLabel, wxStaticText)
  EVT_COMMAND(-1, wxEVT_LEFT_DOWN, TLabel::ClickEvent)
END_EVENT_TABLE()
//..............................................................................
void TLabel::ClickEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnClick.Execute(this, &TEGC::New<olxstr>( GetOnClickStr() ));
  EndEvtProcessing()
}

