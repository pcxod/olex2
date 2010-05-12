#ifndef __olx_updateoptions_H
#define __olx_updateoptions_H
#include "ctrls.h"
#include "updateapi.h"
//---------------------------------------------------------------------------

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
