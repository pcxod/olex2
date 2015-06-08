/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_base_H
#define __olx_ctrl_base_H
#include "actions.h"
#include "../wininterface.h"

namespace ctrl_ext  {

  class AOlxCtrl : public virtual IOlxObject {
  protected:
    AOlxCtrl(wxWindow* this_window) : WI(this_window) {}
    virtual ~AOlxCtrl() {}
    TActionQList Actions;
  public:
    TWindowInterface WI;
    class ActionQueue : public TActionQueue {
      public:
        ActionQueue(TActionQList &parent, const olxstr &name)
          : TActionQueue(&parent, name)
        {
          parent.Add(this);
        }
        bool Execute(const IOlxObject *Sender, const IOlxObject *Data=NULL,
          TActionQueue *caller=NULL)
        {
          return TActionQueue::Execute(Sender, Data == NULL ? &data : Data);
        }
        static ActionQueue& New(TActionQList &parent, const olxstr &name)  {
          return *(new ActionQueue(parent, name));
        }
        olxstr data;
    };
    bool ExecuteActionQueue(const olxstr &name) const {
      ActionQueue *q = dynamic_cast<ActionQueue *>(
        Actions.Find(name.ToUpperCase()));
      if( q == NULL )  return false;
      olxstr data = GetActionQueueData(*q);
      q->Execute(this, &data, NULL);
      return true;
    }
  protected:
    // use this to do any required substitutions
    virtual olxstr GetActionQueueData(const ActionQueue &q) const {
      return q.data;
    }
  };

  const size_t UnknownSwitchState = (size_t)(-2);
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

  class TKeyEvent: public IOlxObject  {
    wxKeyEvent* Event;
  public:
    TKeyEvent(wxKeyEvent& evt)  {  Event = &evt;  }
    wxKeyEvent& GetEvent()    const {  return *Event;  }
    void SetEvent(wxKeyEvent& evt)  {  Event = &evt;  }
  };
};  //end namespace ctrl_ext
using namespace ctrl_ext;
#endif
