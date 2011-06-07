#ifndef xglCanvasH
#define xglCanvasH
#include "wx/glcanvas.h"
#include "gxapp.h"

class TGlCanvas: public wxGLCanvas  {
private:
  class TGXApp *FXApp;
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseDblClick(wxMouseEvent& event);

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);

  int FMX, FMY; // mouse coordinates to process clicks
  short MouseButton;
  class TMainForm *FParent;
  wxGLContext* Context;
  static int* glAttrib;
  short EncodeEvent(const wxMouseEvent &evt, bool update_button=true);
public:
  TGlCanvas(TMainForm *parent, int* gl_attr, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxT("TGlCanvas"));
  ~TGlCanvas();

  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMouse( wxMouseEvent& event );
  void InitGL(void);
  void XApp(TGXApp *XA){  FXApp = XA; };
  TGXApp *GetXApp()    {  return FXApp;};

  void Render();
  /* the arrays is staically allocated and should not be modified!!! 
  wxWidgets needs not a const pointer... If default is true - NULL is returned,
  else if stereo is true - Olex2 will try to initialise stereo buffers */
  static int* GetGlAttributes(bool _default, bool stereo);
  DECLARE_CLASS(wxGLCanvas)
  DECLARE_EVENT_TABLE()
};
#endif
