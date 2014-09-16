/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_updateoptions_H
#define __olx_updateoptions_H
#include "ctrls.h"
#include "updateapi.h"

class TdlgUpdateOptions: public TDialog  {
  wxStaticText *stProxy, *stProxyUser, *stProxyPasswd, 
    *stRepository, *stLastUpdated;
  wxTextCtrl *tcProxy;
  wxComboBox* cbRepository;
  wxCheckBox* cbQueryUpdate;
  wxRadioBox *rbUpdateInterval;
protected:
  void OnOK(wxCommandEvent& event);
  updater::UpdateAPI uapi;
public:

  TdlgUpdateOptions(TMainFrame *ParentFrame);
  virtual ~TdlgUpdateOptions();

  DECLARE_EVENT_TABLE()
};
#endif
