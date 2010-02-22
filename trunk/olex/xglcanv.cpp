//----------------------------------------------------------------------------//
// GlCanvas implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xglcanv.h"
#include "mainform.h"
#include "glgroup.h"

int TGlCanvas::glAttrib[] = {
  WX_GL_RGBA, 1, 
  WX_GL_DOUBLEBUFFER, 1, 
//  WX_GL_SAMPLE_BUFFERS, 1,  // these are not defined though documented...
//  WX_GL_SAMPLES, 4,
  0};
 //----------------------------------------------------------------------------//
// TGlCanvas function bodies
//----------------------------------------------------------------------------//

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
  Context = new wxGLContext( this, NULL);
#ifdef __WIN32__ // on GTK the context initialisation is delayed
  Context->SetCurrent(*this);
#endif
#endif
  FXApp = NULL;
  FLeftMouseDown = FRightMouseDown = false;
  FParent = parent;
}
//..............................................................................
TGlCanvas::~TGlCanvas(){
  if( Context != NULL )
    delete Context; 
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
void TGlCanvas::OnMouseDown(wxMouseEvent& me)  {
  short Fl = 0, Btn = 0;
  if( me.m_altDown )      Fl |= sssAlt;
  if( me.m_shiftDown )    Fl |= sssShift;
  if( me.m_controlDown )  Fl |= sssCtrl;
  if( me.ButtonDown(1) )    Btn = smbLeft;
  if( me.ButtonDown(2) )    Btn = smbMiddle;
  if( me.ButtonDown(3) )    Btn = smbRight;

  FXApp->MouseDown(me.m_x, me.m_y, Fl, Btn);
  if( me.ButtonDown(1) )  FLeftMouseDown = true;
  if( me.ButtonDown(3) )  FRightMouseDown = true;
  FMX = me.m_x;
  FMY = me.m_y;
  SetFocus();
  me.Skip();
}
//..............................................................................
void TGlCanvas::OnMouseUp(wxMouseEvent& me)  {
  me.Skip();
  short Fl = 0, Btn = 0;
  if( me.m_altDown )      Fl |= sssAlt;
  if( me.m_shiftDown )    Fl |= sssShift;
  if( me.m_controlDown )  Fl |= sssCtrl;
  if( me.ButtonDown(1) )    Btn = smbLeft;
  if( me.ButtonDown(2) )    Btn = smbMiddle;
  if( me.ButtonDown(3) )    Btn = smbRight;

  int left = 0, top = 0;
#ifdef __MAC__  // a solution for MAC's single button
  int os_mask = sssCtrl;
#else
  int os_mask = 0;
#endif
  GetPosition(&left, &top);

  if( (abs(me.m_x-FMX) <= 4) && (abs(me.m_y-FMY) <= 4) &&
    (FRightMouseDown) && (Fl == os_mask || Fl == 0) )
  {
//    FMY += (wxSystemSettings::GetMetric(wxSYS_MENU_Y)*FParent->pmMenu->GetMenuItemCount());
    FXApp->ResetMouseState();  // reset mouse state
    if( FRightMouseDown )  {
      FXApp->MouseUp(me.m_x, me.m_y, Fl, Btn);
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
    }
    SetFocus();
    FRightMouseDown = false;
    FLeftMouseDown = false;
    return;
  }
  if( FParent->OnMouseUp(me.m_x, me.m_y, Fl, Btn) )  {
    FXApp->ResetMouseState();
    FXApp->Draw();
  }
  else  {
    if( FXApp->MouseUp(me.m_x, me.m_y, Fl, Btn) )  
      FXApp->Draw();
  }
  FRightMouseDown = false;
  FLeftMouseDown = false;
}
//..............................................................................
void TGlCanvas::OnMouseMove(wxMouseEvent& me)  {
  short Fl = 0;
  if( me.m_altDown )      Fl |= sssAlt;
  if( me.m_shiftDown )    Fl |= sssShift;
  if( me.m_controlDown )  Fl |= sssCtrl;

  if( !FLeftMouseDown && !FRightMouseDown )  
    FParent->OnMouseMove(me.m_x, me.m_y);
  if( FXApp != NULL && FXApp->MouseMove(me.m_x, me.m_y, Fl) )  {  // check if a handler for the event is found
#ifdef __WIN32__
    wxWindow::Refresh();
#else
    FXApp->Draw();
#endif
  }
}
//..............................................................................
void TGlCanvas::OnMouseDblClick(wxMouseEvent& me)  {
  short Fl = 0, Btn = 0;
  if( me.m_altDown )      Fl |= sssAlt;
  if( me.m_shiftDown )    Fl |= sssShift;
  if( me.m_controlDown )  Fl |= sssCtrl;
  if( me.ButtonDown(1) )    Btn = smbLeft;
  if( me.ButtonDown(2) )    Btn = smbMiddle;
  if( me.ButtonDown(3) )    Btn = smbRight;

  if( FXApp != NULL && !FXApp->DblClick() )
    FParent->OnMouseDblClick( me.m_x, me.m_y, Fl, Btn );
  FRightMouseDown = false;
}
//..............................................................................
void TGlCanvas::OnMouse(wxMouseEvent& me)  {
  if( me.GetWheelRotation() != 0 )  
    FParent->OnMouseWheel(me.GetX(), me.GetY(), (double)me.GetWheelRotation()/me.GetWheelDelta());
  me.Skip();
}
//..............................................................................
void TGlCanvas::OnKeyDown(wxKeyEvent& m)
{
  FParent->OnKeyDown(m);
}
//..............................................................................
void TGlCanvas::OnKeyUp(wxKeyEvent& m)
{
  FParent->OnKeyUp(m);
}
void TGlCanvas::OnChar(wxKeyEvent& m)
{
  FParent->OnChar(m);
}
//..............................................................................
