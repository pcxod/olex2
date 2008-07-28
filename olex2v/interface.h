#ifndef  _olx_viewer
#define _olx_viewer

#include <windows.h>
#include "gxapp.h"
#include "integration.h"
#include "exception.h"

#ifdef _MSC_VER
  #define DllImport   __declspec( dllimport )
  #define DllExportA   __declspec( dllexport )
  #define DllExportB
#endif
#ifdef __BORLANDC__
  #define DllExportA
  #define DllExportB __export
  #define DllImport __import
  #ifdef __WXWIDGETS__
    #define HAVE_SNPRINTF_DECL
  #endif
#endif

const short  // mouse event
  olxv_MouseUp     = 0x0001,
  olxv_MouseDown   = 0x0002,
  olxv_MouseMove   = 0x0003;
const short  // mouse button
  olxv_MouseLeft   = 0x0001,
  olxv_MouseMiddle = 0x0002,
  olxv_MouseRight  = 0x0004;
const short // shift modifiers 
  olxv_ShiftCtrl   = 0x0001,
  olxv_ShiftShift  = 0x0002,
  olxv_ShiftAlt    = 0x0004;
const unsigned short  // labels
  olxv_LabelsNone  = 0x0000,
  olxv_LabelsQ     = 0x0001,
  olxv_LabelsH     = 0x0002,
  olxv_LabelsAll   = 0xFFFF;
const short // draw style
  olxv_DrawStylePers = 0x0001, // balls and stricks
  olxv_DrawStyleTelp = 0x0002, // ellipsoids
  olxv_DrawStyleSfil = 0x0003; // sphere packing
const short // show Q peaks 
  olxv_ShowQNone  = 0x0000,
  olxv_ShowQBonds = 0x0001,
  olxv_ShowQAtoms = 0x0002;


class TOlexViewer : public olex::IOlexProcessor {
  HDC WindowDC;
  HGLRC GlContext;
  int Width, Height;
  TGXApp* GXApp;
  short DefDS;
public:
  TOlexViewer(HDC windowDC, int w, int h);
  ~TOlexViewer();
  void OnPaint();
  void OnSize(int w, int h);
  bool OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState);
  void OnFileChanged(const char* fileName);
  olxstr GetObjectLabelAt(int x, int y);
  olxstr GetSelectionInfo();
  void ShowLabels(unsigned short type);
  void ShowQPeaks(short what);
  void ShowCell(bool v);
  void DrawStyle(short style);
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

extern "C" {
  DllExportA const char* DllExportB olxv_Initialize(HDC hdc, int w, int h);
  DllExportA void DllExportB olxv_Finalize();
  DllExportA void DllExportB olxv_OnPaint();
  DllExportA void DllExportB olxv_OnSize(int w, int h);
  DllExportA bool DllExportB olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState);
  DllExportA void DllExportB olxv_OnFileChanged(const char* FN);
  DllExportA const char* DllExportB olxv_GetObjectLabelAt(int x, int y);
  DllExportA const char* DllExportB olxv_GetSelectionInfo();
  DllExportA void DllExportB olxv_ShowLabels(unsigned short type);
  DllExportA void DllExportB olxv_ShowQPeaks(short what /* bonds, atoms, none*/);
  DllExportA void DllExportB olxv_ShowCell(bool v);
  DllExportA void DllExportB olxv_DrawStyle(short style);
};  // extern "C"
#endif
