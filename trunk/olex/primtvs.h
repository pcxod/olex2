/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dlg_primtvs_H
#define __olx_dlg_primtvs_H
#include "ctrls.h"
#include "gpcollection.h"

class TdlgPrimitive: public TDialog  {
protected:
  void OnOK(wxCommandEvent& event);
  TPtrList<wxCheckBox> Boxes;
  wxComboBox *cbApplyTo;
  AGDrawObject& Object;
public:
  TdlgPrimitive(TMainFrame *P, AGDrawObject& object);
  virtual ~TdlgPrimitive()  {}
  int32_t Mask;
  short Level;
  DECLARE_EVENT_TABLE()
};

#endif
