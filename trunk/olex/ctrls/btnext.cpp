#include "btnext.h"
#include "frameext.h"
#include "../obase.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TButton, wxButton)
IMPLEMENT_CLASS(TBmpButton, wxBitmapButton)

//..............................................................................
void AButtonBase::SetActionQueue(TActionQueue* q, const olxstr& dependMode)  {
  ActionQueue = q;
  DependMode = dependMode;
  ActionQueue->Add( this );
}
bool AButtonBase::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    SetDown( mc->CheckStatus(DependMode) );
  }
  return true;
}
//..............................................................................
void AButtonBase::SetDown(bool v)  {
  if( Down == v )  return;
  StartEvtProcessing()
  if( Down )  {
    Down = false;
    if( !GetOnUpStr().IsEmpty() )
      OnUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnUpStr()));
  }
  else  {
    Down = true;
    if( !GetOnDownStr().IsEmpty() )
      OnDown.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnDownStr()));
  }
  EndEvtProcessing()
}
//..............................................................................
void AButtonBase::ClickEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnClick.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( Down )  {
      Down = false;
      if( !GetOnUpStr().IsEmpty() )
        OnUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnUpStr() ));
    }
    else  {
      Down = true;
      if( !GetOnDownStr().IsEmpty() )
        OnDown.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnDownStr() ));
    }
  EndEvtProcessing()
}
//..............................................................................
BEGIN_EVENT_TABLE(TButton, wxButton)
  EVT_BUTTON(-1, TButton::ClickEvent)
  EVT_ENTER_WINDOW(TButton::MouseEnterEvent)
  EVT_LEAVE_WINDOW(TButton::MouseLeaveEvent)
END_EVENT_TABLE()

//..............................................................................
void TButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor( wxCursor(wxCURSOR_HAND) );
  if( !GetHint().IsEmpty() )
    SetToolTip( GetHint().u_str() );
  event.Skip();
}
//..............................................................................
void TButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  event.Skip();
}
//..............................................................................
BEGIN_EVENT_TABLE(TBmpButton, wxBitmapButton)
  EVT_BUTTON(-1, TBmpButton::ClickEvent)
  EVT_ENTER_WINDOW(TBmpButton::MouseEnterEvent)
  EVT_LEAVE_WINDOW(TBmpButton::MouseLeaveEvent)
END_EVENT_TABLE()
//..............................................................................
void TBmpButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor( wxCursor(wxCURSOR_HAND) );
  event.Skip();
}
//..............................................................................
void TBmpButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  event.Skip();
}
