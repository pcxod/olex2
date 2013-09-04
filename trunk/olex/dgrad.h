/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_graddlg_H
#define __olx_graddlg_H
#include "ctrls.h"

class TdlgGradient: public TDialog, AActionHandler  {
private:
  TTextEdit *tcA, *tcB, *tcC, *tcD;
  wxStaticText *stcA, *stcB, *stcC, *stcD;
protected:
  void OnOK(wxCommandEvent& event);
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  int A, B, C, D;
  void Init();
public:
  TdlgGradient(TMainFrame *ParentFrame);
  virtual ~TdlgGradient();
  DefPropP(int, A)
  DefPropP(int, B)
  DefPropP(int, C)
  DefPropP(int, D)
  DECLARE_EVENT_TABLE()
};
#endif
