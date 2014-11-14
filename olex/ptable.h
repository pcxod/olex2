/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dlg_ptable_H
#define __olx_dlg_ptable_H
#include "ctrls.h"
#include "chemdata.h"

class TPTableDlg: public TDialog, public AActionHandler  {
protected:
  void CreateButton(int i, int j, int offset);
  TPtrList<TButton> ButtonsList;
  cm_Element *Selected;
public:
  TPTableDlg(TMainFrame *Parent);
  virtual ~TPTableDlg();
  cm_Element* GetSelected()  {  return Selected; }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *);
};
#endif
