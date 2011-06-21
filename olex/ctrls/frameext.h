#ifndef __olx_ctrl_frameext_H
#define __olx_ctrl_frameext_H
#include "olxctrlbase.h"
#include "edict.h"
#include "dataitem.h"
#include "gllightmodel.h"

namespace ctrl_ext  {
  class TMainFrame: public wxFrame, public AOlxCtrl  {
    static TMainFrame* MainFrameInstance;
  protected:
    struct TWindowInfo  {  int x, y;  };
    olxdict<olxstr, TWindowInfo, olxstrComparator<false> > WindowPos;
    // extends filter for case sensitive OS
    olxstr PortableFilter(const olxstr& filter);
  public:
    TMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, const wxString &ClassName) :
      wxFrame((wxFrame*)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE),
      AOlxCtrl(this)  
    {
      MainFrameInstance = this;
    }

    virtual ~TMainFrame() {  MainFrameInstance = NULL;  }
    void RestorePosition(wxWindow *Window); // restores previously saved position
    void SavePosition(wxWindow *Window);    //saves current position of the window on screen
    olxstr PickFile(const olxstr &Caption, const olxstr &Filter, const olxstr &DefFolder, bool Open);
    virtual void LockWindowDestruction(wxWindow* wnd, const IEObject* caller) = 0;
    virtual void UnlockWindowDestruction(wxWindow* wnd, const IEObject* caller) = 0;

    virtual void SetScenesFolder(const olxstr& sf) = 0;
    virtual const olxstr& GetScenesFolder() const = 0;
    virtual void LoadScene(const TDataItem &root, TGlLightModel &scene) = 0;
    virtual void SaveScene(TDataItem &root, const TGlLightModel &scene) const = 0;
    
    static TMainFrame& GetMainFrameInstance() {  return *MainFrameInstance;  }
    static int ShowAlert(const olxstr &msg, const olxstr &title, int flags);
    static void ShowAlert(const TExceptionBase &esc, const olxstr &msg=EmptyString(), bool log=true);
    DECLARE_CLASS(TMainFrame)
  };
};  // end namespace ctrl_ext
#endif
