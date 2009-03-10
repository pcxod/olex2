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
public:

  bool Dispatch();
//  int MainLoop();
  void OnIdle(wxIdleEvent& event);

  static TGlXApp*  GetInstance()  {  return Instance;  }
  static TMainForm* GetMainForm() {  return GetInstance()->MainForm;  }
  static TGXApp* GetGXApp()       {  return GetInstance()->XApp;  }

  DECLARE_EVENT_TABLE()
};

DECLARE_APP(TGlXApp);

#endif
