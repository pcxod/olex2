/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_editdlg_H
#define __olx_editdlg_H
#include "ctrls.h"

class TdlgEdit: public wxDialog  {
private:
  wxTextCtrl *Text;
  TMainFrame *FParent;
public:
  TdlgEdit(TMainFrame *ParentFrame, bool MultiLine);
  ~TdlgEdit();
  void SetText(const olxstr& Text);
  olxstr GetText();
  TWindowInterface WI;
};

class TdlgStyledEdit: public wxDialog  {
private:
  wxStyledTextCtrl *Text;
  TMainFrame *FParent;
public:
  TdlgStyledEdit(TMainFrame *ParentFrame, bool MultiLine);
  ~TdlgStyledEdit();
  void SetText(const olxstr& Text);
  olxstr GetText();
  TWindowInterface WI;
};
#endif
