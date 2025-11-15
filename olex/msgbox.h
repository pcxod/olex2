/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dlg_msgbox_H
#define __olx_dlg_msgbox_H
#include "ctrls.h"

class TdlgMsgBox: public TDialog, public AEventsDispatcher {
protected:
  wxCheckBox* cbRemember;
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
    const IOlxObject* Data, TActionQueue*);
  TPtrList<TButton> buttons;
  uint64_t SelfTimeout, ShownAt;
  wxWindowID SelfTimeoutId;
public:
  TdlgMsgBox(TMainFrame* Parent, const olxstr& msg, const olxstr& title,
    const olxstr& tickBoxMsg, long flags, bool ShowRememberCheckBox,
    long selfTimeout=0, wxWindowID selfTimeoutId=0);
  virtual ~TdlgMsgBox();
  bool IsChecked()  {  return cbRemember == 0 ? false : cbRemember->IsChecked();  }
  // return a string YNCO for yes, no, cancel, or OK, R-remember choice
  static olxstr Execute(TMainFrame* Parent, const olxstr& msg, const olxstr& title,
    const olxstr& tickBoxMsg ="Remember my decision",
    long flags=wxID_OK|wxICON_INFORMATION,
    bool ShowRememberCheckBox=false,
    long selfTimeout=0, wxWindowID selfTimeoutId=0);
};
#endif
