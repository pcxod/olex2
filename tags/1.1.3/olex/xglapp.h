#ifndef xglAppH
#define xglAppH
#include "wx/wx.h"
#include "gxapp.h"

class TGlXApp: public wxApp  {
private:
  bool OnInit();
  int OnExit();
  TGXApp* XApp;
  class TMainForm* MainForm;
  static TGlXApp* Instance;
  TEFile* pid_file;
  void OnChar(wxKeyEvent& event);  
  void OnKeyDown(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);
  void OnIdle(wxIdleEvent& event);
public:
  TGlXApp() : pid_file(NULL)  {}
  bool Dispatch();
//  int MainLoop();

  static TGlXApp*  GetInstance()  {  return Instance;  }
  static TMainForm* GetMainForm() {  return GetInstance()->MainForm;  }
  static TGXApp* GetGXApp()       {  return GetInstance()->XApp;  }

  DECLARE_EVENT_TABLE()
};

DECLARE_APP(TGlXApp);

#endif
