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

int* TGlCanvas::glAttrib = NULL;

IMPLEMENT_CLASS(TGlCanvas, wxGLCanvas)

BEGIN_EVENT_TABLE(TGlCanvas, wxGLCanvas)
  EVT_SIZE(TGlCanvas::OnSize)
  EVT_PAINT(TGlCanvas::OnPaint)
  EVT_ERASE_BACKGROUND(TGlCanvas::OnEraseBackground)

  EVT_LEFT_UP(TGlCanvas::OnMouseUp)
  EVT_RIGHT_UP(TGlCanvas::OnMouseUp)
  EVT_LEFT_DOWN(TGlCanvas::OnMouseDown)
  EVT_RIGHT_DOWN(TGlCanvas::OnMouseDown)
  EVT_MOTION(TGlCanvas::OnMouseMove)
  EVT_MOUSE_EVENTS(TGlCanvas::OnMouse)
  EVT_LEFT_DCLICK(TGlCanvas::OnMouseDblClick)

  EVT_KEY_UP(TGlCanvas::OnKeyUp)
  EVT_CHAR(TGlCanvas::OnChar)
  EVT_KEY_DOWN(TGlCanvas::OnKeyDown)

END_EVENT_TABLE()
//..............................................................................
TGlCanvas::TGlCanvas(TMainForm *parent, int* gl_attr, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name):
#if defined(__WXX11__) || defined(__MAC__)
  wxGLCanvas(parent, (wxGLCanvas*)NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name )  {
  Context = NULL;
#else
  wxGLCanvas(parent, id, gl_attr, pos, size, style, name )  {
  Context = new wxGLContext(this, NULL);
#ifdef __WIN32__ // on GTK the context initialisation is delayed
  Context->SetCurrent(*this);
#endif
#endif
  FXApp = NULL;
  MouseButton = 0;
  FParent = parent;
}
//..............................................................................
TGlCanvas::~TGlCanvas(){
  if( Context != NULL )
    delete Context;
  if( glAttrib != NULL )  {
    delete [] glAttrib;
    glAttrib = NULL;
  }
}
//..............................................................................
void TGlCanvas::Render()  {
#if !defined(__WXMOTIF__) && !defined(__WIN32__) && !defined(__WXGTK__)
  if( !GetContext() ) return;
#endif
#if defined(__WXX11__) || defined(__MAC__)  // context is null
  SetCurrent();
#else
  Context->SetCurrent(*this); 
#endif

  /* init OpenGL once, but after SetCurrent */
  if( FXApp == NULL )  return;
  FXApp->Draw();
}
//..............................................................................
void TGlCanvas::OnPaint(wxPaintEvent& event)  {
  wxPaintDC dc(this);
  Render();
}
//..............................................................................
void TGlCanvas::OnSize(wxSizeEvent& event)  {
  // this is also necessary to update the context on some platforms
  wxGLCanvas::OnSize(event);
  // this causes many problems with GTK, as BadMatch ...
//  if( IsShown() )
//    Context->SetCurrent(*this); 
}
//..............................................................................
void TGlCanvas::OnEraseBackground(wxEraseEvent& event)  {
}
//..............................................................................
void TGlCanvas::InitGL()  {
  if( FXApp != NULL )  
    FXApp->Init();
}
//..............................................................................
short TGlCanvas::EncodeEvent(const wxMouseEvent &evt, bool update_button)  {
  short Fl = 0;
  if( evt.m_altDown )  Fl |= sssAlt;
  if( evt.m_shiftDown )  Fl |= sssShift;
  if( evt.m_controlDown )  Fl |= sssCtrl;
  if( update_button )  {
    MouseButton = 0;
    if( evt.LeftIsDown() )  MouseButton = smbLeft;
    if( evt.MiddleIsDown() )  MouseButton |= smbMiddle;
    if( evt.RightIsDown() )  MouseButton |= smbRight;
  }
  return Fl;
}
//..............................................................................
void TGlCanvas::OnMouseDown(wxMouseEvent& me)  {
  short Fl = EncodeEvent(me);
  FParent->OnMouseDown(me.m_x, me.m_y, Fl, MouseButton);
  FXApp->MouseDown(me.m_x, me.m_y, Fl, MouseButton);
  AGDrawObject *G = FXApp->SelectObject(me.m_x, me.m_y);
  if( G != NULL )  {
    TGlGroup *GlG = FXApp->FindObjectGroup(*G);
    if( GlG != NULL )  
      G = GlG;
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
  MouseButton ^= mb;
  int left = 0, top = 0;
#ifdef __MAC__  // a solution for MAC's single button
  int os_mask = sssCtrl;
#else
  int os_mask = 0;
#endif
  GetPosition(&left, &top);

  if( FParent->OnMouseUp(me.m_x, me.m_y, Fl, MouseButton) )  {
  }
  else if( FXApp->MouseUp(me.m_x, me.m_y, Fl, MouseButton) )  {
  }
  else if( (abs(me.m_x-FMX) <= 4) && (abs(me.m_y-FMY) <= 4) &&
    (MouseButton == smbRight) && (Fl == os_mask || Fl == 0) )
  {
//    FMY += (wxSystemSettings::GetMetric(wxSYS_MENU_Y)*FParent->pmMenu->GetMenuItemCount());
//      FXApp->MouseUp(me.m_x, me.m_y, Fl, Btn);
    AGDrawObject *G = FXApp->SelectObject(me.m_x, me.m_y);
    bool Handled = false;
    if( G != NULL )  {
      TGlGroup *GlG = FXApp->FindObjectGroup(*G);
      if( GlG == NULL )  
        FParent->ObjectUnderMouse(G);
      else        
        FParent->ObjectUnderMouse(GlG);
      if( FParent->CurrentPopupMenu() )  {
        FParent->PopupMenu(FParent->CurrentPopupMenu(), FMX+left, FMY+top);
        Handled = true;
      }
      if( !Handled )
        FParent->PopupMenu(FParent->DefaultPopup(), FMX+left, FMY+top);
    }
    else
      FParent->PopupMenu(FParent->GeneralPopup(), FMX+left, FMY+top);
    SetFocus();
    return;
  }
  else if( FParent->OnMouseUp(me.m_x, me.m_y, Fl, MouseButton) )  {
    FXApp->ResetMouseState();
  }
  FXApp->Draw();
  FXApp->ResetMouseState();
  MouseButton = 0;
}
//..............................................................................
void TGlCanvas::OnMouseMove(wxMouseEvent& me)  {
  short Fl = EncodeEvent(me, false);
  if( MouseButton == 0 )  
    FParent->OnMouseMove(me.m_x, me.m_y);
  if( FXApp != NULL && FXApp->MouseMove(me.m_x, me.m_y, Fl) )  {  // check if a handler for the event is found
#ifdef __WIN32__
    wxWindow::Refresh();  // this seems to help but not to solve! with the desktop compositionon Vista/7
#else
    FXApp->Draw();
#endif
  }
}
//..............................................................................
void TGlCanvas::OnMouseDblClick(wxMouseEvent& me)  {
  short Fl = EncodeEvent(me);
  if( FXApp != NULL && !FXApp->DblClick() )
    FParent->OnMouseDblClick(me.m_x, me.m_y, Fl, MouseButton);
}
//..............................................................................
void TGlCanvas::OnMouse(wxMouseEvent& me)  {
  if( me.GetWheelRotation() != 0 )  
    FParent->OnMouseWheel(me.GetX(), me.GetY(), (double)me.GetWheelRotation()/me.GetWheelDelta());
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
int* TGlCanvas::GetGlAttributes(bool _default, bool stereo, short depth_bits)  {
  if( _default )  return NULL;
  if( glAttrib != NULL )
    delete [] glAttrib;
  const int cnt = stereo ? 8 : 7;
  glAttrib = new int [2*cnt+1];
  glAttrib[2*cnt] = 0;
  glAttrib[0] = WX_GL_RGBA;  glAttrib[1] = 1;
  glAttrib[2] = WX_GL_DOUBLEBUFFER;  glAttrib[3] = 1;
  glAttrib[4] = WX_GL_MIN_ACCUM_RED;  glAttrib[5] = 16;
  glAttrib[6] = WX_GL_MIN_ACCUM_GREEN;  glAttrib[7] = 16;
  glAttrib[8] = WX_GL_MIN_ACCUM_BLUE;  glAttrib[9] = 16;
  glAttrib[10] = WX_GL_MIN_ACCUM_ALPHA;  glAttrib[11] = 16;
  glAttrib[12] = WX_GL_DEPTH_SIZE;  glAttrib[13] = depth_bits;
  if( stereo )  {
    glAttrib[14] = WX_GL_STEREO;  glAttrib[15] = 1;
  }
//  WX_GL_SAMPLE_BUFFERS, 1,  // these are not defined though documented...
//  WX_GL_SAMPLES, 4,
  return glAttrib;
}