/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xglcanv.h"
#include "mainform.h"
#include "glgroup.h"
#include "wxglscene.h"

IMPLEMENT_CLASS(TGlCanvas, wxGLCanvas)
//..............................................................................
TGlCanvas::TGlCanvas(TMainForm *parent, int* gl_attr, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name):
#if wxCHECK_VERSION(2,9,0) || !(defined(__WXX11__) || defined(__MAC__))
  wxGLCanvas(parent, id, gl_attr, pos, size, style, name )  {
  Context = new wxGLContext(this, NULL);
  
// on GTK the context initialisation is delayed
#  if defined(__WIN32__) || defined(__MAC__)
  Context->SetCurrent(*this);
#  endif
#else
  wxGLCanvas(parent, (wxGLCanvas*)NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name )  {
  Context = NULL;
#endif
  FXApp = NULL;
  MouseButton = 0;
  FParent = parent;
  Bind(wxEVT_PAINT, &TGlCanvas::OnPaint, this);
  Bind(wxEVT_ERASE_BACKGROUND, &TGlCanvas::OnEraseBackground, this);

  Bind(wxEVT_LEFT_UP, &TGlCanvas::OnMouseUp, this);
  Bind(wxEVT_RIGHT_UP, &TGlCanvas::OnMouseUp, this);
  Bind(wxEVT_LEFT_DOWN, &TGlCanvas::OnMouseDown, this);
  Bind(wxEVT_RIGHT_DOWN, &TGlCanvas::OnMouseDown, this);
  Bind(wxEVT_MOTION, &TGlCanvas::OnMouseMove, this);
  Bind(wxEVT_MOUSE_CAPTURE_CHANGED, &TGlCanvas::OnMouseCaptureChange, this);
  Bind(wxEVT_MOUSEWHEEL, &TGlCanvas::OnMouseWheel, this);
  Bind(wxEVT_LEFT_DCLICK, &TGlCanvas::OnMouseDblClick, this);

  Bind(wxEVT_KEY_UP, &TGlCanvas::OnKeyUp, this);
  Bind(wxEVT_CHAR, &TGlCanvas::OnChar, this);
  Bind(wxEVT_KEY_DOWN, &TGlCanvas::OnKeyDown, this);
  }
//..............................................................................
TGlCanvas::~TGlCanvas() {
  if (TBasicApp::HasInstance()) {
    TwxGlScene *wgls = dynamic_cast<TwxGlScene*>(
      &FXApp->GetRenderer().GetScene());
    if (wgls != 0) {
      wgls->SetCanvas(0);
      wgls->SetContext(0);
    }
  }
  if (Context != 0) {
    delete Context;
  }
}
//..............................................................................
void TGlCanvas::XApp(TGXApp *XA) {
  FXApp = XA;
  TwxGlScene *wgls = dynamic_cast<TwxGlScene*>(&XA->GetRenderer().GetScene());
  if (wgls == NULL) {
    return;
  }
  wgls->SetCanvas(this);
#if wxCHECK_VERSION(2,9,0) || !(defined(__WXX11__) || defined(__MAC__))
  wgls->SetContext(Context);
#else
  wgls->SetContext(GetContext());
#endif
}
//..............................................................................
void TGlCanvas::Render() {
#if !defined(__WXMOTIF__) && !defined(__WIN32__) && !defined(__WXGTK__) && !wxCHECK_VERSION(3,1,0)
  if (GetContext() == 0) {
    return;
  }
#endif
  if (FXApp == 0 || !IsShown()) {
    return;
  }
#if (defined(__WXX11__) || defined(__MAC__))  && !wxCHECK_VERSION(3,1,0)
  SetCurrent();
#else
  Context->SetCurrent(*this);
#endif
  FXApp->Draw();
}
//..............................................................................
void TGlCanvas::OnPaint(wxPaintEvent& event)  {
  Render();
}
//..............................................................................
void TGlCanvas::OnEraseBackground(wxEraseEvent& event)  {
}
//..............................................................................
short TGlCanvas::EncodeEvent(const wxMouseState &evt, bool update_button)  {
  short Fl = 0;
  if (evt.m_altDown) {
    Fl |= sssAlt;
  }
  if (evt.m_shiftDown) {
    Fl |= sssShift;
  }
  if (evt.m_controlDown) {
    Fl |= sssCtrl;
  }
  if (update_button) {
    MouseButton = 0;
    if (evt.LeftIsDown()) {
      MouseButton = smbLeft;
    }
    if (evt.MiddleIsDown()) {
      MouseButton |= smbMiddle;
    }
    if (evt.RightIsDown()) {
      MouseButton |= smbRight;
    }
  }
  return Fl;
}
//..............................................................................
void TGlCanvas::OnMouseDown(wxMouseEvent& me) {
  //short mb = MouseButton;
  short Fl = EncodeEvent(me);
  //MouseButton |= mb;
  FParent->OnMouseDown(me.m_x, me.m_y, Fl, MouseButton);
  FXApp->MouseDown(me.m_x, me.m_y, Fl, MouseButton);
  //AGDrawObject* G = FXApp->SelectObject(me.m_x, me.m_y, false);
  AGDrawObject* G = FXApp->GetMouseObject();
  if (G != 0) {
    TGlGroup* GlG = FXApp->FindObjectGroup(*G);
    if (GlG != 0) {
      G = GlG;
    }
  }
  FParent->ObjectUnderMouse(G);
  FMX = me.m_x;
  FMY = me.m_y;
  SetFocus();
  me.Skip();
}
//..............................................................................
void TGlCanvas::OnMouseUp(wxMouseEvent& me)  {
  me.Skip();
  short mb = MouseButton;
  short Fl = EncodeEvent(me, true);
  short up = MouseButton ^ mb;
  int left = 0, top = 0;
#ifdef __MAC__  // a solution for MAC's single button
  int os_mask = sssCtrl;
#else
  int os_mask = 0;
#endif
  GetPosition(&left, &top);
  // MouseUp resets the previously set object - capture it before that
  AGDrawObject* G = FXApp->GetMouseObject();
  if (FParent->OnMouseUp(me.m_x, me.m_y, Fl, up)) {
  }
  else if (FXApp->MouseUp(me.m_x, me.m_y, Fl, up)) {
  }
  else if ((abs(me.m_x-FMX) <= 4) && (abs(me.m_y-FMY) <= 4) &&
    (up == smbRight) && (Fl == os_mask || Fl == 0))
  {
//    FMY += (wxSystemSettings::GetMetric(wxSYS_MENU_Y)*FParent->pmMenu->GetMenuItemCount());
//      FXApp->MouseUp(me.m_x, me.m_y, Fl, Btn);
    //AGDrawObject *G = FXApp->SelectObject(me.m_x, me.m_y, false);
    bool Handled = false;
    if (G != 0) {
      TGlGroup *GlG = FXApp->FindObjectGroup(*G);
      if (GlG == 0) {
        FParent->ObjectUnderMouse(G);
      }
      else {
        FParent->ObjectUnderMouse(GlG);
      }
      if (FParent->CurrentPopupMenu()) {
        FParent->PopupMenu(FParent->CurrentPopupMenu(), FMX+left, FMY+top);
        Handled = true;
      }
      if (!Handled) {
        FParent->PopupMenu(FParent->DefaultPopup(), FMX + left, FMY + top);
      }
    }
    else {
      FParent->PopupMenu(FParent->GeneralPopup(), FMX + left, FMY + top);
    }
    SetFocus();
    return;
  }
  else if (FParent->OnMouseUp(me.m_x, me.m_y, Fl, up)) {
    FXApp->ResetMouseState(me.m_x, me.m_y);
  }
  FXApp->Draw();
  FXApp->ResetMouseState(me.m_x, me.m_y, Fl, MouseButton);
  MouseButton = 0;
}
//..............................................................................
void TGlCanvas::OnMouseMove(wxMouseEvent& me) {
  short Fl = EncodeEvent(me, false);
  if (MouseButton == 0)
    FParent->OnMouseMove(me.m_x, me.m_y);
  else
    FParent->OnNonIdle();
  // check if a handler for the event is found
  if (FXApp != NULL && FXApp->MouseMove(me.m_x, me.m_y, Fl)) {
#ifdef __WIN32__
    /* this seems to help but not to solve! with the desktop composition on
    Vista/7
    */
    wxWindow::Refresh();
#else
    FXApp->Draw();
#endif
  }
}
//..............................................................................
void TGlCanvas::OnMouseDblClick(wxMouseEvent& me)  {
  short Fl = EncodeEvent(me);
  if (FXApp != NULL && !FXApp->DblClick())
    FParent->OnMouseDblClick(me.m_x, me.m_y, Fl, MouseButton);
}
//..............................................................................
void TGlCanvas::OnMouseWheel(wxMouseEvent& me) {
  if (me.GetWheelRotation() != 0) {
    FParent->OnMouseWheel(me.GetX(), me.GetY(),
    (double)me.GetWheelRotation()/me.GetWheelDelta());
  }
  me.Skip();
}
//..............................................................................
void TGlCanvas::OnMouseCaptureChange(wxMouseCaptureChangedEvent& me) {
  wxMouseState ms = wxGetMouseState();
  short fl = EncodeEvent(ms, true);
  FXApp->ResetMouseState(ms.m_x, ms.m_y, fl, MouseButton, true);
  me.Skip();
}
//..............................................................................
void TGlCanvas::OnKeyDown(wxKeyEvent& m)  {
  FParent->OnKeyDown(m);
}
//..............................................................................
void TGlCanvas::OnKeyUp(wxKeyEvent& m)  {
  FParent->OnKeyUp(m);
}
void TGlCanvas::OnChar(wxKeyEvent& m)  {
  FParent->OnChar(m);
}
//..............................................................................
olx_array_ptr<int> TGlCanvas::GetGlAttributes(bool _default, bool stereo,
  bool multisampling, short depth_bits)
{
  if (_default)  return (int*)NULL;
  size_t idx=0;
  olx_array_ptr<int> glAttrib(2*11+1);
  glAttrib[idx++] = WX_GL_RGBA;
  glAttrib[idx++] = WX_GL_DOUBLEBUFFER;
  //glAttrib[idx++] = WX_GL_MIN_ACCUM_RED;  glAttrib[idx++] = 16;
  //glAttrib[idx++] = WX_GL_MIN_ACCUM_GREEN;  glAttrib[idx++] = 16;
  //glAttrib[idx++] = WX_GL_MIN_ACCUM_BLUE;  glAttrib[idx++] = 16;
  //glAttrib[idx++] = WX_GL_MIN_ACCUM_ALPHA;  glAttrib[idx++] = 16;
  glAttrib[idx++] = WX_GL_DEPTH_SIZE;  glAttrib[idx++] = depth_bits;
  glAttrib[idx++] = WX_GL_STENCIL_SIZE;  glAttrib[idx++] = 8;
#if wxCHECK_VERSION(2,9,0)
  if (multisampling) {
    glAttrib[idx++] = WX_GL_SAMPLE_BUFFERS;  glAttrib[idx++] = 1;
    glAttrib[idx++] = WX_GL_SAMPLES;  glAttrib[idx++] = 4;
  }
#endif
  if (stereo)  {
    glAttrib[idx++] = WX_GL_STEREO;
  }
  glAttrib[idx++] = 0;
  return glAttrib;
}
