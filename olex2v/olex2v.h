#ifndef  _olx_viewer
#define _olx_viewer

#include <windows.h>

#ifdef _MSC_VER
  #define DllImport   __declspec( dllimport )
  #define DllExport   __declspec( dllexport )
#endif
#ifdef __BORLANDC__
  #define DllExport __export
  #define DllImport __import
#endif

const short  olxv_MouseUp     = 0x0001,
             olxv_MouseDown   = 0x0002,
             olxv_MouseMove   = 0x0003;
const short  olxv_MouseLeft   = 0x0001,
             olxv_MouseMiddle = 0x0002,
             olxv_MouseRight  = 0x0004;
const short  olxv_ShiftCtrl   = 0x0001,
             olxv_ShiftShift  = 0x0002,
             olxv_ShiftAlt    = 0x0004;


extern "C" {
  const char* DllImport olxv_Initialize(HDC hdc, int w, int h);
  void DllImport olxv_Finalize();
  void DllImport olxv_OnPaint();
  void DllImport olxv_OnSize(int w, int h);
  bool DllImport olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState);
  void DllImport olxv_OnFileChanged(const char* FN);
  const char* DllImport olxv_GetObjectLabelAt(int x, int y);
  const char* DllImport olxv_GetSelectionInfo();
  void DllImport olxv_ShowLabels(bool v);
};

#endif
