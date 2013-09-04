/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dlg_genoptdlg_H
#define __olx_dlg_genoptdlg_H
#include "ctrls.h"

class TdlgGenerate: public TDialog, AActionHandler  {
private:
  wxStaticText *stAFrom, *stBFrom, *stCFrom, *stATo, *stBTo, *stCTo;
  wxTextCtrl *tcAFrom, *tcBFrom, *tcCFrom, *tcATo, *tcBTo, *tcCTo;
  TComboBox *cbA, *cbB, *cbC;
protected:
  void OnOK(wxCommandEvent& event);
  void OnLinkChanges(wxCommandEvent& event);
  void OnAChange();
  void OnBChange();
  void OnCChange();
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *);

  float AFrom, BFrom, CFrom, ATo, BTo, CTo;
public:
  TdlgGenerate(TMainFrame *ParentFrame);
  virtual ~TdlgGenerate();
  DefPropP(float, AFrom)
  DefPropP(float, BFrom)
  DefPropP(float, CFrom)
  DefPropP(float, ATo)
  DefPropP(float, BTo)
  DefPropP(float, CTo)

  DECLARE_EVENT_TABLE()
};
#endif
