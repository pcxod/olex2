#include "checkboxext.h"
#include "frameext.h"
#include "../obase.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TCheckBox, wxCheckBox)

BEGIN_EVENT_TABLE(TCheckBox, wxCheckBox)
  EVT_CHECKBOX(-1, TCheckBox::ClickEvent)
  EVT_ENTER_WINDOW(TCheckBox::MouseEnterEvent)
END_EVENT_TABLE()

//..............................................................................
void TCheckBox::MouseEnterEvent(wxMouseEvent& event)  {
  event.Skip();
  SetCursor( wxCursor(wxCURSOR_HAND) );
}
//..............................................................................
void TCheckBox::SetActionQueue(TActionQueue& q, const olxstr& dependMode)  {
  ActionQueue = &q;
  DependMode = dependMode;
  ActionQueue->Add(this);
}
bool TCheckBox::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    SetChecked( mc->CheckStatus(DependMode) );
  }
  StartEvtProcessing()
    OnClick.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( IsChecked() )
      OnCheck.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnCheckStr()) );
    else
      OnUncheck.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnUncheckStr()) );
  EndEvtProcessing()

  return true;
}
//..............................................................................
void TCheckBox::ClickEvent(wxCommandEvent &event)  {
  StartEvtProcessing()
    OnClick.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( IsChecked() )
      OnCheck.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnCheckStr()) );
    else
      OnUncheck.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnUncheckStr()) );
  EndEvtProcessing()
}

