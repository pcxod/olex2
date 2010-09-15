#include "btnext.h"
#include "frameext.h"
#include "../obase.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TButton, wxButton)
IMPLEMENT_CLASS(TBmpButton, wxBitmapButton)

//..............................................................................
void AButtonBase::SetActionQueue(TActionQueue& q, const olxstr& dependMode)  {
  ActionQueue = &q;
  DependMode = dependMode;
  ActionQueue->Add(this);
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
void AButtonBase::ClickEvent()  {
  StartEvtProcessing()
    if( Down )  {
      Down = false;
      if( !GetOnUpStr().IsEmpty() )
        OnUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnUpStr() ));
    }
    if( !GetOnClickStr().IsEmpty() )
      OnClick.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( !Down )  {
      Down = true;
      if( !GetOnDownStr().IsEmpty() )
        OnDown.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnDownStr() ));
    }
  EndEvtProcessing()
}
//..............................................................................
//..............................................................................
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
    SetToolTip(GetHint().u_str());
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
  SetCursor(wxCursor(wxCURSOR_HAND));
  if( !GetHint().IsEmpty() )
    SetToolTip(GetHint().u_str());
  event.Skip();
}
//..............................................................................
void TBmpButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  event.Skip();
}
//..............................................................................
//..............................................................................
//..............................................................................
BEGIN_EVENT_TABLE(TImgButton, wxPanel)
  EVT_LEFT_DOWN(TImgButton::MouseDownEvent)
  EVT_LEFT_UP(TImgButton::MouseUpEvent)
  EVT_MOTION(TImgButton::MouseMoveEvent)
  EVT_ENTER_WINDOW(TImgButton::MouseEnterEvent)
  EVT_LEAVE_WINDOW(TImgButton::MouseLeaveEvent)
  EVT_PAINT(TImgButton::PaintEvent)
END_EVENT_TABLE()
//..............................................................................
TImgButton::TImgButton(wxWindow* parent) : wxPanel(parent), AButtonBase(this) {
  state = 0;
  ProcessingOnDown = MouseIn = false;
}
//..............................................................................
void TImgButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor(wxCursor(wxCURSOR_HAND));
  if( !GetHint().IsEmpty() )
    SetToolTip(GetHint().u_str());
  MouseIn = true;
  if( state != stDisabled && state != stDown )  {
    state = stHover;
    Paint();
  }
  event.Skip();
}
//..............................................................................
void TImgButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  MouseIn = false;
  if( state != stDisabled && state != stUp && !ProcessingOnDown )  {
    state = stUp;
    Paint();
  }
  event.Skip();
}
//..............................................................................
const wxBitmap& TImgButton::ChooseBitmap() const {
  switch( state )  {
  case stUp:        return bmpUp;
  case stDown:      return bmpDown;
  case stDisabled:  return bmpDisabled;
  case stHover:     return (bmpHover.IsOk() ? bmpHover : bmpUp);
  default:          return bmpUp;
  }
}
//..............................................................................
void TImgButton::Render(wxDC& dc) const {
  const wxBitmap& bmp = ChooseBitmap();
  if( bmp.IsOk() )
    dc.DrawBitmap(bmp, 0, 0);
}
//..............................................................................
void TImgButton::MouseDownEvent(wxMouseEvent& event)  {
  if( state != stDisabled && state != stDown )  {
    state = stDown;
    Paint();
    ProcessingOnDown = true;
    StartEvtProcessing()
      if( !GetOnDownStr().IsEmpty() )
        OnDown.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnDownStr() ));
    if( !MouseIn )  {
      if( !GetOnUpStr().IsEmpty() )
        OnUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnUpStr() ));
      state = stUp;
      Paint();
    }
    EndEvtProcessing()
    ProcessingOnDown = false;
  }
}
//..............................................................................
void TImgButton::MouseUpEvent(wxMouseEvent& event)  {
  if( state != stDisabled && state != stUp && state != stHover )  {
    StartEvtProcessing()
      if( !GetOnUpStr().IsEmpty() )
        OnUp.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>( GetOnUpStr() ));
      if( !GetOnClickStr().IsEmpty() )
        OnClick.Execute((AOlxCtrl*)this, &TEGC::New<olxstr>(GetOnClickStr()) );
    EndEvtProcessing()
    state = stUp;
    Paint();
  }
}
//..............................................................................
void TImgButton::MouseMoveEvent(wxMouseEvent& event)  {
  if( state != stDisabled && state != stDown && state != stHover )  {
    state = stHover;
    Paint();
  }
}
//..............................................................................
wxBitmap TImgButton::BmpFromImage(const wxImage& img, int w, int h) const {
  if( img.GetWidth() != w || img.GetHeight() != h )
    return wxBitmap(img.Scale(w,h));
  return wxBitmap(img);
}
//..............................................................................
void TImgButton::SetImages(const TTypeList<wxImage>& images, short imgState, int w, int h)  {
  if( images.IsEmpty() )  return;
  if( w == -1 )  w = images[0].GetWidth();
  if( h == -1 )  h = images[0].GetHeight();
  wxPanel::SetSize(w, h);
  size_t img_index = 0;
  if( (imgState & stUp) != 0 )
    bmpUp = BmpFromImage(images[img_index++], w, h);
  if( img_index < images.Count() && (imgState & stDown) != 0 )
    bmpDown = BmpFromImage(images[img_index++], w, h);
  if( img_index < images.Count() && (imgState & stHover) != 0 )
    bmpHover = BmpFromImage(images[img_index++], w, h);
  if( img_index < images.Count() && (imgState & stDisabled) != 0 )
    bmpDisabled = BmpFromImage(images[img_index++], w, h);
}
//..............................................................................
