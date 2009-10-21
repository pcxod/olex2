#ifndef __olx_ctrl_base_H
#define __olx_ctrl_base_H

#define StartEvtProcessing()   TMainFrame::GetMainFrameInstance().LockWindowDestruction(GetParent()); \
  try  {

#define EndEvtProcessing()  }  catch( const TExceptionBase& exc )  {\
    TMainFrame::GetMainFrameInstance().UnlockWindowDestruction(GetParent());\
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate() );\
  }\
  TMainFrame::GetMainFrameInstance().UnlockWindowDestruction(GetParent());

#include "actions.h"
#include "../wininterface.h"

namespace ctrl_ext  {

  class AOlxCtrl : public IEObject {
  protected:
    AOlxCtrl(wxWindow* this_window) : WI(this_window) {}
    virtual ~AOlxCtrl() {}
    TActionQList Actions;
  public:
    TWindowInterface WI;
  };

  const short UnknownSwitchState   = -2;
  static olxstr 
    evt_change_id("ONCHANGE"),
    evt_on_mouse_leave_id("ONLEAVE"),
    evt_on_mouse_enter_id("ONENTER"),
    evt_on_return_id("ONRETURN"),
    evt_on_char_id("ONCHAR"),
    evt_on_key_down_id("ONKEYDOWN"),
    evt_on_key_up_id("ONKEYUP"),
    evt_on_click_id("ONCLICK"),
    evt_on_dbl_click_id("ONDBLCLICK"),
    evt_on_mouse_down_id("ONMOUSEDOWN"),
    evt_on_mouse_up_id("ONMOUSEUP"),
    evt_on_mode_change_id("ONMODECHANGE"),
    evt_on_select_id("ONSELECT"),
    evt_on_check_id("ONCHECK"),
    evt_on_uncheck_id("ONUNCKECK")
    ;

  class TKeyEvent: public IEObject  {
    wxKeyEvent* Event;
  public:
    TKeyEvent(wxKeyEvent& evt)  {  Event = &evt;  }
    virtual ~TKeyEvent()  {  ;  }

    wxKeyEvent& GetEvent()    const {  return *Event;  }
    void SetEvent(wxKeyEvent& evt)  {  Event = &evt;  }
  };
};  //end namespace ctrl_ext
using namespace ctrl_ext;
#endif
