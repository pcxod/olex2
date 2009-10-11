#ifndef  _olx_viewer
#define _olx_viewer

#include <windows.h>
#include "gxapp.h"
#include "integration.h"
#include "exception.h"

#ifdef __DLL__
#ifdef _MSC_VER
  #define DllImport   __declspec( dllimport )
  #define DllExport   __declspec( dllexport )
#endif
#ifdef __BORLANDC__
  #define DllExport __export
  #define DllImport __import
  #ifdef __WXWIDGETS__
    #define HAVE_SNPRINTF_DECL
  #endif
#endif // not a DLL build
#else
  #define DllExport
  #define DllImport
#endif // __DLL__
const short  olxv_MouseUp     = 0x0001,
             olxv_MouseDown   = 0x0002,
             olxv_MouseMove   = 0x0003;
const short  olxv_MouseLeft   = 0x0001,
             olxv_MouseMiddle = 0x0002,
             olxv_MouseRight  = 0x0004;
const short  olxv_ShiftCtrl   = 0x0001,
             olxv_ShiftShift  = 0x0002,
             olxv_ShiftAlt    = 0x0004;

class TOlexViewer : public olex::IOlexProcessor {
  HDC WindowDC;
  HGLRC GlContext;
  int Width, Height;
  TGXApp* GXApp;
public:
  TOlexViewer(const char* olex_path, HDC windowDC, int w, int h);
  ~TOlexViewer();
  void OnPaint();
  void OnSize(int w, int h);
  bool OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState);
  void OnFileChanged(const char* fileName);
    virtual bool executeMacro(const olxstr& cmdLine);
    virtual void print(const olxstr& Text, const short MessageType = olex::mtNone);
    virtual bool executeFunction(const olxstr& funcName, olxstr& retValue);
    // returns a value, which should be deleted, of the TPType <> type
    virtual IEObject* executeFunction(const olxstr& funcName);
    virtual TLibrary&  GetLibrary();
    virtual bool registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn);
    virtual void unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName);

    virtual const olxstr& getDataDir() const;
    virtual const olxstr& getVar(const olxstr &name, const olxstr &defval=NullString) const;
    virtual void setVar(const olxstr &name, const olxstr &val) const;

  static TOlexViewer* Instance;
};

#ifdef __DLL__
extern "C" {
#endif
const char* DllExport olxv_Initialize(const char* olex_path, HDC hdc, int w, int h)  {
  try  {
    new TOlexViewer(olex_path, hdc, w, h);
  }
  catch(const TExceptionBase& exc)  {
    return exc.GetException()->GetError().c_str();
  }
  return "";
}
void DllExport olxv_Finalize()  {
  if( TOlexViewer::Instance != NULL )  delete TOlexViewer::Instance;
}
void DllExport olxv_OnPaint()  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->OnPaint();
}
void DllExport olxv_OnSize(int w, int h)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->OnSize(w, h);
}
bool DllExport olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState)  {
  if( TOlexViewer::Instance == NULL )  return false;
  return TOlexViewer::Instance->OnMouse(w, h, MouseEvent, MouseButton, ShiftState);
}
void DllExport olxv_OnFileChanged(const char* FN)  {
  if( TOlexViewer::Instance != NULL )  TOlexViewer::Instance->OnFileChanged(FN);
}
#ifdef __DLL__
};  // extern "C"
#endif
#endif
