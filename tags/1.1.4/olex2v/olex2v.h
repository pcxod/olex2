#ifndef  _olx_viewer
#define _olx_viewer

#include "ebase.h"

#ifdef _MSC_VER
  #define DllImport   __declspec( dllimport )
  #define DllExport   __declspec( dllexport )
#endif
#ifdef __BORLANDC__
  #define DllExport __export
  #define DllImport __import
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

extern "C" {
  const char* DllImport olxv_Initialize(HDC hdc, int w, int h);
  void DllImport olxv_Finalize();
  void DllImport olxv_OnPaint();
  void DllImport olxv_OnSize(int w, int h);
  bool DllImport olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState);
  bool DllImport olxv_OnFileChanged(const char* FN);
  const char* DllImport olxv_GetObjectLabelAt(int x, int y);
  const char* DllImport olxv_GetSelectionInfo();
  void DllImport olxv_ShowLabels(unsigned short what);
  void DllImport olxv_ShowQPeaks(short what /* bonds, atoms, none*/);
  void DllImport olxv_ShowCell(bool v);
  void DllImport olxv_EnableSelection(bool v);
  void DllImport olxv_DrawStyle(short style);
  void DllImport olxv_LoadStyle(const char* FN);
  void DllImport olxv_LoadScene(const char* FN);
  void DllImport olxv_Clear();
  /* this changes if any of the function signatures changes or new functions added
    2008.09.19 - version 1 is returned, the starting point. Interface:
    const char* DllImport olxv_Initialize(HDC hdc, int w, int h);
    void DllImport olxv_Finalize();
    void DllImport olxv_OnPaint();
    void DllImport olxv_OnSize(int w, int h);
    bool DllImport olxv_OnMouse(int w, int h, short MouseEvent, short MouseButton, short ShiftState);
    void DllImport olxv_OnFileChanged(const char* FN);
    const char* DllImport olxv_GetObjectLabelAt(int x, int y);
    const char* DllImport olxv_GetSelectionInfo();
    void DllImport olxv_ShowLabels(unsigned short what);
    void DllImport olxv_ShowQPeaks(short what);
    void DllImport olxv_ShowCell(bool v);
    void DllImport olxv_DrawStyle(short style);
    void DllImport olxv_LoadStyle(const char* FN);
    void DllImport olxv_LoadScene(const char* FN);
    void DllImport olxv_Clear();
    int DllImport olxv_GetVersion();
    2008.09.30 - olxv_OnFileChange returns boolean to specify if load failed or not
  */
  int DllImport olxv_GetVersion();
};

#endif
