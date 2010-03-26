#include "interface.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include "ins.h"
#include "olxvar.h"
//#include "wx/wx.h"
//#include "wx/fontmap.h"
//#include "wx/font.h"
//#include "wx/settings.h"

#include "xatom.h"
#include "xbond.h"
#include "gllabels.h"
#include "shellutil.h"
#include "patchapi.h"

TOlexViewer* TOlexViewer::Instance = NULL;

TOlexViewer::TOlexViewer(HDC windowDC, int w, int h) : WindowDC(windowDC) {
  GXApp = new TGXApp(TBasicApp::GuessBaseDir(GetCommandLine(), "OLEX2_DIR")); 
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
  if( !SetPixelFormat(WindowDC, PixelFormat, &pfd) )
    throw TFunctionFailedException(__OlxSourceInfo, "SetPixelFormat failed");
  GlContext = wglCreateContext(WindowDC);
  if( GlContext == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "wglCreateContext failed");
  if( !wglMakeCurrent(WindowDC, GlContext) ) 
    throw TFunctionFailedException(__OlxSourceInfo, "wglMakeCurrent failed");
  
  olx_gl::clearColor(0.5, 0.5, 0.0, 0.0);
  OnSize(w, h);
  TGlMaterial glm;
  glm.SetFlags(sglmAmbientF|sglmEmissionF|sglmIdentityDraw);
  glm.AmbientF = 0x7fff7f;
  glm.EmissionF = 0x1f2f1f;
  //wxFont Font(*wxNORMAL_FONT);//|wxFONTFLAG_ANTIALIASED);
  //GXApp->GetRender().GetScene().CreateFont("Labels", Font.GetNativeFontInfoDesc().c_str())->SetMaterial(glm);
  GXApp->GetRender().GetScene().CreateFont("Labels", "@20")->SetMaterial(glm);
 
  GXApp->SetLabelsFont(0);

  TIns *Ins = new TIns;
  GXApp->RegisterXFileFormat(Ins, "ins");
  GXApp->RegisterXFileFormat(Ins, "res");
  GXApp->SetMainFormVisible(true);
  GXApp->AtomRad("isot");
  //GXApp->SetAtomDrawingStyle(adsEllipsoid);
  TXAtom::DefMask(5);
  TXAtom::DefDS(adsEllipsoid);
  TXBond::DefMask(48);
  GXApp->SetLabelsVisible(false);
  GXApp->SetLabelsMode(lmLabels);
  GXApp->CalcProbFactor(50);
  GXApp->EnableSelection(false);
  DefDS = olxv_DrawStyleTelp;
  
  DataDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
  DataDir = patcher::PatchAPI::ComposeNewSharedDir(DataDir);

  if( TEFile::Exists(DataDir + "default.glsp") )
    LoadScene(DataDir + "default.glsp");
  if( TEFile::Exists(DataDir + "default.glds") )
    LoadStyle(DataDir + "default.glds");
  
}
TOlexViewer::~TOlexViewer()  {
  Instance = NULL;
  GXApp->OnIdle.Execute(NULL, NULL);
  delete GXApp;
  if( GlContext != NULL )  {
    wglMakeCurrent(WindowDC, NULL); 
    wglDeleteContext(GlContext);
  }
  TOlxVars::Finalise();
  // ! wxStockGDI::DeleteAll();
  // there is still a static font variable left from wxWidgest, duno how to reach it cleanly...
}
void TOlexViewer::OnPaint()  {
//  OnSize(Width, Height);
  GXApp->OnIdle.Execute(NULL, NULL);
  GXApp->Draw();
  GdiFlush();
  olx_gl::flush();
  SwapBuffers(WindowDC);
}
void TOlexViewer::OnSize(int w, int h)  {
  GXApp->GetRender().Resize(0, 0, w, h, 1);
  Width = w;
  Height = h;
}
//.......................................................................................
bool TOlexViewer::OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState)  {
  GXApp->OnIdle.Execute(NULL, NULL);
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
  if( res )  {
    GXApp->Draw();
    SwapBuffers(WindowDC);
  }
  return res;
}
//.......................................................................................
bool TOlexViewer::OnFileChanged(const char* fileName)  {
  try  {
    GXApp->OnIdle.Execute(NULL, NULL);
    GXApp->LoadXFile(fileName);
    GXApp->SetAtomDrawingStyle(adsEllipsoid);
    GXApp->Uniq();
    DrawStyle( DefDS );
    return true;
  }
  catch(...)  {
    return false;
  }
}
//.......................................................................................
void TOlexViewer::Clear()  {
  GXApp->XFile().GetRM().ClearAll();
  GXApp->XFile().GetLattice().Clear(true);
  GXApp->CreateObjects(false, false);
}
//.......................................................................................
olxstr TOlexViewer::GetObjectLabelAt(int x, int y)  {
  AGDrawObject *G = GXApp->SelectObject(x, y, 0);
  olxstr Tip;
  if( G != NULL )  {
    if( EsdlInstanceOf( *G, TXAtom) )  {
      TXAtom& xa = *(TXAtom*)G;
      Tip = xa.Atom().GetLabel();
      if( xa.Atom().GetType() == iQPeakZ )  {
        Tip << ':' << xa.Atom().CAtom().GetQPeak();
      }
    }
    else  if( EsdlInstanceOf( *G, TXBond) )  {
      Tip = ((TXBond*)G)->Bond().A().GetLabel();
      Tip << '-' << ((TXBond*)G)->Bond().B().GetLabel() << ": ";
      Tip << olxstr::FormatFloat(3, ((TXBond*)G)->Bond().Length());
    } 
  }
  return Tip;
}
//.......................................................................................
olxstr TOlexViewer::GetSelectionInfo()  {
  return GXApp->GetSelectionInfo();
}
//.......................................................................................
void TOlexViewer::ShowLabels(unsigned short type)  {
  short t = type == 0 ? 0 : lmLabels;
  if( (type & olxv_LabelsH) != 0 )  t |= lmHydr;
  if( (type & olxv_LabelsQ) != 0 )  t |= lmQPeak;
  if( t == 0 )
    GXApp->SetLabelsVisible(false);
  else  {
    GXApp->SetLabelsMode(t);
    GXApp->SetLabelsVisible(true);
  }
}
//.......................................................................................
void TOlexViewer::ShowQPeaks(short what)  {
  GXApp->SetQPeaksVisible((what & olxv_ShowQAtoms) != 0);
  GXApp->SetQPeaksVisible((what & olxv_ShowQBonds) != 0);
}
//.......................................................................................
void TOlexViewer::ShowCell(bool v)  {
  GXApp->SetCellVisible(v);
}
//.......................................................................................
void TOlexViewer::DrawStyle(short style)  {
  if( style == olxv_DrawStylePers )  {
    GXApp->AtomRad("pers");
    TXAtom::DefDS(adsSphere);
    TXBond::DefMask(48);
    GXApp->UpdateAtomPrimitives(1);
    GXApp->SetAtomDrawingStyle(adsSphere);
    DefDS = style;
  }
  else if( style == olxv_DrawStyleTelp )  {
    GXApp->AtomRad("isot");
    TXAtom::DefDS(adsEllipsoid);
    TXBond::DefMask(48);
    GXApp->CalcProbFactor(50);
    GXApp->UpdateAtomPrimitives(5);
    GXApp->SetAtomDrawingStyle(adsEllipsoid);
    DefDS = style;
  }
  else if( style == olxv_DrawStyleSfil )  {
    GXApp->AtomRad("sfil");
    TXAtom::DefDS(adsSphere);
    TXBond::DefMask(48);
    GXApp->UpdateAtomPrimitives(1);
    GXApp->SetAtomDrawingStyle(adsSphere);
    DefDS = style;
  }
}
//.......................................................................................
void TOlexViewer::LoadStyle(const olxstr& _styleFile)  {
  olxstr styleFile( TEFile::ChangeFileExt(_styleFile, "glds"));
  try  {
    if( TEFile::Exists(styleFile) )  {
      TDataFile df;
      df.LoadFromXLFile(styleFile);
      GXApp->GetRender().GetStyles().FromDataItem(*df.Root().FindItem("style"), true);
      GXApp->CreateObjects(true, false);
    }
  }
  catch( ... )  {  }  // be quite
}
//.......................................................................................
void TOlexViewer::LoadScene(const olxstr& _sceneFile)  {
  olxstr sceneFile( TEFile::ChangeFileExt(_sceneFile, "glsp"));
  try  {
    if( TEFile::Exists(sceneFile) )  {
      TDataFile df;
      df.LoadFromXLFile(sceneFile);
      GXApp->GetRender().LightModel.FromDataItem(*df.Root().FindItem("Scene_Properties"));
    }
  }
  catch( ... )  {  }  // be quite
}
//.......................................................................................
bool TOlexViewer::executeMacroEx(const olxstr& cmdLine, TMacroError& er)  {
  return false;
}
//.......................................................................................
void TOlexViewer::print(const olxstr& Text, const short MessageType)  {
  return;
}
//.......................................................................................
bool TOlexViewer::executeFunction(const olxstr& funcName, olxstr& retValue)  {
  return false;
}
//.......................................................................................
IEObject* TOlexViewer::executeFunction(const olxstr& funcName)  {
  return NULL;
}
//.......................................................................................
TLibrary&  TOlexViewer::GetLibrary()  {  return GXApp->GetLibrary();  }
//.......................................................................................
bool TOlexViewer::registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn) {
  return false;
}
//.......................................................................................
void TOlexViewer::unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName) {
  return;
}
//.......................................................................................
const olxstr& TOlexViewer::getDataDir() const  {  return EmptyString;  }
//.......................................................................................
const olxstr& TOlexViewer::getVar(const olxstr &name, const olxstr &defval) const  {
  return EmptyString;
}
//.......................................................................................
void TOlexViewer::setVar(const olxstr &name, const olxstr &val) const {
  return;
}
//.......................................................................................
//.......................................................................................
const char* olxv_Initialize(HDC hdc, int w, int h)  {
  if( TOlexViewer::Instance != NULL )  return "";
  try  {
    new TOlexViewer(hdc, w, h);
  }
  catch(const TExceptionBase& exc)  {
    return exc.GetException()->GetError().c_str();
  }
  return "";
}
void olxv_Finalize()  {
  if( TOlexViewer::Instance != NULL )  delete TOlexViewer::Instance;
}
void olxv_OnPaint()  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->OnPaint();
}
void olxv_OnSize(int w, int h)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->OnSize(w, h);
}
bool olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState)  {
  if( TOlexViewer::Instance == NULL )  return false;
  return TOlexViewer::Instance->OnMouse(w, h, MouseEvent, MouseButton, ShiftState);
}
bool olxv_OnFileChanged(const char* FN)  {
  if( TOlexViewer::Instance != NULL )  return TOlexViewer::Instance->OnFileChanged(FN);
  return false;
}
const char* olxv_GetObjectLabelAt(int x, int y)  {
  return (TOlexViewer::Instance != NULL) ? TOlexViewer::Instance->GetObjectLabelAt(x, y).c_str() : "";
}
const char* olxv_GetSelectionInfo()  {
  return (TOlexViewer::Instance != NULL) ? TOlexViewer::Instance->GetSelectionInfo().c_str() : "";
}
void olxv_ShowLabels(unsigned short type)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->ShowLabels(type);
}
void olxv_ShowQPeaks(short what)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->ShowQPeaks(what);
}
void olxv_ShowCell(bool v)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->ShowCell(v);
}
void olxv_DrawStyle(short style)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->DrawStyle(style);
}
void olxv_LoadScene(const char* FN)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->LoadScene(FN);
}
void olxv_LoadStyle(const char* FN)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->LoadStyle(FN);
}
void olxv_Clear()  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->Clear();
}
int olxv_GetVersion()  {
  return DllVersion;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)  {
    return TRUE;
}

