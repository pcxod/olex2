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

class TdlgMsgBox: public TDialog, public AActionHandler  {
protected:
  wxCheckBox* cbRemember;
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  TPtrList<TButton> buttons;
public:
  TdlgMsgBox(TMainFrame* Parent, const olxstr& msg, const olxstr& title, 
    const olxstr& tickBoxMsg, long flags, bool ShowRememberCheckBox);
  virtual ~TdlgMsgBox();
  bool IsChecked()  {  return cbRemember == NULL ? false : cbRemember->IsChecked();  }
  // return a string YNCO for yes, no, cancel, or OK, R-remember choice
  static olxstr Execute(TMainFrame* Parent, const olxstr& msg, const olxstr& title,
    const olxstr& tickBoxMsg ="Remember my decision", 
    long flags=wxID_OK|wxICON_INFORMATION, bool ShowRememberCheckBox=false);
};
#endif
