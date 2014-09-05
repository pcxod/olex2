/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_H
#define __olx_ctrl_H
#include "actions.h"
#include "ctrls/olxctrlbase.h"
#include "wx/wx.h"

class olxCommandEvent;
wxDECLARE_EVENT(OLX_COMMAND_EVT, olxCommandEvent);
class olxCommandEvent : public wxEvent {
  olxstr cmd;
public:
  olxCommandEvent(const olxCommandEvent &e)
    : wxEvent(e), cmd(e.cmd)
  {}
  olxCommandEvent(const olxstr &cmd, int id=-1)
    : wxEvent(id, OLX_COMMAND_EVT), cmd(cmd)
  {}
  wxEvent *Clone() const { return new olxCommandEvent(*this); }
  const olxstr &GetCommand() const { return cmd; }
  void SetCommand(const olxstr &c) { cmd = c; }
};

namespace ctrl_ext  {

  class TDialog: public wxDialog, public AOlxCtrl {
    void OnSizeEvt(wxSizeEvent& event);
  protected:
    class TMainFrame *Parent;
    TActionQList Actions;
    bool manage_parent;
  public:
    TDialog(TMainFrame *Parent,
      const wxString &Title,
      const wxString &ClassName,
      const wxPoint& position = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      int style=wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE);
    TDialog(wxWindow *Parent,
      const wxString &Title,
      const wxString &ClassName,
      const wxPoint& position = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      int style=wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE);
    virtual ~TDialog();
    virtual bool Show(bool show=true);
    /* extended version, if manage_parent is true, the parent window is
    disabled before the execution and is enabled before hiding this window
    */
    int ShowModalEx(bool manage_parent);
    TActionQueue &OnResize;
    DECLARE_CLASS(TDialog)
    DECLARE_EVENT_TABLE()
  };

  class TTimer: public wxTimer, public IEObject  {
    void Notify()  {  OnTimer.Execute(this, NULL);  }
    TActionQList Actions;
  public:
    TTimer() : OnTimer(Actions.New("ONTIMER"))  {}
    virtual ~TTimer()  {}

    TActionQueue &OnTimer;
    DECLARE_CLASS(TTimer)
  };
};  // end of the _xl_Controls namespace
using namespace ctrl_ext;
#include "ctrls/frameext.h"
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
#include "ctrls/datectrlext.h"
#include "ctrls/colorext.h"

#endif
