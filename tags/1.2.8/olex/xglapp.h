/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xglApp_H
#define __olx_xglApp_H
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
    static TGlXApp* Instance=NULL;
    return Instance;
  }
  TEFile* pid_file;
  void OnChar(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnCmd(olxCommandEvent &evt);
  void MacOpenFile(const wxString &fileName);
public:
  TGlXApp() : pid_file(NULL)  {}
  bool Dispatch();
//  int MainLoop();
  TEFile *GetPIDFile() const { return pid_file; }
  static TGlXApp* GetInstance() { return Instance(); }
  static TMainForm* GetMainForm() { return GetInstance()->MainForm; }
  static TGXApp* GetGXApp() { return GetInstance()->XApp; }
};

DECLARE_APP(TGlXApp);
#endif
