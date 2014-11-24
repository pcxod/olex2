/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_checkbox_H
#define __olx_ctrl_checkbox_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class TCheckBox: public wxCheckBox, public AActionHandler, public AOlxCtrl {
    void ClickEvent(wxCommandEvent& event);
    void MouseEnterEvent(wxMouseEvent& event);
    olxstr Data, OnCheckStr, OnUncheckStr, OnClickStr, DependMode;
    TActionQueue *ActionQueue;
  public:
    TCheckBox(wxWindow *Parent, wxWindowID id = -1,
      const wxString& label = wxEmptyString,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0);

    virtual ~TCheckBox() {
      if (ActionQueue != NULL)  ActionQueue->Remove(this);
    }

    void SetActionQueue(TActionQueue& q, const olxstr& dependMode);
    bool Execute(const IOlxObject *, const IOlxObject *, TActionQueue *);
    void OnRemove(TActionQueue *) { ActionQueue = NULL; }

    DefPropC(olxstr, Data)

    AOlxCtrl::ActionQueue &OnClick, &OnCheck, &OnUncheck;

    void SetCaption(const olxstr &T) { SetLabel(T.u_str()); }
    olxstr GetCaption() const { return GetLabel(); }

    bool IsChecked() const { return wxCheckBox::IsChecked(); }
    void SetChecked(bool v) { wxCheckBox::SetValue(v); }
  };
}; // end namespace ctrl_ext
#endif
