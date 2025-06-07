/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "gxapp.h"
#include "olex2app_imp.h"
#include "ctrls.h"

class TGlXApp: public wxApp  {
private:
  bool OnInit();
  int OnExit();
  Olex2App* XApp;
  class TMainForm* MainForm;
  static TGlXApp *&Instance() {
    static TGlXApp* Instance=0;
    return Instance;
  }
  TEFile* pid_file;
  void OnChar(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnCmd(olxCommandEvent &evt);
  void MacOpenFile(const wxString &fileName);
#ifdef _WIN32
  static BOOL CALLBACK QueryOlex2Windows(HWND w, LPARAM p);
#endif
public:
  TGlXApp() : pid_file(0)  {}
  ~TGlXApp();
  bool Dispatch();
//  int MainLoop();
  TEFile *GetPIDFile() const { return pid_file; }
  static TGlXApp* GetInstance() { return Instance(); }
  static TMainForm* GetMainForm() { return GetInstance()->MainForm; }
  static TGXApp* GetGXApp() { return GetInstance()->XApp; }

#ifdef _WIN32
  static HWND FindWindow(const olxstr& p_name, const olxstr& f_name);
  static const olxstr& GetFileQueryEvtName();
  static const olxstr& GetFileQueryFileName();
  static UINT GetFileQueryEvtId();
#endif
};

DECLARE_APP(TGlXApp);
