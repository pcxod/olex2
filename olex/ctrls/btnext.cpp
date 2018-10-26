/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "btnext.h"
#include "frameext.h"
#include "olxstate.h"
#include "fsext.h"
#include "olxvar.h"

using namespace ctrl_ext;
//..............................................................................
void AButtonBase::SetActionQueue(TActionQueue& q, const olxstr& dependMode) {
  ActionQueue = &q;
  DependMode = dependMode;
  ActionQueue->Add(this);
}
//..............................................................................
bool AButtonBase::Execute(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  if (Data && Data->Is<TModeChange>()) {
    const TModeChange* mc = (const TModeChange*)Data;
    SetDown(TModeRegistry::CheckMode(DependMode));
  }
  return true;
}
//..............................................................................
void AButtonBase::SetDown(bool v) {
  if (Down == v) {
    return;
  }
  if (Down) {
    Down = false;
    OnUp.Execute((AOlxCtrl*)this);
  }
  else {
    Down = true;
    OnDown.Execute((AOlxCtrl*)this);
  }
}
//..............................................................................
void AButtonBase::_ClickEvent() {
  if (Down) {
    OnUp.Execute((AOlxCtrl*)this);
  }
  OnClick.Execute((AOlxCtrl*)this);
  if (!Down) {
    OnDown.Execute((AOlxCtrl*)this);
  }
  Down = !Down;
}
//..............................................................................
//..............................................................................
//..............................................................................
TButton::TButton(wxWindow* parent, wxWindowID id, const wxString& label,
  const wxPoint& pos, const wxSize& size, long style)
: wxButton(parent, id, label, pos, size, style),
  AButtonBase(this)
{
  Bind(wxEVT_BUTTON, &TButton::ClickEvent, this);
  Bind(wxEVT_ENTER_WINDOW, &TButton::MouseEnterEvent, this);
  Bind(wxEVT_LEAVE_WINDOW, &TButton::MouseLeaveEvent, this);
  Bind(wxEVT_PAINT, &TButton::PaintEvent, this);
}
//..............................................................................
void TButton::ClickEvent(wxCommandEvent&) {
  AButtonBase::_ClickEvent();
  SetFocus();
}
//..............................................................................
void TButton::MouseEnterEvent(wxMouseEvent& event) {
  SetCursor(wxCursor(wxCURSOR_HAND));
  if (!GetHint().IsEmpty()) {
    SetToolTip(GetHint().u_str());
  }
  event.Skip();
}
//..............................................................................
void TButton::MouseLeaveEvent(wxMouseEvent& event) {
  event.Skip();
}
//..............................................................................
void TButton::PaintEvent(wxPaintEvent& evt) {
  int alpha = drawParams.Find("border.lightness",
    CustomDraw_Border_Lightness).ToInt();
  wxColor bg;
  if (IsMouseInWindow()) {
    int alpha1 = drawParams.Find("highlight.lightness",
      CustomDraw_Highlight_Lightness).ToInt();
    bg = GetBackgroundColour().ChangeLightness(alpha1);
  }
  else {
    bg = GetBackgroundColour();
  }
  wxPaintDC dc(this);
  dc.SetBrush(wxBrush(bg, wxBRUSHSTYLE_SOLID));
  dc.SetPen(wxPen(GetBackgroundColour().ChangeLightness(alpha), 1, wxPENSTYLE_SOLID));
  dc.DrawRectangle(0, 0, WI.GetWidth(), WI.GetHeight());
  wxSize sz = dc.GetTextExtent(GetLabel());
  dc.DrawText(GetLabel(), (WI.GetWidth() - sz.GetWidth()) / 2,
    (WI.GetHeight()-sz.GetHeight())/2);
  evt.Skip(false);
}
//..............................................................................
//..............................................................................
//..............................................................................
TBmpButton::TBmpButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap,
const wxPoint& pos, const wxSize& size, long style)
: wxBitmapButton(parent, id, bitmap, pos, size, style),
AButtonBase(this)
{
  Bind(wxEVT_BUTTON, &TBmpButton::ClickEvent, this);
  Bind(wxEVT_ENTER_WINDOW, &TBmpButton::MouseEnterEvent, this);
  Bind(wxEVT_LEAVE_WINDOW, &TBmpButton::MouseLeaveEvent, this);
}
//..............................................................................
void TBmpButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor(wxCursor(wxCURSOR_HAND));
  if (!GetHint().IsEmpty()) {
    SetToolTip(GetHint().u_str());
  }
  event.Skip();
}
//..............................................................................
void TBmpButton::MouseLeaveEvent(wxMouseEvent& event) {
  event.Skip();
}
//..............................................................................
//..............................................................................
//..............................................................................
#ifdef __GNUC__
const short TImgButton::stUp;
const short TImgButton::stDown;
const short TImgButton::stDisabled;
const short TImgButton::stHover;
#endif
//..............................................................................
TImgButton::TImgButton(wxWindow* parent)
  : wxPanel(parent), AButtonBase(this)
{
  Bind(wxEVT_LEFT_DOWN, &TImgButton::MouseDownEvent, this);
  Bind(wxEVT_LEFT_UP, &TImgButton::MouseUpEvent, this);
  Bind(wxEVT_MOTION, &TImgButton::MouseMoveEvent, this);
  Bind(wxEVT_ENTER_WINDOW, &TImgButton::MouseEnterEvent, this);
  Bind(wxEVT_LEAVE_WINDOW, &TImgButton::MouseLeaveEvent, this);
  Bind(wxEVT_PAINT, &TImgButton::PaintEvent, this);
  Bind(wxEVT_ERASE_BACKGROUND, &TImgButton::EraseBGEvent, this);
  state = 0;
  width = height = -1;
  ProcessingOnDown = MouseIn = false;
}
//..............................................................................
void TImgButton::MouseEnterEvent(wxMouseEvent& event) {
  SetCursor(wxCursor(wxCURSOR_HAND));
  if (!GetHint().IsEmpty()) {
    SetToolTip(GetHint().u_str());
  }
  MouseIn = true;
  if (state != stDisabled && state != stDown) {
    state = stHover;
    Paint();
  }
  event.Skip();
}
//..............................................................................
void TImgButton::MouseLeaveEvent(wxMouseEvent& event) {
  MouseIn = false;
  if (state != stDisabled && state != stUp && !ProcessingOnDown) {
    state = stUp;
    Paint();
  }
  event.Skip();
}
//..............................................................................
const wxBitmap& TImgButton::ChooseBitmap() const {
  switch (state) {
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
  if (bmp.IsOk()) {
    wxBitmap tbmp(bmp.GetWidth(), bmp.GetHeight(), bmp.GetDepth());
    wxMemoryDC mdc(tbmp);
    mdc.SetBackground(wxBrush(GetBackgroundColour()));
    mdc.Clear();
    mdc.DrawBitmap(bmp, 0, 0, true);
    dc.Blit(0, 0, bmp.GetWidth(), bmp.GetHeight(), &mdc, 0, 0, wxCOPY);
  }
}
//..............................................................................
void TImgButton::MouseDownEvent(wxMouseEvent& event) {
  if (state == stDisabled || state == stDown) {
    return;
  }
  state = stDown;
  Paint();
  ProcessingOnDown = true;
  OnDown.Execute(this);
  if (!MouseIn) {
    OnUp.Execute((AOlxCtrl*)this);
    state = stUp;
    Paint();
  }
  SetFocus();
  ProcessingOnDown = false;
}
//..............................................................................
void TImgButton::MouseUpEvent(wxMouseEvent& event) {
  if (state == stDisabled || state == stUp || state == stHover) {
    return;
  }
  OnUp.Execute(this);
  OnClick.Execute(this);
  state = stUp;
  Paint();
}
//..............................................................................
void TImgButton::MouseMoveEvent(wxMouseEvent& event) {
  if (state != stDisabled && state != stDown && state != stHover) {
    state = stHover;
    Paint();
  }
}
//..............................................................................
wxBitmap TImgButton::BmpFromImage(const wxImage& img, int w, int h) const {
  if (img.GetWidth() != w || img.GetHeight() != h) {
    return wxBitmap(img.Scale(w, h));
  }
  return wxBitmap(img);
}
//..............................................................................
void TImgButton::SetImages(const olxstr& src, int w, int h) {
  Source = src;
  width = w;
  height = h;
  const TStrList toks(src, ',');
  TTypeList<wxImage> images(4, false);
  short imgState = 0;
  for (size_t i = 0; i < toks.Count(); i++)  {
    const size_t ei = toks[i].IndexOf('=');
    if (ei == InvalidIndex) {
      continue;
    }
    const olxstr dest = toks[i].SubStringTo(ei),
      fn = toks[i].SubStringFrom(ei + 1);
    wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler(fn);
    if (fsFile == NULL) {
      TBasicApp::NewLogEntry(logError) << __OlxSrcInfo <<
        ": could not locate image '" << fn << '\'';
      continue;
    }
    wxImage* img;
    {
      wxLogNull bl;
      img = new wxImage(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
      if (!img->IsOk()) {
        TBasicApp::NewLogEntry(logError) << "Failed to load image: " <<
          fn;
        delete img;
        continue;
      }
    }
    if (dest.Equalsi("up")) {
      imgState |= TImgButton::stUp;
      images.Set(0, img);
    }
    else if (dest.Equalsi("down")) {
      imgState |= TImgButton::stDown;
      images.Set(1, img);
    }
    else if (dest.Equalsi("disabled")) {
      imgState |= TImgButton::stDisabled;
      images.Set(2, img);
    }
    else if (dest.Equalsi("hover")) {
      imgState |= TImgButton::stHover;
      images.Set(3, img);
    }
    else {
      delete img;
    }
    delete fsFile;
  }
  images.Pack();
  SetImages(images, imgState, w, h);
}
//..............................................................................
void TImgButton::SetImages(const TTypeList<wxImage>& images, short imgState,
  int w, int h)
{
  if (images.IsEmpty()) {
    return;
  }
  if (w == -1) {
    w = images[0].GetWidth();
  }
  if (h == -1) {
    h = images[0].GetHeight();
  }
  wxPanel::SetSize(w, h);
  size_t img_index = 0;
  if ((imgState & stUp) != 0) {
    bmpUp = BmpFromImage(images[img_index++], w, h);
  }
  if (img_index < images.Count() && (imgState & stDown) != 0) {
    bmpDown = BmpFromImage(images[img_index++], w, h);
  }
  if (img_index < images.Count() && (imgState & stHover) != 0) {
    bmpHover = BmpFromImage(images[img_index++], w, h);
  }
  if (img_index < images.Count() && (imgState & stDisabled) != 0) {
    bmpDisabled = BmpFromImage(images[img_index++], w, h);
  }
}
//..............................................................................
void TImgButton::SetDown(bool v) {
  if (IsDown() != v) {
    olx_swap(bmpUp, bmpDown);
  }
  _SetDown(v);
}
//..............................................................................
