/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_cmdline_H
#define __olx_cmdline_H
#include "ctrls.h"

class TCmdLine : public TTextEdit {
protected:
  olxstr PromptStr;
  TStrList Commands;
  int CmdIndex;
public:
  TCmdLine(wxWindow *parent, int flags);
  virtual ~TCmdLine();

  bool ProcessKey(wxKeyEvent& evt);
  // checks only Del, Backspace
  bool WillProcessKey(wxKeyEvent& evt);

  DefPropC(olxstr, PromptStr)

  olxstr GetCommand()  {
    return GetText().Length() > PromptStr.Length()
      ? GetText().SubStringFrom(PromptStr.Length())
      : EmptyString();
  }
  void SetCommand(const olxstr& cmd);
  const olxstr& GetLastCommand(const olxstr &name) const;

  TActionQueue &OnCommand;
};

#endif
