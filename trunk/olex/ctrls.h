//---------------------------------------------------------------------------

#ifndef _xl_ctrlsH
#define _xl_ctrlsH
#include "actions.h"
#include "ctrls/olxctrlbase.h"
#include "wx/wx.h"

#ifndef uiStr  // ansi string to wxString in unicode
  #define uiStr(v)  (wxString((v).u_str()))
  #define uiStrT(v) (wxString(v, *wxConvUI))
#endif

//---------------------------------------------------------------------------
namespace ctrl_ext  {

  class TMainFrame: public wxFrame, public AOlxCtrl  {
  protected:
    struct TWindowInfo  {  int x, y;  };
    TSStrPObjList<olxstr,TWindowInfo*, false> WindowPos;
    // extends filter for case sensitive OS
    olxstr PortableFilter(const olxstr& filter);
  public:
    TMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, const wxString &ClassName);
    virtual ~TMainFrame();
    void RestorePosition(wxWindow *Window); // restores previously saved position
    void SavePosition(wxWindow *Window);    //saves current position of the window on screen
    olxstr PickFile(const olxstr &Caption, const olxstr &Filter, const olxstr &DefFolder, bool Open);
    virtual void LockWindowDestruction(wxWindow* wnd) = 0;
    virtual void UnlockWindowDestruction(wxWindow* wnd) = 0;

    DECLARE_CLASS(TMainFrame)
  };

  class TDialog: public wxDialog, public AOlxCtrl {
  protected:
    TMainFrame *Parent;
  public:
    TDialog(TMainFrame *Parent, const wxString &Title, const wxString &ClassName);
    virtual ~TDialog();

    DECLARE_CLASS(TDialog)
  };

  class TTimer: public wxTimer, public IEObject  {
    void Notify()  {  OnTimer.Execute(this, NULL);  }
    TActionQList Actions;
  public:
    TTimer() : OnTimer(Actions.NewQueue("ONTIMER"))  {}
    virtual ~TTimer()  {}

    TActionQueue &OnTimer;
    DECLARE_CLASS(TTimer)
  };
};  // end of the _xl_Controls namespace
using namespace ctrl_ext;
#include "ctrls/menuext.h"
#include "ctrls/labelext.h"
#include "ctrls/comboext.h"
#include "ctrls/texteditext.h"
#include "ctrls/treeviewext.h"
#include "ctrls/trackbarext.h"
#include "ctrls/btnext.h"
#include "ctrls/spinctrlext.h"
#include "ctrls/listboxext.h"
#include "ctrls/checkboxext.h"

#endif
