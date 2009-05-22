#include "interface.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "pyext.h"
#include "ins.h"
#include "olxvar.h"
//#include "wx/fontmap.h"
#include "wx/font.h"

#include "xatom.h"
#include "xbond.h"

TOlexViewer* TOlexViewer::Instance = NULL;

TOlexViewer::TOlexViewer(const char* olex_path, HDC windowDC, int w, int h) : WindowDC(windowDC) {
  PythonExt::Init(this);
  GXApp = new TGXApp(olex_path); 
  Instance = this;
  int PixelFormat;
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW |	// support window
	  PFD_SUPPORT_OPENGL |	// support OpenGL
	  PFD_DOUBLEBUFFER,	// double buffered
	  PFD_TYPE_RGBA,
    24,  // 24 bit
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
  };
  PixelFormat = ChoosePixelFormat(WindowDC, &pfd);
  SetPixelFormat(WindowDC, PixelFormat, &pfd);
  GlContext = wglCreateContext(WindowDC);
  wglMakeCurrent(WindowDC, GlContext); 
  glClearColor(0, 0, 0, 0);
  OnSize(w, h);
  wxFont Font(*wxNORMAL_FONT);//|wxFONTFLAG_ANTIALIASED);
//  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  // create 4 fonts
  TGlFont *fnt = GXApp->GetRender().Scene()->CreateFont("Labels", &Font, NULL, true, true);
  fnt->Material().SetFlags(sglmAmbientF|sglmEmissionF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;
  fnt->Material().EmissionF = 0x1f2f1f;
  
  GXApp->LabelsFont(0);

  TIns *Ins = new TIns(GXApp->AtomsInfo());
  GXApp->RegisterXFileFormat(Ins, "ins");
  GXApp->RegisterXFileFormat(Ins, "res");
  GXApp->SetMainFormVisible(true);
  GXApp->AtomRad("isot");
  //GXApp->SetAtomDrawingStyle(adsEllipsoid);
  TXAtom::DefElpMask(5);
  TXAtom::DefDS(adsEllipsoid);
  TXBond::DefMask(48);
  GXApp->LabelsVisible(false);
  GXApp->CalcProbFactor(50);
}
TOlexViewer::~TOlexViewer()  {
  Instance = NULL;
  delete GXApp;
  if( GlContext != NULL )  {
    wglMakeCurrent(WindowDC, NULL); 
    wglDeleteContext(GlContext);
  }
  TOlxVars::Finalise();
  PythonExt::Finilise();
}
void TOlexViewer::OnPaint()  {
//  OnSize(Width, Height);
  GXApp->Draw();
  GdiFlush();
  glFlush();
  SwapBuffers(WindowDC);
}
void TOlexViewer::OnSize(int w, int h)  {
  GXApp->GetRender().Resize(0, 0, w, h, 1);
  Width = w;
  Height = h;
}
bool TOlexViewer::OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState)  {
  bool res = false;
  short btn = 0, shift = 0;
  if( MouseButton & olxv_MouseLeft )   btn |= smbLeft;
  else if( MouseButton & olxv_MouseRight )  btn |= smbRight;
  else if( MouseButton & olxv_MouseMiddle ) btn |= smbMiddle;
  if( ShiftState & olxv_ShiftShift )  shift |= sssShift;
  if( ShiftState & olxv_ShiftAlt )    shift |= sssAlt;
  if( ShiftState & olxv_ShiftCtrl )   shift |= sssCtrl;

  if( MouseEvent == olxv_MouseDown ) 
    res = GXApp->MouseDown(x, y, shift, btn);
  else if( MouseEvent == olxv_MouseUp ) 
    res = GXApp->MouseUp(x, y, shift, btn);
  else if( MouseEvent == olxv_MouseMove ) 
    res = GXApp->MouseMove(x, y, shift);
  if( res || true )  {
    GXApp->Draw();
    SwapBuffers(WindowDC);
  }
  return res;
}
void TOlexViewer::OnFileChanged(const char* fileName)  {
  try  {
    GXApp->LoadXFile(fileName);
    GXApp->SetAtomDrawingStyle(adsEllipsoid);
    GXApp->Uniq();
  }
  catch(...)  {}
}

bool TOlexViewer::executeMacro(const olxstr& cmdLine)  {
  return false;
}
void TOlexViewer::print(const olxstr& Text, const short MessageType)  {
  return;
}
bool TOlexViewer::executeFunction(const olxstr& funcName, olxstr& retValue)  {
  return false;
}
IEObject* TOlexViewer::executeFunction(const olxstr& funcName)  {
  return NULL;
}
TLibrary&  TOlexViewer::GetLibrary()  {  return GXApp->GetLibrary();  }
bool TOlexViewer::registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn) {
  return false;
}
void TOlexViewer::unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName) {
  return;
}

const olxstr& TOlexViewer::getDataDir() const  {  return EmptyString;  }
const olxstr& TOlexViewer::getVar(const olxstr &name, const olxstr &defval) const  {
  return EmptyString;
}
void TOlexViewer::setVar(const olxstr &name, const olxstr &val) const {
  return;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}