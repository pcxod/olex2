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

  class TCheckBox: public wxCheckBox, public AActionHandler, public AOlxCtrl  {
    void ClickEvent(wxCommandEvent& event);
    void MouseEnterEvent(wxMouseEvent& event);
    olxstr Data, OnCheckStr, OnUncheckStr, OnClickStr, DependMode;
    TActionQueue *ActionQueue;
  public:
    TCheckBox(wxWindow *Parent, long style=0) :
      wxCheckBox(Parent, -1, wxString(), wxDefaultPosition, wxDefaultSize, style),
      AOlxCtrl(this),
      OnClick(Actions.New(evt_on_click_id)),
      OnCheck(Actions.New(evt_on_check_id)),
      OnUncheck(Actions.New(evt_on_uncheck_id)),
      ActionQueue(NULL),
      Data(EmptyString()),
      OnCheckStr(EmptyString()),
      OnUncheckStr(EmptyString()),
      OnClickStr(EmptyString()),
      DependMode(EmptyString())  {  SetToDelete(false);  }
    virtual ~TCheckBox()  {  if( ActionQueue != NULL )  ActionQueue->Remove(this);  }

    void SetActionQueue(TActionQueue& q, const olxstr& dependMode);
    bool Execute(const IEObject *Sender, const IEObject *Data);
    void OnRemove()  {  ActionQueue = NULL;  }

    DefPropC(olxstr, Data)       // data associated with the object
    DefPropC(olxstr, OnUncheckStr)   // this is passed to the OnUncheck event
    DefPropC(olxstr, OnCheckStr)    // this is passed to the OnCheck event
    DefPropC(olxstr, OnClickStr) // this is passed to the OnClick event

    TActionQueue &OnClick, &OnCheck, &OnUncheck;

    inline void SetCaption(const olxstr &T)  {  SetLabel(T.u_str()); }
    inline olxstr GetCaption() const {  return GetLabel(); }

    inline bool IsChecked() const {  return wxCheckBox::IsChecked();  }
    inline void SetChecked(bool v)  {  wxCheckBox::SetValue(v);  }

    DECLARE_CLASS(TCheckBox)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
